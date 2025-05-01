#pragma once

#ifndef CC_PARSING_CONCRETE_TREE_HPP
#define CC_PARSING_CONCRETE_TREE_HPP

/*
    <program>       ::= <function>
    <function>      ::= "int" <identifier> "(" "void" ")" "{" { block_item* } "}"
    <block_item>    ::= <statement> | <declaration>
    <declaration>   ::= "int" <identifier> [ "=" <exp> ] ";"
    <statement>     ::= "return" <exp> ";" | <exp> ";" | ";"
    <exp>           ::= <factor> | <exp> <binop> <exp>
    <factor>        ::= <int> | <identifier> | <unop> <exp> | "(" <exp> ")"
    <unop>          ::= "-" | "~" | "!"
    <binop>         ::= "-" | "+" | "*" | "/" | "%" | "^" | "<<" | ">>" | "&" | "|" |
                         "&&" | "||" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "="
    <identifier>    ::= ? An identifier token ?
    <int>           ::= ? A constant token ?
*/

#include "AbstractTree.hpp"
#include "Lexing/Token.hpp"

#include <vector>

namespace Parsing {

class Parse {
    using TokenType = Lexing::Token::Type;
    std::vector<Lexing::Token> c_tokens;
    size_t m_current = 0;
public:
    Parse() = delete;
    explicit Parse(const std::vector<Lexing::Token> &c_tokens)
        : c_tokens(c_tokens) {}
    [[nodiscard]] bool programParse(Program& program);
    [[nodiscard]] std::unique_ptr<BlockItem> blockItemParse();
    [[nodiscard]] std::unique_ptr<Declaration> declarationParse();
    [[nodiscard]] std::unique_ptr<Function> functionParse();
    [[nodiscard]] std::unique_ptr<Stmt> stmtParse();
    [[nodiscard]] std::unique_ptr<Expr> exprParse(i32 minPrecedence);
    [[nodiscard]] std::unique_ptr<Expr> factorParse();
    [[nodiscard]] std::unique_ptr<Expr> unaryExprParse();
    [[nodiscard]] static UnaryExpr::Operator unaryOperator(TokenType type);
    [[nodiscard]] static BinaryExpr::Operator binaryOperator(TokenType type);
private:
    bool match(const TokenType& type);
    Lexing::Token advance() { return c_tokens[m_current++]; }
    [[nodiscard]] Lexing::Token peek() const { return c_tokens[m_current]; }
    [[nodiscard]] bool expect(const Lexing::Token::Type type)
    {
        if (peek().m_type == type) {
            if (advance().m_type == TokenType::EndOfFile)
                return false;
            return true;
        }
        return false;
    }
    [[nodiscard]] static i32 getPrecedenceLevel(BinaryExpr::Operator oper);
    [[nodiscard]] static i32 precedence(BinaryExpr::Operator oper);
    [[nodiscard]] static i32 getPrecedenceLevel(UnaryExpr::Operator oper);
    [[nodiscard]] static i32 precedence(UnaryExpr::Operator oper);
    [[nodiscard]] static bool isBinaryOperator(TokenType type);
    [[nodiscard]] static bool isUnaryOperator(TokenType type);
};
} // namespace Parsing

#endif // CC_PARSING_CONCRETE_TREE_HPP
