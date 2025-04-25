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

class Parse {
    std::vector<Lexing::Token> c_tokens;
    size_t m_current = 0;
public:
    Parse() = delete;
    explicit Parse(const std::vector<Lexing::Token> &c_tokens)
        : c_tokens(c_tokens) {}
    [[nodiscard]] bool programParse(Program& program);
    [[nodiscard]] bool functionParse(Function& function);
    [[nodiscard]] bool statementParse(Statement& statement);
    [[nodiscard]] std::shared_ptr<Expr> expressionParse();
    [[nodiscard]] std::shared_ptr<UnaryExpr> unaryParse();
    [[nodiscard]] bool unaryOperatorParse(UnaryExpr::Operator& unaryOperator);
private:
    bool match(const Lexing::TokenType& type);
    [[nodiscard]] Lexing::Token peek() const { return c_tokens[m_current]; }
    [[nodiscard]] Lexing::Token advance() { return c_tokens[m_current++]; }
    [[nodiscard]] bool expect(const Lexing::TokenType type)
    {
        if (peek().m_type == type) {
            if (advance().m_type == Lexing::TokenType::EndOfFile)
                return false;
            return true;
        }
        return false;
    }
};
} // Parsing

#endif // CC_PARSING_CONCRETE_TREE_HPP
