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
static void makeLib(const std::string& asmFile, const std::string& outputFile);
void fixAsm(const CodeGen::Program& codegenProgram);
static void cleanUp();

i32 CompilerDriver::run()
{
    cleanUp();
    ErrorCode code = wrappedRun();
    if (code == ErrorCode::OK)
        return 0;
    std::cerr << to_string(code) << '\n' << std::flush;
    return static_cast<i32>(code);
}

ErrorCode CompilerDriver::wrappedRun()
{
    std::string argument;
    if (ErrorCode errorCode = validateAndSetArg(argument); errorCode != ErrorCode::OK)
        return errorCode;
    const std::string inputFile = m_args.back();
    std::vector<Lexing::Token> tokens;
    SymbolTable symbolTable;
    FrontendDriver frontend(argument, inputFile, symbolTable);
    auto [irProgram, err] = frontend.run();
    if (err != ErrorCode::OK)
        return err;
    if (argument == "--tacky")
        return ErrorCode::OK;
    if (argument == "--printTacky") {
        printIr(irProgram);
        return ErrorCode::OK;
    }
    CodeGen::Program codegenProgram = codegen(irProgram);
    if (argument == "--codegen")
        return ErrorCode::OK;
    if (argument == "--printAsm") {
        CodeGen::AsmPrinter printer;
        std::cout << printer.printProgram(codegenProgram);
        return ErrorCode::OK;
    }
    fixAsm(codegenProgram);
    if (argument == "--printAsmAfter") {
        CodeGen::AsmPrinter printer;
        std::cout << printer.printProgram(codegenProgram);
        return ErrorCode::OK;
    }
    if (argument == "--printAsm") {
        CodeGen::AsmPrinter printer;
        std::cout << printer.printProgram(codegenProgram);
        return ErrorCode::OK;
    }
    std::string output = CodeGen::asmProgram(codegenProgram);
    if (ErrorCode errorCode = writeAssmFile(inputFile, output, argument); errorCode != ErrorCode::OK)
        return errorCode;
    if (argument == "--assemble")
        return ErrorCode::OK;
    if (argument == "-c")
        makeLib(m_outputFileName, inputFile.substr(0, inputFile.length() - 2));
    else
        assemble(m_outputFileName, inputFile.substr(0, inputFile.length() - 2));
    return ErrorCode::OK;
}

ErrorCode CompilerDriver::writeAssmFile(const std::string& inputFile, const std::string& output, const std::string& argument)
{
    std::string stem = std::filesystem::path(inputFile).stem().string();
    const std::string inputFolder = std::filesystem::path(inputFile).parent_path().string();
    m_outputFileName = std::format("{}/{}.s", inputFolder, stem);
    std::ofstream ofs(m_outputFileName);
    if (!ofs) {
        std::cerr << "Error: Could not open output file " << m_outputFileName << '\n';
        return ErrorCode::AsmFileWrite;
    }
    ofs << output;
    ofs.close();
    return ErrorCode::OK;
}

ErrorCode CompilerDriver::validateAndSetArg(std::string& argument) const
{
    if (m_args.size() < 2 || 3 < m_args.size()) {
        std::cerr << "Usage: <input_file> possible-argument" << '\n';
        return ErrorCode::NoInputFile;
    }
    if (const std::filesystem::path m_inputFile(m_args.back()); !fileExists(m_inputFile)) {
        std::cerr << "File " << m_inputFile.string() << " not found" << '\n';
        return ErrorCode::FileNotFound;
    }
    if (m_args.size() == 3)
        argument = m_args[1];
    if (!isCommandLineArgumentValid(argument)) {
        std::cerr << "Invalid argument: " << argument << '\n';
        return ErrorCode::InvalidCommandlineArgs;
    }
    return ErrorCode::OK;
}

void cleanUp()
{
    const std::filesystem::path generatedDir = std::filesystem::path(PROJECT_ROOT_DIR) / "generated_files";
    for (const auto& entry : std::filesystem::directory_iterator(generatedDir)) {
        if (entry.path().filename() == ".gitkeep")
            continue;
        try {
            if (std::filesystem::is_directory(entry.path()))
                std::filesystem::remove_all(entry.path());
            else
                std::filesystem::remove(entry.path());
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error deleting " << entry.path() << ": " << e.what() << '\n';
        }
    }
}

void printIr(const Ir::Program& irProgram)
{
    Ir::IrPrinter printer;
    std::cout << printer.print(irProgram);
}

void fixAsm(const CodeGen::Program& codegenProgram)
{
    for (auto& topLevel : codegenProgram.topLevels) {
        if (topLevel->type == CodeGen::TopLevel::Type::StaticVariable)
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
    generateAsmTree.generateProgram(irProgram, codegenProgram);
    return codegenProgram;
}

static bool fileExists(const std::filesystem::path& name)
{
    return std::filesystem::exists(name);
}

static bool isCommandLineArgumentValid(const std::string &argument)
{
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

void makeLib(const std::string& asmFile, const std::string& outputFile)
{
    const std::string command = "gcc -c " + asmFile + " -o " + outputFile + ".o";
    std::system(command.c_str());
}