#include "FrontendDriver.hpp"
#include "VariableResolution.hpp"
#include "ASTIr.hpp"
#include "Token.hpp"
#include "ErrorCodes.hpp"
#include "ASTPrinter.hpp"
#include "GenerateIr.hpp"
#include "GotoLabelsUnique.hpp"
#include "Lexer.hpp"
#include "LvalueVerification.hpp"
#include "Parser.hpp"
#include "LoopLabeling.hpp"
#include "ValidateReturn.hpp"
#include "TypeResolution.hpp"

#include <fstream>
#include <iostream>


static i32 lex(std::vector<Lexing::Token>& lexemes, const std::filesystem::path& inputFile);
static bool parse(const std::vector<Lexing::Token>& tokens, Parsing::Program& programNode);
static void printParsingAst(const Parsing::Program& program);
static Ir::Program ir(const Parsing::Program& parsingProgram);
static std::string preProcess(const std::filesystem::path& file);
static std::string getSourceCode(const std::filesystem::path& inputFile);

std::tuple<Ir::Program, ErrorCode> FrontendDriver::run() const
{
    std::vector<Lexing::Token> tokens;
    if (lex(tokens, m_inputFile) != 0)
        return {std::move(Ir::Program()), ErrorCode::Lexer};
    if (m_arg == "--lex")
        return {Ir::Program(), ErrorCode::OK};
    Parsing::Program program;
    if (!parse(tokens, program))
        return {std::move(Ir::Program()), ErrorCode::Parser};
    if (m_arg == "--parse")
        return {std::move(Ir::Program()), ErrorCode::OK};
    if (m_arg == "--printAst") {
        printParsingAst(program);
        return {std::move(Ir::Program()), ErrorCode::OK};
    }
    SymbolTable symbolTable;
    if (ErrorCode err = validateSemantics(program, symbolTable); err != ErrorCode::OK)
        return {std::move(Ir::Program()), err};
    if (m_arg == "--validate")
        return {std::move(Ir::Program()), ErrorCode::OK};
    if (m_arg == "--printAstAfter") {
        printParsingAst(program);
        return {std::move(Ir::Program()), ErrorCode::OK};
    }
    Ir::Program irProgram = ir(program);
    return {std::move(irProgram), ErrorCode::OK};
}

std::string getSourceCode(const std::filesystem::path& inputFile)
{
    std::ifstream file(inputFile);
    if (!file.is_open())
        return {};
    std::string source((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return source;
}

ErrorCode validateSemantics(Parsing::Program& programNode, SymbolTable& symbolTable)
{
    Semantics::VariableResolution variableResolution(symbolTable);
    if (!variableResolution.resolve(programNode))
        return ErrorCode::VariableResolution;
    Semantics::ValidateReturn validateReturn;
    if (!validateReturn.programValidate(programNode))
        return ErrorCode::ValidateReturn;
    Semantics::LvalueVerification lvalueVerification;
    if (!lvalueVerification.resolve(programNode))
        return ErrorCode::LValueVerification;
    Semantics::GotoLabelsUnique labelsUnique;
    if (!labelsUnique.programValidate(programNode))
        return ErrorCode::LabelsUnique;
    Semantics::LoopLabeling loopLabeling;
    if (!loopLabeling.programValidate(programNode))
        return ErrorCode::LoopLabeling;
    Semantics::TypeResolution typeResolution;
    if (!typeResolution.validate(programNode))
        return ErrorCode::TypeResolution;
    return ErrorCode::OK;
}

void printParsingAst(const Parsing::Program& program)
{
    Parsing::ASTPrinter printer;
    program.accept(printer);
    std::cout << printer.getString();
}

i32 lex(std::vector<Lexing::Token> &lexemes, const std::filesystem::path& inputFile)
{
    const std::string source = preProcess(inputFile);
    Lexing::Lexer lexer(source);
    if (const i32 err = lexer.getLexemes(lexemes); err != 0)
        return err;
    return 0;
}

bool parse(const std::vector<Lexing::Token>& tokens, Parsing::Program& programNode)
{
    Parsing::Parser parser(tokens);
    if (!parser.programParse(programNode))
        return false;
    return true;
}

Ir::Program ir(const Parsing::Program& parsingProgram)
{
    Ir::Program irProgram;
    Ir::program(parsingProgram, irProgram);
    return irProgram;
}


static std::string preProcess(const std::filesystem::path& file)
{
    const std::filesystem::path& inputFile(file);
    const std::filesystem::path generatedFilesDir = std::filesystem::path(PROJECT_ROOT_DIR) / "generated_files";
    const std::filesystem::path generatedFile = generatedFilesDir / (inputFile.stem().string() + ".i");
    std::string command = "gcc -E -P ";
    command += inputFile.string();
    command += " -o ";
    command += generatedFile.string();
    if (std::system(command.c_str()) != 0)
        return {};
    return getSourceCode(generatedFile.string());
}