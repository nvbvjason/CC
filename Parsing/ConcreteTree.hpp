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
#include "../Lexing/Token.hpp"

#include <vector>

namespace Parsing {

class Parse {
    std::vector<Lexing::Token> c_tokens;
    i32 m_current = 0;
public:
    Parse() = delete;
    Parse(const std::vector<Lexing::Token> &c_tokens)
        : c_tokens(c_tokens) {}
    i32 programParse(ProgramNode& program);
    i32 functionParse(FunctionNode* function);
    i32 statementParse(StatementNode* statement);
    i32 expressionParse(ExpressionNode* expression);
    i32 unaryParse(UnaryNode* unary);
    i32 unaryOperatorParse(UnaryOperator& unaryOperator);
    [[nodiscard]] Lexing::Token advance()
    {
        Lexing::Token lexeme = c_tokens[m_current];
        ++m_current;
        return lexeme;
    }
private:
    i32 match(const Lexing::TokenType& type);
    [[nodiscard]] bool expect(const Lexing::TokenType type)
    {
        const bool result = c_tokens[m_current].m_type == type;
        ++m_current;
        return result;
    }
};
} // Parsing

#endif // CC_PARSING_CONCRETE_TREE_HPP
