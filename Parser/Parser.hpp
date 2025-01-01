#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>

#include "ProgramNode.hpp"
#include "FunctionNode.hpp"
#include "StatementNode.hpp"
#include "ExpressionNode.hpp"
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

    ProgramNode parseProgram();
private:
    FunctionNode parseFunction();
    StatementNode parseStatement();
    ExpressionNode parseExpression();

    void expect(Lexing::TokenType expected);
    void advance();
    [[nodiscard]] bool isAtEnd() const { return m_current == c_tokens.size() - 1; }
    [[nodiscard]] Lexing::Token peek() const { return c_tokens[m_current]; }
    [[nodiscard]] Lexing::Token match(Lexing::TokenType expected);
};
}

#endif //PARSER_HPP
