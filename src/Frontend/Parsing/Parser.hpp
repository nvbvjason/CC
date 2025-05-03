#pragma once

#ifndef CC_PARSING_CONCRETE_TREE_HPP
#define CC_PARSING_CONCRETE_TREE_HPP

/*
    <program>       ::= <function>
    <function>      ::= "int" <identifier> "(" "void" ")" "{" { block_item* } "}"
    <block_item>    ::= <statement> | <declaration>
    <declaration>   ::= "int" <identifier> [ "=" <exp> ] ";"
    <statement>     ::= "return" <exp> ";"
                      | <exp> ";"
                      | "if" "(" <exp> ")" <statement> [ "else" <statement> ]
                      | ";"
    <exp>           ::= <unary_exp> | <exp> <binop> <exp>
    <unary_exp>     ::= <postfix_exp> | <unop> <unary_exp>
    <postfix_exp>   ::= <factor> | <postfix_exp> "--"
    <factor>        ::= <int> | <identifier> | "(" <exp> ")"
    <unop>          ::= "-" | "~" | "!" | "--" | "++"
    <postfixop>     ::= "--" | "++"
    <binop>         ::= "-" | "+" | "*" | "/" | "%" | "^" | "<<" | ">>" | "&" | "|"
                      | "&&" | "||" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "="
                      | "+=" | "-=" | "*=" | "/=" | "%="
                      | "&=" | "|=" | "^=" | "<<=" | ">>="
    <identifier>    ::= ? An identifier token ?
    <int>           ::= ? A constant token ?
*/

#include "../AST/ASTParser.hpp"
#include "Frontend/Lexing/Token.hpp"

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
    bool programParse(Program& program);
    [[nodiscard]] std::unique_ptr<BlockItem> blockItemParse();
    [[nodiscard]] std::unique_ptr<Declaration> declarationParse();
    [[nodiscard]] std::unique_ptr<Stmt> stmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> returnStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> exprStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> ifStmtParse();
    [[nodiscard]] std::unique_ptr<Function> functionParse();
    [[nodiscard]] std::unique_ptr<Expr> exprPostfix();
    [[nodiscard]] std::unique_ptr<Expr> exprParse(i32 minPrecedence);
    [[nodiscard]] std::unique_ptr<Expr> factorParse();
    [[nodiscard]] std::unique_ptr<Expr> unaryExprParse();
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
    [[nodiscard]] static UnaryExpr::Operator unaryOperator(TokenType type);
    [[nodiscard]] static BinaryExpr::Operator binaryOperator(TokenType type);
    [[nodiscard]] static AssignmentExpr::Operator assignOperator(TokenType type);
    [[nodiscard]] static i32 getPrecedenceLevel(BinaryExpr::Operator oper);
    [[nodiscard]] static i32 getPrecedenceLevel(UnaryExpr::Operator oper);
    [[nodiscard]] static i32 getPrecedenceLevel(AssignmentExpr::Operator oper);
    [[nodiscard]] static bool isBinaryOperator(TokenType type);
    [[nodiscard]] static bool isUnaryOperator(TokenType type);
    [[nodiscard]] static bool isAssignmentOperator(TokenType type);
    static i32 precedence(const TokenType type)
    {
        constexpr i32 precedenceMult = 1024;
        constexpr i32 precedenceLevels = 16;
        if (isBinaryOperator(type)) {
            BinaryExpr::Operator oper = binaryOperator(type);
            return (precedenceLevels - getPrecedenceLevel(oper)) * precedenceMult;
        }
        if (isUnaryOperator(type)) {
            UnaryExpr::Operator oper = unaryOperator(type);
            return (precedenceLevels - getPrecedenceLevel(oper)) * precedenceMult;
        }
        if (isAssignmentOperator(type)) {
            AssignmentExpr::Operator oper = assignOperator(type);
            return (precedenceLevels - getPrecedenceLevel(oper)) * precedenceMult;
        }
        return 0;
    }
};
} // namespace Parsing

#endif // CC_PARSING_CONCRETE_TREE_HPP
