#include "CompilerDriver.hpp"
#include "Lexing/Lexer.hpp"
#include "Parsing/ConcreteTree.hpp"
#include "Parsing/AstVisualizer.hpp"
#include "IR/AbstractTree.hpp"
#include "IR/ConcreteTree.hpp"
#include "CodeGen/AbstractTree.hpp"
#include "CodeGen/ConcreteTree.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

static i32 lex(std::vector<Lexing::Token>& lexemes, const std::string& inputFile);
static bool parse(const std::vector<Lexing::Token>& tokens, Parsing::Program& programNode);
static Ir::Program ir(const Parsing::Program& parsingProgram);
static CodeGen::Program codegen(const Ir::Program& irProgram);
static bool fileExists(const std::string &name);
static bool isCommandLineArgumentValid(const std::string &argument);
static std::string preProcess(const std::string &file);
static i32 assemble(const std::string& asmFile);

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
    if (const i32 err = lex(tokens, inputFile) != 0)
        return err;
    if (argument == "--lex")
        return 0;
    Parsing::Program program;
    if (!parse(tokens, program))
        return 1;
    if (argument == "--parse")
        return 0;
    if (argument == "--printAst") {
        Parsing::Visualizer visualizer;
        std::cout << visualizer.visualize(program) << '\n';
        return 0;
    }
    Ir::Program irProgram = ir(program);
    if (argument == "--tacky")
        return 0;
    CodeGen::Program codegenProgram = codegen(irProgram);
    if (argument == "--codegen")
        return 0;
    std::string output;
    std::string stem = std::filesystem::path(inputFile).stem();
    std::string outputFileName = std::format("/home/jason/src/CC/AssemblyFiles/{}.s", stem);
    std::ofstream ofs(outputFileName);
    ofs << output;
    ofs.close();
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

Ir::Program ir(const Parsing::Program& parsingProgram)
{
    Ir::Program irProgram;
    Ir::program(&parsingProgram, irProgram);
    return irProgram;
}

CodeGen::Program codegen(const Ir::Program& irProgram)
{
    CodeGen::Program codegenProgram;
    CodeGen::program(irProgram, codegenProgram);
    CodeGen::replacingPseudoRegisters(codegenProgram);
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
        "--lex", "--parse", "--tacky", "--codegen"};
    return std::any_of(validArguments.begin(), validArguments.end(), [&](const std::string &arg) {
        return arg == argument;
    });
}

std::string getSourceCode(const std::string &inputFile)
{
    std::ifstream file(inputFile);
    std::string source((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return source;
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

i32 assemble(const std::string &asmFile)
{
    const auto outputDir = std::filesystem::path(PROJECT_ROOT_DIR) / "AssemblyFiles";
    std::filesystem::create_directories(outputDir);
    const auto outputFile = outputDir / std::format("{}.s", std::filesystem::path(asmFile).stem().string());
    std::ofstream ofs(outputFile);
    if (!ofs) {
        std::cerr << std::format("Error writing to {}\n", outputFile.string());
        return -1;
    }
    ofs << asmFile;
    return 0;
}