#include "Program.hpp"
#include "Lexing/Lexer.hpp"

#include <iostream>
#include <fstream>

#include "Codegen/Assembly.hpp"
#include "Parser/Parser.hpp"

int Program::run() const
{
    if (args.size() < 2 || 3 < args.size()) {
        std::cerr << "Usage: <input_file> possible-argument" << '\n';
        return 1;
    }
    if (!fileExists(args[1])) {
        std::cerr << "File " << args[1] << " not found" << '\n';
        return 2;
    }
    const std::string inputFile = args[1];
    if (args.size() == 3) {
        std::string argument = args[2];
        if (!isCommandLineArgumentValid(argument)) {
            std::cerr << "Invalid argument: " << argument << '\n';
            return 3;
        }
        std::ifstream file(inputFile);
        std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        Lexing::Lexer lexer(source);
        std::vector<Lexing::Token> tokens = lexer.tokenize();
        if (argument == "--lex") {
            for (const auto& token : tokens)
                if (token.type == Lexing::TokenType::INVALID)
                    return 4;
            return 0;
        }
        Parsing::Parser parser(tokens);
        Parsing::ProgramNode program;
        if (const Lexing::i32 err = parser.parseProgram(program); err != 0)
            return err;
        if (argument == "--parse")
            return 0;
        if (argument == "--codegen") {
            Codegen::Assembly astToAssembly(program);
            astToAssembly.writeToFile(argument);
        }
    }
    return 0;
}

static bool fileExists(const std::string &name)
{
    std::ifstream f(name.c_str());
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
    std::cout << "\t\t\t" << "Constant(" << program.function.body.expression.constant << ")\n";
    std::cout << "\t\t" << ")\n";
    std::cout << "\t" << ")\n";
    std::cout << ")\n";
}