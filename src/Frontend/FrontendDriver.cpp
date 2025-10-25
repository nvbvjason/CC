#include "FrontendDriver.hpp"
#include "VariableResolution.hpp"
#include "ASTIr.hpp"
#include "Token.hpp"
#include "StateCode.hpp"
#include "ASTPrinter.hpp"
#include "GenerateIr.hpp"
#include "GotoLabelsUnique.hpp"
#include "Lexer.hpp"
#include "LvalueVerification.hpp"
#include "Parser.hpp"
#include "LoopLabeling.hpp"
#include "ValidateReturn.hpp"
#include "TypeResolution.hpp"
#include "DeSugarSimple.hpp"

#include <fstream>
#include <iostream>

#include "DeSugarDeref.hpp"

static i32 lex(TokenStore& tokenStore, const std::filesystem::path& inputFile);
static bool parse(const TokenStore& tokenStore, Parsing::Program& programNode);
static void printParsingAst(const Parsing::Program& program);
static Ir::Program ir(const Parsing::Program& parsingProgram, SymbolTable& symbolTable);
static std::string preProcess(const std::filesystem::path& file);
static std::string getSourceCode(const std::filesystem::path& inputFile);

std::tuple<std::optional<Ir::Program>, StateCode> FrontendDriver::run() const
{
    TokenStore tokenStore;
    if (lex(tokenStore, m_inputFile) != 0) {
        if (m_arg == "--printTokens")
            for (size_t i = 0; i < tokenStore.size(); ++i)
                std::cout << tokenStore.getToken(i) << '\n';
        return {std::nullopt, StateCode::Lexer};
    }
    if (m_arg == "--lex")
        return {std::nullopt, StateCode::Done};
    if (m_arg == "--printTokens") {
        for (size_t i = 0; i < tokenStore.size(); ++i)
            std::cout << tokenStore.getToken(i) << '\n';
        return {std::nullopt, StateCode::Done};
    }
    Parsing::Program program;
    if (!parse(tokenStore, program))
        return {std::nullopt, StateCode::Parser};
    if (m_arg == "--parse")
        return {std::nullopt, StateCode::Done};
    if (m_arg == "--printAst") {
        printParsingAst(program);
        return {std::nullopt, StateCode::Done};
    }
    SymbolTable symbolTable;
    if (StateCode err = validateSemantics(program, symbolTable); err != StateCode::Done)
        return {std::nullopt, err};
    if (m_arg == "--validate")
        return {std::nullopt, StateCode::Done};
    if (m_arg == "--printAstAfter") {
        printParsingAst(program);
        return {std::nullopt, StateCode::Done};
    }
    Ir::Program irProgram = ir(program, symbolTable);
    return {std::move(irProgram), StateCode::Continue};
}

std::string getSourceCode(const std::filesystem::path& inputFile)
{
    std::ifstream file(inputFile);
    if (!file.is_open())
        return {};
    std::string source((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return source;
}

StateCode validateSemantics(Parsing::Program& program, SymbolTable& symbolTable)
{
    Semantics::DeSugarSimple deSugarCompoundAssign;
    deSugarCompoundAssign.deSugar(program);
    Semantics::VariableResolution variableResolution(symbolTable);
    if (!variableResolution.resolve(program))
        return StateCode::VariableResolution;
    Semantics::TypeResolution typeResolution;
    if (!typeResolution.validate(program))
        return StateCode::TypeResolution;
    Semantics::LvalueVerification lvalueVerification;
    if (!lvalueVerification.resolve(program))
        return StateCode::LValueVerification;
    Semantics::DeSugarDeref deSugarDeref;
    deSugarDeref.deSugar(program);
    Semantics::ValidateReturn validateReturn;
    if (!validateReturn.programValidate(program))
        return StateCode::ValidateReturn;
    Semantics::GotoLabelsUnique labelsUnique;
    if (!labelsUnique.programValidate(program))
        return StateCode::LabelsUnique;
    Semantics::LoopLabeling loopLabeling;
    if (!loopLabeling.programValidate(program))
        return StateCode::LoopLabeling;
    return StateCode::Done;
}

void printParsingAst(const Parsing::Program& program)
{
    Parsing::ASTPrinter printer;
    program.accept(printer);
    std::cout << printer.getString();
}

i32 lex(TokenStore& tokenStore, const std::filesystem::path& inputFile)
{
    const std::string source = preProcess(inputFile);
    Lexing::Lexer lexer(source, tokenStore);
    if (const i32 err = lexer.getLexemes(); err != 0)
        return err;
    return 0;
}

bool parse(const TokenStore& tokenStore, Parsing::Program& programNode)
{
    Parsing::Parser parser(tokenStore);
    if (!parser.programParse(programNode))
        return false;
    return true;
}

Ir::Program ir(const Parsing::Program& parsingProgram, SymbolTable& symbolTable)
{
    Ir::Program irProgram;
    Ir::GenerateIr generateIr(symbolTable);
    generateIr.program(parsingProgram, irProgram);
    return irProgram;
}

static std::string preProcess(const std::filesystem::path& file)
{
    const std::filesystem::path& inputFile(file);
    const std::filesystem::path generatedFilesDir = std::filesystem::path(PROJECT_ROOT_DIR) / "generated_files";
    const std::filesystem::path generatedFile = inputFile.string() + ".i";
    std::string command = "gcc -E -P ";
    command += inputFile.string();
    command += " -o ";
    command += generatedFile.string();
    if (std::system(command.c_str()) != 0)
        return {};
    return getSourceCode(generatedFile.string());
}