#include "Parser.hpp"
#include "Parsing/Parser.hpp"

#include <vector>

std::vector<Lexing::Token> generateTokens(const std::vector<Lexing::Token::Type>& tokenTypes)
{
    std::vector<Lexing::Token> tokens;
    tokens.reserve(tokenTypes.size() + 1);
    for (const auto& tokenType : tokenTypes)
        tokens.emplace_back(1, 1, tokenType, "");
    tokens.emplace_back(1, 1, Lexing::Token::Type::EndOfFile, "");
    return tokens;
}

Parsing::Parser createParser(const std::vector<Lexing::Token::Type>& tokenTypes)
{
    const std::vector<Lexing::Token> tokens = generateTokens(tokenTypes);
    return Parsing::Parser(tokens);
}
