#include "Program.h"
#include "Lexing/Lexer.h"

#include <iostream>
#include <fstream>

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
        if (argument == "--lex") {
            std::ifstream file(inputFile);
            std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            Lexing::Lexer lexer(source);
            std::vector<Lexing::Token> tokens = lexer.tokenize();
            // for (const auto& token : tokens)
            //     std::cout << token << '\n';
            for (const auto& token : tokens)
                if (token.type == Lexing::TokenType::INVALID)
                    return 4;
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
    const std::vector<std::string> validArguements = {"--help", "-h", "--version", "--lex", "--parse", "--codegen"};
    for (const std::string &validArguement : validArguements)
        if (argument == validArguement)
            return true;
    return false;
}