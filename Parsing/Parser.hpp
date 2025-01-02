#pragma once

#ifndef PARSING_PARSER_HPP
#define PARSING_PARSER_HPP

#include "../Lexing/Token.hpp"
#include "../ShortTypes.hpp"

#include <vector>

/*
    program = Program(function_definition)
    function_definition = Function(identifier names, statement body)
    statement = Return(exp)
    exp = Constant(int) | Unary(unary_operator, exp)
    unary_operator = Complement | Negate
*/

/*
    <program> ::= <function>
    <function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
    <statement> ::= "return" <exp> ";"
    <exp> ::= <int> | <unop> <exp> | "(" <exp> ")"
    <unop> = ::= "-" | "~"
    <identifier> ::= ? An identifier token ?
    <int> ::= ? A constant token ?
*/

namespace Parsing {

struct ConstantNode {
    i32 constant;
};

struct ReturnNode {
    ConstantNode expression;
};

struct FunctionNode {
    std::string name;
    ReturnNode body;
};

struct ProgramNode {
    FunctionNode function;
};

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

    i32 parseProgram(ProgramNode& program);
private:
    i32 parseFunction(FunctionNode& function);
    i32 parseStatement(ReturnNode& statement);
    i32 parseExpression(ConstantNode& expression);

    [[nodiscard]] i32 expect(Lexing::TokenType type);
    [[nodiscard]] bool isAtEnd() const { return m_current == c_tokens.size() - 1; }
    [[nodiscard]] Lexing::Token peek() const { return c_tokens[m_current]; }
    [[nodiscard]] Lexing::Token match(Lexing::TokenType expected);
    [[nodiscard]] static bool is_number(const std::string& s)
    {
        return !s.empty() && std::ranges::find_if(s,[](const unsigned char c) {
            return !std::isdigit(c);
        }) == s.end();
    }
};
}

#endif // PARSING_PARSER_HPP
