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

int CompilerDriver::run() const
{
    if (args.size() < 2 || 3 < args.size()) {
        std::cerr << "Usage: <input_file> possible-argument" << '\n';
        return 1;
    }
    if (const std::filesystem::path m_inputFile(args.back()); !fileExists(m_inputFile)) {
        std::cerr << "File " << m_inputFile.string() << " not found" << '\n';
        return 2;
    }
    std::string argument;
    if (args.size() == 3)
        argument = args[1];
    if (!isCommandLineArgumentValid(argument)) {
        std::cerr << "Invalid argument: " << argument << '\n';
        return 3;
    }
    const std::string inputFile = args.back();
    std::vector<Lexing::Token> tokens;
    FrontendDriver frontend(argument, inputFile);
    auto [irProgram, err] = frontend.run();
    if (err != 0)
        return err;
    if (irProgram == nullptr)
        return 0;
    if (argument == "--tacky")
        return 0;
    if (argument == "--printTacky") {
        printIr(*irProgram);
        return 0;
    }
    CodeGen::Program codegenProgram = codegen(*irProgram);
    if (argument == "--codegen")
        return 0;
    std::string output = CodeGen::asmProgram(codegenProgram);
    std::string stem = std::filesystem::path(inputFile).stem();
    std::string outputFileName = std::format("/home/jason/src/CC/generated_files/{}.s", stem);
    std::ofstream ofs(outputFileName);
    ofs << output;
    ofs.close();
    if (argument == "-c")
        makeLib(outputFileName, inputFile.substr(0, inputFile.length() - 2));
    else
        assemble(outputFileName, inputFile.substr(0, inputFile.length() - 2));
    return 0;
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