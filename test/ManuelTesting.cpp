#include "ManuelTesting.h"
#include "../Lexing/Lexer.h"

#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>

void chapter_1()
{
    std::string invalid = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/invalid_lex";
    checkTokens(invalid);
    std::string invalidParse = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/invalid_parse";
    checkTokens(invalidParse);
    std::string valid = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/valid";
    checkTokens(valid);
}

bool allTokensValid(const std::vector<Lexing::Token> &tokens)
{
    for (const auto& token : tokens)
        if (token.type == Lexing::TokenType::INVALID)
            return false;
    return true;
}

void checkTokens(const std::string& directory)
{
    for (const auto &path : std::filesystem::directory_iterator(directory)) {
        std::ifstream file(path.path());
        std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        Lexing::Lexer lexer(source);
        std::vector<Lexing::Token> tokens = lexer.tokenize();
        if (allTokensValid(tokens))
            std::cout << "0  " << path.path() << '\n';
        else
            std::cout << "4  " << path.path() << '\n';
    }
    std::cout << '\n';
}