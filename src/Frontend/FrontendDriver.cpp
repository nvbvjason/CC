#include "FrontendDriver.hpp"
#include "VariableResolution.hpp"
#include "ASTIr.hpp"
#include "Token.hpp"

#include <fstream>
#include <iostream>

#include "ASTPrinter.hpp"
#include "GenerateIr.hpp"
#include "Lexer.hpp"
#include "LvalueVerification.hpp"
#include "Parser.hpp"
#include "ValidateReturn.hpp"

static i32 lex(std::vector<Lexing::Token>& lexemes, const std::string& inputFile);
static bool parse(const std::vector<Lexing::Token>& tokens, Parsing::Program& programNode);
static i32 printParsingAst(const Parsing::Program* program);
static bool validateSemantics(Parsing::Program& programNode);
static Ir::Program ir(const Parsing::Program* parsingProgram);
static std::string preProcess(const std::string &file);

std::tuple<std::unique_ptr<Ir::Program>, i32> FrontendDriver::run() const
{
    std::vector<Lexing::Token> tokens;
    if (const i32 err = lex(tokens, m_inputFile) != 0)
        return {nullptr, err};
    if (m_arg == "--lex")
        return {nullptr, 0};
    Parsing::Program program;
    if (!parse(tokens, program))
        return {nullptr, 1};
    if (m_arg == "--parse")
        return {nullptr, 0};
    if (!validateSemantics(program))
        return {nullptr, 1};
    if (m_arg == "--validate")
        return {nullptr, 0};
    if (m_arg == "--printAst")
        return {nullptr, printParsingAst(&program)};
    std::unique_ptr<Ir::Program> irProgram = std::make_unique<Ir::Program>(ir(&program));
    return {std::move(irProgram), 0};
}

std::string getSourceCode(const std::string &inputFile)
{
    std::ifstream file(inputFile);
    std::string source((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return source;
}

bool validateSemantics(Parsing::Program& programNode)
{
    Semantics::VariableResolution variableResolution(programNode);
    if (!variableResolution.resolve())
        return false;
    Semantics::ValidateReturn validateReturn;
    if (!validateReturn.programValidate(programNode))
        return false;
    Semantics::LvalueVerification lvalueVerification(programNode);
    return lvalueVerification.resolve();
}

i32 printParsingAst(const Parsing::Program* program)
{
    Parsing::ASTPrinter printer;
    std::cout << printer.print(*program);
    return 0;
}

i32 lex(std::vector<Lexing::Token> &lexemes, const std::string& inputFile)
{
    const std::string source = preProcess(inputFile);
    Lexing::Lexer lexer(source);
    if (const i32 err = lexer.getLexemes(lexemes); err != 0)
        return err;
    return 0;
}

bool parse(const std::vector<Lexing::Token>& tokens, Parsing::Program& programNode)
{
    Parsing::Parse parser(tokens);
    if (!parser.programParse(programNode))
        return false;
    return true;
}

Ir::Program ir(const Parsing::Program* parsingProgram)
{
    Ir::Program irProgram;
    Ir::program(parsingProgram, irProgram);
    return irProgram;
}


static std::string preProcess(const std::string &file)
{
    const std::filesystem::path inputFile(file);
    const std::filesystem::path generatedFilesDir = std::filesystem::path(PROJECT_ROOT_DIR) / "generated_files";
    const std::filesystem::path generatedFile = generatedFilesDir / (inputFile.stem().string() + ".i");
    std::string command = "gcc -E -P ";
    command += inputFile.string();
    command += " -o ";
    command += generatedFile.string();
    system(command.c_str());
    return getSourceCode(generatedFile.string());
}