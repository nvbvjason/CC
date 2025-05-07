#pragma once

#ifndef CC_PARSING_CONCRETE_TREE_HPP
#define CC_PARSING_CONCRETE_TREE_HPP

/*
    <program>               ::= { <function-declaration> }
    <declaration>           ::= <function_declaration> | <variable_declaration>
    <variable_declaration>  ::= "int" <identifier> [ <exp> ] ";"
    <function_declaration>  ::= "init <identifier> "(" <param-list> ")" ( <block> | ";" )
    <param-list>            ::= "void" | "int" <identifier> { "," <identifier> }
    <block>                 ::= "{" { <block-item> } "}"
    <block_item>            ::= <statement> | <declaration>
    <for-inti>              ::= <variable-declaration> | <exp> ";"
    <statement>             ::= "return" <exp> ";"
                              | <exp> ";"
                              | "if" "(" <exp> ")" <statement> [ "else" <statement> ]
                              | "switch" ( <exp> ")" <statement>
                              | "goto" <identifier> ";"
                              | <block>
                              | "break" ";"
                              | "continue" ";"
                              | <identifier> ":" <statement>
                              | "case" <exp> ":" <statement>
                              | default ":" <statement>
                              | "while" "(" <exp> ")" <statement>
                              | "do" <statement> "while" "(" <exp> ")" ";"
                              | "for" "(" <for-inti> [ <exp> ] ";" [ <exp> ] ")" <statement>
                              | ";"
    <exp>                   ::= <unary_exp>
                              | <exp> <binop> <exp>
                              | <exp> "?" <exp> ":" <exp>
    <unary_exp>             ::= <postfix_exp> | <unop> <unary_exp>
    <postfix_exp>           ::= <factor> | <postfix_exp> <postfixop>
    <factor>                ::= <int> | <identifier> | "(" <exp> ")"
    <argument-list>         ::= <exp> { "," <exp> }
    <unop>                  ::= "-" | "~" | "!" | "--" | "++"
    <postfixop>             ::= "--" | "++"
    <binop>                 ::= "-" | "+" | "*" | "/" | "%" | "^" | "<<" | ">>" | "&" | "|"
                              | "&&" | "||" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "="
                              | "+=" | "-=" | "*=" | "/=" | "%="
                              | "&=" | "|=" | "^=" | "<<=" | ">>="
    <identifier>            ::= ? An identifier token ?
    <int>                   ::= ? A constant token ?
*/

#include "ASTParser.hpp"
#include "Frontend/Lexing/Token.hpp"
#include "Operators.hpp"

#include <vector>

namespace Parsing {

class Parser {
    using TokenType = Lexing::Token::Type;
    std::vector<Lexing::Token> c_tokens;
    std::string m_breakLabel;
    std::string m_continueLabel;
    std::string m_switchLabel;
    size_t m_current = 0;
public:
    Parser() = delete;
    explicit Parser(const std::vector<Lexing::Token> &c_tokens)
        : c_tokens(c_tokens) {}
    bool programParse(Program& program);
    [[nodiscard]] std::unique_ptr<Declaration> declarationParse();
    [[nodiscard]] std::unique_ptr<VarDecl> varDeclParse();
    [[nodiscard]] std::unique_ptr<FunDecl> funDeclParse();
    [[nodiscard]] std::unique_ptr<std::vector<std::string>> paramsListParse();

    [[nodiscard]] std::unique_ptr<Block> blockParse();
    [[nodiscard]] std::unique_ptr<BlockItem> blockItemParse();
    [[nodiscard]] std::unique_ptr<ForInit> forInitParse();

    [[nodiscard]] std::unique_ptr<Stmt> stmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> returnStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> exprStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> ifStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> gotoStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> breakStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> continueStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> labelStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> caseStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> defaultStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> whileStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> doWhileStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> forStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> switchStmtParse();
    [[nodiscard]] std::unique_ptr<Stmt> nullStmtParse();

    [[nodiscard]] std::unique_ptr<Expr> exprParse(i32 minPrecedence);
    [[nodiscard]] std::unique_ptr<Expr> exprPostfix();
    [[nodiscard]] std::unique_ptr<Expr> factorParse();
    [[nodiscard]] std::unique_ptr<Expr> unaryExprParse();

    [[nodiscard]] std::unique_ptr<std::vector<std::unique_ptr<Expr>>> argumentListParse();
private:
    bool match(const TokenType& type);
    Lexing::Token advance() { return c_tokens[m_current++]; }
    [[nodiscard]] bool isAtEnd() const { return peekTokenType() == TokenType::EndOfFile; }
    [[nodiscard]] static bool continuePrecedenceClimbing(i32 minPrecedence, TokenType nextToken);
    [[nodiscard]] Lexing::Token peek() const { return c_tokens[m_current]; }
    [[nodiscard]] TokenType peekTokenType() const;
    [[nodiscard]] TokenType peekNextTokenType() const;
    [[nodiscard]] TokenType peekNextNextTokenType() const;
    [[nodiscard]] bool expect(TokenType type);
    static std::string makeTemporary(const std::string& name);
};

inline std::string Parser::makeTemporary(const std::string& name)
{
    static i32 m_counter = 0;
    return name + '.' + std::to_string(m_counter++);
}

inline bool Parser::continuePrecedenceClimbing(const i32 minPrecedence, TokenType nextToken)
{
    return (Operators::isBinaryOperator(nextToken)
            || Operators::isAssignmentOperator(nextToken)
            || nextToken == TokenType::QuestionMark)
           && minPrecedence <= Operators::precedence(nextToken);
}
} // namespace Parsing

#endif // CC_PARSING_CONCRETE_TREE_HPP
