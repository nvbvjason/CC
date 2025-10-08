#include "CompilerDriver.hpp"
#include "AsmPrinter.hpp"
#include "FrontendDriver.hpp"
#include "ASTIr.hpp"
#include "IrPrinter.hpp"
#include "AsmAST.hpp"
#include "GenerateAsmTree.hpp"
#include "Assembly.hpp"
#include "PseudoRegisterReplacer.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <format>

static void printIr(const Ir::Program& irProgram);
static CodeGen::Program codegen(const Ir::Program& irProgram);
static bool fileExists(const std::filesystem::path& name);
static bool isCommandLineArgumentValid(const std::string& argument);
static void assemble(const std::string& asmFile, const std::string& outputFile);
static void linkLib(const std::string& asmFile, const std::string& outputFile, const std::string& argument);
static void makeLib(const std::string& asmFile, const std::string& outputFile);
void fixAsm(const CodeGen::Program& codegenProgram);

i32 CompilerDriver::run()
{
    StateCode code = wrappedRun();
    if (code == StateCode::Done)
        return 0;
    std::cerr << to_string(code) << '\n' << std::flush;
    return static_cast<i32>(code);
}

StateCode CompilerDriver::wrappedRun()
{
    std::string argument;
    if (StateCode errorCode = validateAndSetArg(argument); errorCode != StateCode::Continue)
        return errorCode;
    const std::string inputFile = m_args.back();
    std::vector<Lexing::Token> tokens;
    FrontendDriver frontend(argument, inputFile);
    auto [irProgramOptional, err] = frontend.run();
    if (!irProgramOptional.has_value())
        return StateCode::ERROR_UNKNOWN;
    Ir::Program irProgram = std::move(irProgramOptional.value());
    if (err != StateCode::Continue)
        return err;
    if (argument == "--tacky")
        return StateCode::Done;
    if (argument == "--printTacky") {
        printIr(irProgram);
        return StateCode::Done;
    }
    CodeGen::Program codegenProgram = codegen(irProgram);
    if (argument == "--codegen")
        return StateCode::Done;
    if (argument == "--printAsm") {
        CodeGen::AsmPrinter printer;
        std::cout << printer.printProgram(codegenProgram);
        return StateCode::Done;
    }
    fixAsm(codegenProgram);
    if (argument == "--printAsmAfter") {
        CodeGen::AsmPrinter printer;
        std::cout << printer.printProgram(codegenProgram);
        return StateCode::Done;
    }
    std::string output = CodeGen::asmProgram(codegenProgram);
    if (StateCode errorCode = writeAssmFile(inputFile, output, argument); errorCode != StateCode::Done)
        return errorCode;
    if (argument == "--assemble")
        return StateCode::Done;
    if (argument == "-c")
        makeLib(m_outputFileName, inputFile.substr(0, inputFile.length() - 2));
    else if (argument.starts_with("-l"))
        linkLib(m_outputFileName, inputFile.substr(0, inputFile.length() - 2), argument);
    else
        assemble(m_outputFileName, inputFile.substr(0, inputFile.length() - 2));
    return StateCode::Done;
}

StateCode CompilerDriver::writeAssmFile(const std::string& inputFile, const std::string& output, const std::string& argument)
{
    std::string stem = std::filesystem::path(inputFile).stem().string();
    const std::string inputFolder = std::filesystem::path(inputFile).parent_path().string();
    std::filesystem::path inputPath(inputFile);
    std::filesystem::path outputPath = inputPath.parent_path() / (inputPath.stem().string() + ".s");
    m_outputFileName = outputPath.string();
    std::ofstream ofs(m_outputFileName);
    if (!ofs) {
        std::cerr << "Error: Could not open output file " << m_outputFileName << '\n';
        return StateCode::AsmFileWrite;
    }
    ofs << output;
    ofs.close();
    return StateCode::Done;
}

StateCode CompilerDriver::validateAndSetArg(std::string& argument) const
{
    if (m_args.size() < 2 || 3 < m_args.size()) {
        std::cerr << "Usage: <input_file> possible-argument" << '\n';
        return StateCode::NoInputFile;
    }
    if (const std::filesystem::path m_inputFile(m_args.back()); !fileExists(m_inputFile)) {
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

void fixAsm(const CodeGen::Program& codegenProgram)
{
    for (auto& topLevel : codegenProgram.topLevels) {
        if (topLevel->kind != CodeGen::TopLevel::Kind::Function)
            continue;
        const auto function = dynamic_cast<CodeGen::Function*>(topLevel.get());
        const i32 stackAlloc = CodeGen::replacingPseudoRegisters(*function);
        CodeGen::fixUpInstructions(*function, stackAlloc);
    }
}

CodeGen::Program codegen(const Ir::Program& irProgram)
{
    CodeGen::Program codegenProgram;
    CodeGen::GenerateAsmTree generateAsmTree;
    generateAsmTree.genProgram(irProgram, codegenProgram);
    return codegenProgram;
}

static bool fileExists(const std::filesystem::path& name)
{
    return std::filesystem::exists(name);
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

void assemble(const std::string& asmFile, const std::string& outputFile)
{
    const std::string command = "gcc " + asmFile + " -o " + outputFile;
    std::system(command.c_str());
}

void linkLib(const std::string& asmFile, const std::string& outputFile, const std::string& argument)
{
    const std::string command = "gcc " + asmFile + " -o " + outputFile + " " + argument;
    std::system(command.c_str());
}

void makeLib(const std::string& asmFile, const std::string& outputFile)
{
    const std::string command = "gcc -c " + asmFile + " -o " + outputFile + ".o";
    std::system(command.c_str());
}