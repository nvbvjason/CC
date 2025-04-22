#pragma once

#ifndef CC_PARSING_CONCRETE_TREE_HPP
#define CC_PARSING_CONCRETE_TREE_HPP

/*
    <program> ::= <function>
    <function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
    <statement> ::= "return" <exp> ";"
    <exp> ::= <int> | <unop> <exp> | "(" <exp> ")"
    <unop> = ::= "-" | "~"
    <identifier> ::= ? An identifier token ?
    <int> ::= ? A constant token ?
*/

#include "AbstractTree.hpp"
#include "Lexing/Token.hpp"

#include <vector>

namespace Parsing {

namespace ErrorCodes {
    constexpr i32 SUCCESS = 0;
    constexpr i32 MAIN_FUNCTION_EXPECTED = -1;
    constexpr i32 UNEXPECTED_TOKEN = -2;
    constexpr i32 RECURSION_DEPTH_EXCEEDED = -3;
}

class ParseError final : public std::runtime_error {
    size_t m_position;
    Lexing::TokenType m_expected = Lexing::TokenType::Invalid;
    Lexing::TokenType m_received = Lexing::TokenType::Invalid;
public:
    ParseError(const std::string& message,
               const size_t position,
               const Lexing::TokenType expected,
               const Lexing::TokenType received)
        : std::runtime_error(message),
          m_position(position),
          m_expected(expected),
          m_received(received) {}
    ParseError(const std::string& message, const size_t position)
        : std::runtime_error(message),
          m_position(position) {}
    [[nodiscard]] size_t position() const noexcept { return m_position; }
    [[nodiscard]] Lexing::TokenType expected() const noexcept { return m_expected; }
    [[nodiscard]] Lexing::TokenType received() const noexcept { return m_received; }
};

class Parse {
    std::vector<Lexing::Token> c_tokens;
    size_t m_current = 0;
public:
    Parse() = delete;
    explicit Parse(const std::vector<Lexing::Token> &c_tokens)
        : c_tokens(c_tokens) {}
    i32 programParse(Program& program);
    std::pair<Function*, i32> functionParse();
    std::pair<Statement*, i32> statementParse();
    std::pair<Expression*, i32> expressionParse(u16 depth);
    std::pair<Unary*, i32> unaryParse();
    std::pair<UnaryOperator, i32> unaryOperatorParse();
private:
    size_t match(const Lexing::TokenType& type);
    [[nodiscard]] Lexing::Token peek() const { return c_tokens[m_current]; }
    Lexing::Token advance()
    {
        if (c_tokens.size() <= m_current)
            throw ParseError("Unexpected EOF", m_current);
        return c_tokens[m_current++];
    }
    [[nodiscard]] bool expect(const Lexing::TokenType type)
    {
        if (peek().m_type == type) {
            advance();
            return true;
        }
        return false;
    }
};
} // Parsing

#endif // CC_PARSING_CONCRETE_TREE_HPP
