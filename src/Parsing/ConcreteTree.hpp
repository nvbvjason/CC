#pragma once

#ifndef CC_PARSING_CONCRETE_TREE_HPP
#define CC_PARSING_CONCRETE_TREE_HPP

/*
    <program>    ::= <function>
    <function>   ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
    <statement>  ::= "return" <exp> ";"
    <exp>        ::= <factor> | <exp> <binop> <exp>
    <factor>     ::= <int> | <unop> <exp> | "(" <exp> ")"
    <unop>       ::= "-" | "~"
    <binop>      ::= "-" | "+" | "*" | "/" | "%"
    <identifier> ::= ? An identifier token ?
    <int>        ::= ? A constant token ?
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
    [[nodiscard]] bool stmtParse(Statement& statement);
    [[nodiscard]] std::shared_ptr<Expr> exprParse(i32 minPrecedence);
    [[nodiscard]] std::shared_ptr<Expr> factorParse();
    [[nodiscard]] std::shared_ptr<Expr> unaryExprParse();
    [[nodiscard]] bool unaryOperatorParse(UnaryExpr::Operator& unaryOperator);
    [[nodiscard]] bool binaryOperatorParse(BinaryExpr::Operator& binaryOperator);
private:
    bool match(const Lexing::Token::Type& type);
    [[nodiscard]] Lexing::Token peek() const { return c_tokens[m_current]; }
    [[nodiscard]] Lexing::Token advance() { return c_tokens[m_current++]; }
    [[nodiscard]] bool expect(const Lexing::Token::Type type)
    {
        if (peek().m_type == type) {
            if (advance().m_type == Lexing::Token::Type::EndOfFile)
                return false;
            return true;
        }
        return false;
    }

    [[nodiscard]] static i32 precedence(Lexing::Token::Type type);
    [[nodiscard]] static bool isBinaryOperator(Lexing::Token::Type type);
};
} // Parsing

#endif // CC_PARSING_CONCRETE_TREE_HPP
