#pragma once

#ifndef CC_PARSING_CONCRETE_TREE_HPP
#define CC_PARSING_CONCRETE_TREE_HPP

/*
    <program>               ::= { <declaration> }
    <declaration>           ::= <function_declaration> | <variable_declaration>
    <variable_declaration>  ::= { <specifier> }+ <identifier> [ "=" <exp> ] ";"
    <function_declaration>  ::= { <specifier> }+ <identifier> "(" <param-list> ")" ( <block> | ";" )
    <param-list>            ::= "void" | { <type-specifier> }+ <identifier> { "," { <type-specifier> }+ <identifier> }
    <type-specifier>        ::= "int" | "long"
    <specifier>             ::= <type-specifier> | "static" | "extern"
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
    <cast-exp>              ::= "(" { <type-specifier> }+ ")" <cast-exp>
                              | <unary-exp>
    <unary-exp>             ::= <postfix-exp> | <unop> <unary-exp>
    <postfix-exp>           ::= <factor> | <postfix_exp> <postfixop>
    <factor>                ::= <const>
                              | <identifier>
                              | <identifier> "(" [ <argument-list> ] ")"
                              | "(" <exp> ")"
    <argument-list>         ::= <exp> { "," <exp> }
    <unop>                  ::= "+" | "-" | "~" | "!" | "--" | "++"
    <postfixop>             ::= "--" | "++"
    <binop>                 ::= "-" | "+" | "*" | "/" | "%" | "^" | "<<" | ">>" | "&" | "|"
                              | "&&" | "||" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "="
                              | "+=" | "-=" | "*=" | "/=" | "%="
                              | "&=" | "|=" | "^=" | "<<=" | ">>="
    <const>                 ::= <int> | <long>
    <identifier>            ::= ? An identifier token ?
    <int>                   ::= ? A int token ?
    <long>                  ::= ? A int or long token ?
*/

#include "ASTParser.hpp"
#include "Operators.hpp"

#include <vector>

namespace Parsing {

class Parser {
    using TokenType = Lexing::Token::Type;
    using Storage = Declaration::StorageClass;
    struct ParamList {
        std::vector<std::string> params;
        std::vector<std::unique_ptr<TypeBase>> types;
        ParamList(std::vector<std::string>&& params, std::vector<std::unique_ptr<TypeBase>>&& types)
            :params(std::move(params)), types(std::move(types)) {}
    };
    bool m_atFileScope = true;
    std::vector<Lexing::Token> c_tokens;
    size_t m_current = 0;
public:
    Parser() = delete;
    explicit Parser(const std::vector<Lexing::Token> &c_tokens)
        : c_tokens(c_tokens) {}
    bool programParse(Program& program);
    [[nodiscard]] std::unique_ptr<Declaration> declarationParse();
    [[nodiscard]] std::unique_ptr<VarDecl> varDeclParse(TokenType type,
                                                        Storage storage,
                                                        const std::string& iden);
    [[nodiscard]] std::unique_ptr<FunDecl> funDeclParse(TokenType type,
                                                        Storage storage,
                                                        const std::string& iden);
    [[nodiscard]] std::unique_ptr<ParamList> paramsListParse();

    [[nodiscard]] std::unique_ptr<Block> blockParse();
    [[nodiscard]] std::unique_ptr<BlockItem> blockItemParse();
    [[nodiscard]] std::tuple<std::unique_ptr<ForInit>, bool> forInitParse();

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
    [[nodiscard]] std::unique_ptr<Expr> castExpr();
    [[nodiscard]] std::unique_ptr<Expr> exprPostfix();
    [[nodiscard]] std::unique_ptr<Expr> factorParse();
    [[nodiscard]] std::unique_ptr<Expr> unaryExprParse();

    [[nodiscard]] std::unique_ptr<std::vector<std::unique_ptr<Expr>>> argumentListParse();
    [[nodiscard]] TokenType typeParse();
    [[nodiscard]] std::tuple<TokenType, TokenType> specifierParse();
    [[nodiscard]] static TokenType typeResolve(const std::vector<TokenType>& tokens);
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
};

Declaration::StorageClass getStorageClass(Lexing::Token::Type tokenType);
inline bool Parser::continuePrecedenceClimbing(const i32 minPrecedence, const TokenType nextToken)
{
    return (Operators::isBinaryOperator(nextToken) ||
            Operators::isAssignmentOperator(nextToken) ||
            nextToken == TokenType::QuestionMark)
        && minPrecedence <= Operators::precedence(nextToken);
}
} // namespace Parsing

#endif // CC_PARSING_CONCRETE_TREE_HPP
