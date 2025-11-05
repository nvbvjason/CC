#include "CompilerDriver.hpp"
#include "FrontendDriver.hpp"
#include "ASTIr.hpp"
#include "IrPrinter.hpp"
#include "GenerateAsmTree.hpp"
#include "CodeGenDriver.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <format>

static void printIr(const Ir::Program& irProgram);
static bool isCommandLineArgumentValid(const std::string& argument);
static void printHelp();

i32 CompilerDriver::run() const
{
    StateCode code = wrappedRun();
    if (code == StateCode::Done)
        return 0;
    std::cerr << to_string(code) << '\n' << std::flush;
    return static_cast<i32>(code);
}

StateCode CompilerDriver::wrappedRun() const
{
    std::string argument;
    if (const StateCode errorCode = validateAndSetArg(argument); errorCode != StateCode::Continue)
        return errorCode;
    if (argument == "--help" || argument == "-h") {
        printHelp();
        return StateCode::Done;
    }
    const std::string inputFile = m_args.back();
    FrontendDriver frontend(argument, inputFile);
    auto [irProgramOptional, err] = frontend.run();
    if (!irProgramOptional.has_value())
        return err;
    const Ir::Program irProgram = std::move(irProgramOptional.value());
    if (err != StateCode::Continue)
        return err;
    if (argument == "--tacky")
        return StateCode::Done;
    if (argument == "--printTacky") {
        printIr(irProgram);
        return StateCode::Done;
    }
    CodeGen::run(irProgram, argument, inputFile);
    return StateCode::Done;
}

StateCode CompilerDriver::validateAndSetArg(std::string& argument) const
{
    if (m_args.size() < 2 || 3 < m_args.size()) {
        std::cerr << "Usage: possible-argument <input_file>" << '\n';
        return StateCode::NoInputFile;
    }
    if (const std::filesystem::path m_inputFile(m_args.back()); !std::filesystem::exists(m_inputFile)) {
        std::cerr << "File " << m_inputFile.string() << " not found" << '\n';
        return StateCode::FileNotFound;
    }
    if (m_args.size() == 3)
        argument = m_args[1];
    if (!isCommandLineArgumentValid(argument)) {
        std::cerr << "Invalid argument: " << argument << '\n';
        return StateCode::InvalidCommandlineArgs;
    }
    return StateCode::Continue;
}

void printIr(const Ir::Program& irProgram)
{
    Ir::IrPrinter printer;
    std::cout << printer.print(irProgram);
}

static bool isCommandLineArgumentValid(const std::string &argument)
{
    if (argument.starts_with("-l"))
        return true;
    constexpr std::array validArguments = {"",  "--printAst","--help", "-h", "--version",
        "--lex", "--parse", "--tacky", "--codegen", "--printTacky", "--validate",
        "--assemble", "--printAsm", "--printAsmAfter", "-c", "--printAstAfter", "--printTokens"};
    return std::ranges::contains(validArguments, argument);
}

static void printHelp()
{
    const auto helpText =
        "-h               - Print help to the console.\n"
        "--printTokens    - Print the tokens produced by the lexer.\n"
        "--printAst       - Print the abstract syntax tree.\n"
        "--printAstAfter  - Print the converted abstract syntax tree after Semantic analysis.\n"
        "--printTacky     - Print the intermediate representation.\n"
        "--printAsm       - Print the assembly representation before register fixes.\n"
        "--printAsmAfter  - Print the assembly representation after register fixes.\n"
        "--lex            - Stop after the lexing stage.\n"
        "--parse          - Stop after the parsing stage.\n"
        "--codegen        - Stop after the writing the assembly file.\n"
    ;
    std::cout << helpText << '\n';
}