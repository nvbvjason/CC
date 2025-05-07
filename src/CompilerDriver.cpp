#include "CompilerDriver.hpp"

#include "Frontend/FrontendDriver.hpp"

#include "IR/ASTIr.hpp"
#include "IR/Printer.hpp"

#include "CodeGen/AsmAST.hpp"
#include "CodeGen/GenerateAsmTree.hpp"
#include "CodeGen/Assembly.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

static void printIr(const Ir::Program& irProgram);
static CodeGen::Program codegen(const Ir::Program& irProgram);
static bool fileExists(const std::string &name);
static bool isCommandLineArgumentValid(const std::string &argument);
static void assemble(const std::string& asmFile, const std::string& outputFile);
static void makeLib(const std::string& asmFile, const std::string& outputFile);
static void cleanUp();

ErrorCode CompilerDriver::run()
{
    std::string argument;
    ErrorCode value1;
    if (validateCommandLineArguments(argument, value1))
        return value1;
    cleanUp();
    const std::string inputFile = args.back();
    std::vector<Lexing::Token> tokens;
    FrontendDriver frontend(argument, inputFile);
    auto [irProgram, err] = frontend.run();
    if (err != ErrorCode::OK)
        return err;
    if (irProgram == nullptr)
        return ErrorCode::OK;
    if (argument == "--tacky")
        return ErrorCode::OK;
    if (argument == "--printTacky") {
        printIr(*irProgram);
        return ErrorCode::OK;
    }
    CodeGen::Program codegenProgram = codegen(*irProgram);
    if (argument == "--codegen")
        return ErrorCode::OK;
    std::string output = CodeGen::asmProgram(codegenProgram);
    writeAssmFile(inputFile, output);
    if (argument == "-c")
        makeLib(m_outputFileName, inputFile.substr(0, inputFile.length() - 2));
    else
        assemble(m_outputFileName, inputFile.substr(0, inputFile.length() - 2));
    return ErrorCode::OK;
}
void CompilerDriver::writeAssmFile(const std::string& inputFile, const std::string& output)
{
    std::string stem = std::filesystem::path(inputFile).stem();
    m_outputFileName = std::format("/home/jason/src/CC/generated_files/{}.s", stem);
    std::ofstream ofs(m_outputFileName);
    ofs << output;
    ofs.close();
}

bool CompilerDriver::validateCommandLineArguments(std::string& argument, ErrorCode& value1) const
{
    if (args.size() < 2 || 3 < args.size()) {
        std::cerr << "Usage: <input_file> possible-argument" << '\n';
        value1 = ErrorCode::NoInputFile;
        return true;
    }
    if (const std::filesystem::path m_inputFile(args.back()); !fileExists(m_inputFile)) {
        std::cerr << "File " << m_inputFile.string() << " not found" << '\n';
        value1 = ErrorCode::FileNotFound;
        return true;
    }
    if (args.size() == 3)
        argument = args[1];
    if (!isCommandLineArgumentValid(argument)) {
        std::cerr << "Invalid argument: " << argument << '\n';
        value1 = ErrorCode::InvalidCommandlineArgs;
        return true;
    }
    return false;
}

void cleanUp()
{
    const std::filesystem::path generatedDir = std::filesystem::path(PROJECT_ROOT_DIR) / "generated_files";
    for (const auto& entry : std::filesystem::directory_iterator(generatedDir))
        remove_all(entry.path());
}

void printIr(const Ir::Program& irProgram)
{
    Ir::Printer printer;
    std::cout << printer.print(irProgram);
}

CodeGen::Program codegen(const Ir::Program& irProgram)
{
    CodeGen::Program codegenProgram;
    CodeGen::program(irProgram, codegenProgram);
    const i32 stackAlloc = CodeGen::replacingPseudoRegisters(codegenProgram);
    CodeGen::fixUpInstructions(codegenProgram, stackAlloc);
    return codegenProgram;
}

static bool fileExists(const std::string &name)
{
    const std::ifstream ifs(name.c_str());
    return ifs.good();
}

static bool isCommandLineArgumentValid(const std::string &argument)
{
    const std::vector<std::string> validArguments = {"",  "--printAst","--help", "-h", "--version",
        "--lex", "--parse", "--tacky", "--codegen", "--printTacky", "--validate"};
    return std::any_of(validArguments.begin(), validArguments.end(), [&](const std::string &arg) {
        return arg == argument;
    });
}

void assemble(const std::string& asmFile, const std::string& outputFile)
{
    const std::string command = "gcc " + asmFile + "-o" + outputFile;
    system(command.c_str());
}

void makeLib(const std::string& asmFile, const std::string& outputFile)
{
    const std::string command = "gcc -c" + asmFile + "-o" + outputFile;
    system(command.c_str());
}