#include "FrontendDriver.hpp"
#include "VariableResolution.hpp"
#include "ASTIr.hpp"
#include "Token.hpp"
#include "StateCode.hpp"
#include "ASTPrinter.hpp"
#include "GenerateIr.hpp"
#include "Lexer.hpp"
#include "LvalueVerification.hpp"
#include "Parser.hpp"
#include "Labeling.hpp"
#include "ValidateReturn.hpp"
#include "TypeResolution.hpp"

#include <fstream>
#include <iostream>

static std::vector<Error> lex(TokenStore& tokenStore, const std::filesystem::path& inputFile);
static std::vector<Error> parse(const TokenStore& tokenStore, Parsing::Program& programNode);
static void printParsingAst(const Parsing::Program& program);
static Ir::Program ir(const Parsing::Program& parsingProgram, SymbolTable& symbolTable, const VarTable& varTable);
static std::string preProcess(const std::filesystem::path& file);
static std::string getSourceCode(const std::filesystem::path& inputFile);

std::tuple<std::optional<Ir::Program>, StateCode> FrontendDriver::run()
{
    if (const std::vector<Error> errors = lex(m_tokenStore, m_inputFile); !errors.empty()) {
        reportErrors(errors, m_tokenStore);
        return {std::nullopt, StateCode::Lexer};
    }
    if (m_arg == "--lex")
        return {std::nullopt, StateCode::Done};
    if (m_arg == "--printTokens") {
        for (size_t i = 0; i < m_tokenStore.size(); ++i)
            std::cout << m_tokenStore.getToken(i) << '\n';
        return {std::nullopt, StateCode::Done};
    }
    Parsing::Program program;
    if (const std::vector<Error> errors = parse(m_tokenStore, program); !errors.empty()) {
        reportErrors(errors, m_tokenStore);
        return {std::nullopt, StateCode::Parser};
    }
    if (m_arg == "--parse")
        return {std::nullopt, StateCode::Done};
    if (m_arg == "--printAst") {
        printParsingAst(program);
        return {std::nullopt, StateCode::Done};
    }
    SymbolTable symbolTable;
    VarTable varTable;
    if (const auto [err, errors] = validateSemantics(program, symbolTable, varTable);
        err != StateCode::Done) {
        reportErrors(errors, m_tokenStore);
        return {std::nullopt, err};
    }
    if (m_arg == "--validate")
        return {std::nullopt, StateCode::Done};
    if (m_arg == "--printAstAfter") {
        printParsingAst(program);
        return {std::nullopt, StateCode::Done};
    }
    Ir::Program irProgram = ir(program, symbolTable, varTable);
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

std::pair<StateCode, std::vector<Error>> validateSemantics(
    Parsing::Program& program,
    SymbolTable& symbolTable,
    VarTable& varTable)
{
    Semantics::VariableResolution variableResolution(symbolTable, varTable);
    if (const std::vector<Error> errors = variableResolution.resolve(program); !errors.empty())
        return {StateCode::VariableResolution, errors};
    Semantics::TypeResolution typeResolution(varTable);
    if (const std::vector<Error> errors = typeResolution.validate(program); !errors.empty())
        return {StateCode::TypeResolution, errors};
    Semantics::LvalueVerification lvalueVerification;
    if (const std::vector<Error> errors = lvalueVerification.resolve(program); !errors.empty())
        return {StateCode::LValueVerification, errors};
    Semantics::ValidateReturn validateReturn;
    if (std::vector<Error> errors = validateReturn.programValidate(program); !errors.empty())
        return {StateCode::ValidateReturn, errors};
    Semantics::Labeling loopLabeling(varTable);
    if (const std::vector<Error> errors = loopLabeling.programValidate(program); !errors.empty())
        return {StateCode::LoopLabeling, errors};
    return {StateCode::Done, {}};
}

void printParsingAst(const Parsing::Program& program)
{
    Parsing::ASTPrinter printer;
    program.accept(printer);
    std::cout << printer.getString();
}

std::vector<Error> lex(TokenStore& tokenStore, const std::filesystem::path& inputFile)
{
    const std::string source = preProcess(inputFile);
    Lexing::Lexer lexer(source, tokenStore);
    return lexer.getLexemes();
}

std::vector<Error> parse(const TokenStore& tokenStore, Parsing::Program& programNode)
{
    Parsing::Parser parser(tokenStore);
    return parser.programParse(programNode);
}

Ir::Program ir(const Parsing::Program& parsingProgram, SymbolTable& symbolTable, const VarTable& varTable)
{
    Ir::Program irProgram;
    Ir::GenerateIr generateIr(symbolTable, varTable);
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

void reportErrors(const std::vector<Error>& errors, const TokenStore& tokenStore)
{
    for (const Error& error : errors)
        reportError(error, tokenStore);
}

void reportError(const Error& error, const TokenStore& tokenStore)
{
    std::cout << error.msg << ' ' <<
        "line: " << tokenStore.getLineNumber(error.index) << ' ' <<
        "column: " << tokenStore.getColumnNumber(error.index) << '\n';
}