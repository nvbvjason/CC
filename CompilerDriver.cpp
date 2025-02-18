#include "CompilerDriver.hpp"
#include "Lexing/Lexer.hpp"
#include "Codegen/Assembly.hpp"
#include "Parsing/ConcreteTree.hpp"
#include "Parsing/AstVisualizer.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

#include "Tacky/AbstractTree.hpp"
#include "Tacky/ConcreteTree.hpp"

static i32 lex(std::vector<Lexing::Token>& lexemes, const std::string& inputFile);
static i32 parse(const std::vector<Lexing::Token>& tokens, Parsing::Program& programNode);
static void tacky(const Parsing::Program& parsingProgram, Tacky::Program& tackyProgram);
static void codegen(const Parsing::Program& programNode, std::string& output);
static bool fileExists(const std::string &name);
static bool isCommandLineArgumentValid(const std::string &argument);
static std::string preProcess(const std::string &file);
static void assemble(const std::string& asmFile);

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
    if (const i32 err = parse(tokens, program); err != 0)
        return err;
    if (argument == "--parse")
        return 0;
    if (argument == "--printAst") {
        std::cout << astVisualizer(program) << '\n';
        return 0;
    }
    Tacky::Program tackyProgram;
    tacky(program, tackyProgram);
    if (argument == "--tacky")
        return 0;
    std::string output;
    codegen(program, output);
    if (argument == "--codegen")
        return 0;
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
    if (const i32 err = lexer.getLexems(lexemes); err != 0)
        return err;
    return 0;
}

i32 parse(const std::vector<Lexing::Token>& tokens, Parsing::Program& programNode)
{
    Parsing::Parse parser(tokens);
    if (const i32 err = parser.programParse(programNode); err != 0)
        return err;
    return 0;
}

void tacky(const Parsing::Program& parsingProgram, Tacky::Program& tackyProgram)
{
    const Parsing::Program *parsingProgramPtr = &parsingProgram;
    programTacky(parsingProgramPtr, tackyProgram);
}

void codegen(const Parsing::Program& programNode, std::string &output)
{
    const Parsing::Program* temp = &programNode;
    const Codegen::Assembly astToAssembly(temp);
    astToAssembly.getOutput(output);
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
    std::string command = "gcc -E -P ";
    command += inputFile.string();
    command += " -o ";
    const std::string generatedFilesDir = "/home/jason/src/CC/generated_files/";
    const std::string generatedFileName = generatedFilesDir + inputFile.stem().string() + ".i";
    command += generatedFileName;
    system(command.c_str());
    return getSourceCode(generatedFileName);
}

i32 assemble(const std::string &asmFile)
{
    std::string stem = std::filesystem::path().stem();
    const std::string outputFileName = std::format("/home/jason/src/CC/AssemblyFiles/{}.s", stem);
    std::ofstream ofs(outputFileName);
    ofs << asmFile;
    ofs.close();
    return 0;
}