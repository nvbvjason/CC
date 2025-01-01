#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>

#include "Program.hpp"
#include "FunctionDefinition.hpp"
#include "Statement.hpp"
#include "Expression.hpp"
#include "../Lexing/Token.hpp"

namespace Parsing {

class Parser {
    std::vector<Lexing::Token> c_tokens;
    i32 m_current = 0;
public:
    Parser(const Parser& other) = delete;
    Parser(Parser&& other) = delete;
    Parser& operator=(const Parser& other) = delete;
    Parser& operator=(Parser&& other) = delete;
    explicit Parser(std::vector<Lexing::Token> tokens)
        : c_tokens(std::move(tokens)) {}

    FunctionDefinition parseProgram();
private:
    FunctionDefinition parseFunction();
    Statement parseStatement();
    Expression parseExpression();

    void expect(Lexing::TokenType expected);
    void advance();
    [[nodiscard]] bool isAtEnd() const { return m_current == c_tokens.size() - 1; }
    [[nodiscard]] Lexing::Token peek() const { return c_tokens[m_current]; }
    [[nodiscard]] Lexing::Token match(Lexing::TokenType expected);
};
}

#endif //PARSER_HPP
