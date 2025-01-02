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

enum class UnaryType {
    Complement, Negate
};

enum class ExpressionType {
    Constant, Unary
};

struct ExpressionNode {
    ExpressionType type;
    union {
        i32 constant;
        struct UnaryNode {
            UnaryType type;
            ExpressionNode *left;
        };
    } value;
    ExpressionNode()
        : type(ExpressionType::Constant)
    {
        value.constant = 0;
    }
};

struct StatementNode {
    ExpressionNode constant;
};

struct FunctionNode {
    std::string name;
    StatementNode body;
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
    i32 parseStatement(StatementNode& statement);
    i32 parseExpression(ExpressionNode& expression);

    [[nodiscard]] i32 expect(Lexing::TokenType type);
    [[nodiscard]] bool isAtEnd() const { return m_current == c_tokens.size() - 1; }
    [[nodiscard]] Lexing::Token peek() const { return c_tokens[m_current]; }
    [[nodiscard]] static bool is_number(const std::string& s)
    {
        return !s.empty() && std::ranges::find_if(s,[](const unsigned char c) {
            return !std::isdigit(c);
        }) == s.end();
    }
};
}

#endif // PARSING_PARSER_HPP
