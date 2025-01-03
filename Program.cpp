#include "Program.hpp"
#include "Lexing/Lexer.hpp"
#include "Codegen/Assembly.hpp"
#include "Parsing/Parser.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

int Program::run() const
{
    if (args.size() < 2 || 3 < args.size()) {
        std::cerr << "Usage: <input_file> possible-argument" << '\n';
        return 1;
    }
    const std::string inputFile = args.back();
    if (!fileExists(inputFile)) {
        std::cerr << "File " << inputFile << " not found" << '\n';
        return 2;
    }
    if (args.size() == 3)
        return runThreeArgs(inputFile);
    if (args.size() == 2)
        return runTwoArgs(inputFile);
    return 0;
}

i32 Program::runTwoArgs(const std::string& inputFile)
{
    std::vector<Lexing::Lexeme> tokens;
    if (const i32 err = lex(tokens, inputFile) != 0)
        return err;
    Parsing::ProgramNode program;
    if (const i32 err = parse(tokens, program); err != 0)
        return err;
    std::string output;
    if (const i32 err = codegen(program, output); err != 0)
        return err;
    std::string stem = std::filesystem::path(inputFile).stem();
    std::string outputFileName = std::format("/home/jason/src/CC/AssemblyFiles/{}.s", stem);
    std::ofstream ofs(outputFileName);
    ofs << output;
    ofs.close();
    return 0;
}

i32 Program::runThreeArgs(const std::string& inputFile) const
{
    std::string argument = args[1];
    if (!isCommandLineArgumentValid(argument)) {
        std::cerr << "Invalid argument: " << argument << '\n';
        return 3;
    }
    std::vector<Lexing::Lexeme> tokens;
    if (const i32 err = lex(tokens, inputFile) != 0)
        return err;
    if (argument == "--lex")
        return 0;
    Parsing::ProgramNode program;
    if (const i32 err = parse(tokens, program); err != 0)
        return err;
    if (argument == "--parse")
        return 0;
    std::string output;
    if (const i32 err = codegen(program, output); err != 0)
        return err;
    if (argument == "--codegen") {
        Codegen::Assembly astToAssembly(program);
        astToAssembly.getOutput(output);
    }
    return 0;
}

i32 lex(std::vector<Lexing::Lexeme> &tokens, const std::string& inputFile)
{
    std::ifstream file(inputFile);
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Lexing::Lexer lexer(source);
    tokens = lexer.tokenize();
    for (const auto& token : tokens)
        if (token.m_type == Lexing::LexemeType::Invalid)
            return 4;
    return 0;
}

i32 parse(const std::vector<Lexing::Lexeme>& tokens, Parsing::ProgramNode& programNode)
{
    Parsing::Parser parser(tokens);
    if (const i32 err = parser.parseProgram(programNode); err != 0)
        return err;
    return 0;
}

i32 codegen(const Parsing::ProgramNode &programNode, std::string &output)
{
    const Codegen::Assembly astToAssembly(programNode);
    astToAssembly.getOutput(output);
    return 0;
}

static bool fileExists(const std::string &name)
{
    const std::ifstream f(name.c_str());
    return f.good();
}

static bool isCommandLineArgumentValid(const std::string &argument)
{
    const std::vector<std::string> validArguments = {"--help", "-h", "--version", "--lex", "--parse", "--codegen"};
    return std::any_of(validArguments.begin(), validArguments.end(), [&](const std::string &arg) {
        return arg == argument;
    });
}

static void astPrinter(const Parsing::ProgramNode& program)
{
    std::cout << "Program(\n";
    std::cout << "\t" << "Function(\n";
    std::cout << "\t\t" << "name=" << program.function.name << ",\n";
    std::cout << "\t\t" << "body=Return" << program.function.name << "(\n";
    std::cout << "\t\t\t" << "Constant(" << program.function.body.constant.value.constant << ")\n";
    std::cout << "\t\t" << ")\n";
    std::cout << "\t" << ")\n";
    std::cout << ")\n";
}