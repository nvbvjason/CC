#pragma once

/*
    <program>               ::= { <declaration> }
    <declaration>           ::= <function_declaration> | <variable_declaration>
    <variable_declaration>  ::= { <specifier> }+ <declarator> [ "=" <initializer> ] ";"
    <function_declaration>  ::= { <specifier> }+ <declarator> "(" <param-list> ")" ( <block> | ";" )
    <declarator>            ::= "*" <declarator> | <direct-declarator>
    <direct-declarator>     ::= <simple-declarator> [ <declarator-suffix> ]
    <declarator-suffix>     ::= <param-list> | { "[" <const> "]" }+
    <param-list>            ::= "(" "void ")" | "(" <param> [ "," <param> ] ")"
    <param>                 ::= { <type-specifier> }+ <declarator>
    <simple-declarator>     ::= <identifier> | "(" <declarator> ")"
    <type-specifier>        ::= "int" | "long" | "unsigned" | "signed" | "double"
    <specifier>             ::= <type-specifier> | "static" | "extern"
    <block>                 ::= "{" { <block-item> } "}"
    <block_item>            ::= <statement> | <declaration>
    <initializer>           ::= <exp> | "{" <initializer> { "," <initializer> } [ "," ] ")"
    <for-init>              ::= <variable-declaration> | <exp> ";"
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
                              | <exp> <binary-op> <exp>
                              | <exp> "?" <exp> ":" <exp>
    <cast-exp>              ::= "(" { <type-specifier> }+ [ <abstract-declarator> ] ")" <cast-exp>
                              | <unary-exp>
    <unary-exp>             ::= <postfix-exp> | <unary-op> <cast-exp>
    <postfix-exp>           ::= <factor> | <postfix_exp> <postfix-op>
    <factor>                ::= <const>
                              | <identifier>
                              | <identifier> "(" [ <argument-list> ] ")"
                              | "(" <exp> ")"
    <argument-list>         ::= <exp> { "," <exp> }
    <abstract-declarator>   ::= "*" [ <abstract-declarator> ]
                              | <direct-abstract-declarator>
    <direct-abstract-declarator> ::= "(" <abstract-declarator> ")"
    <unary-op>               ::= "+" | "-" | "~" | "!" | "--" | "++" | "*" | "&"
    <postfix-op>             ::= "--" | "++"
    <binary-op>              ::= "-" | "+" | "*" | "/" | "%" | "^" | "<<" | ">>" | "&" | "|"
                              | "&&" | "||" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "="
                              | "+=" | "-=" | "*=" | "/=" | "%="
                              | "&=" | "|=" | "^=" | "<<=" | ">>="
    <const>                 ::= <int> | <long> | <uint> | <ulong> | <double>
    <identifier>            ::= ? An identifier token ?
    <int>                   ::= ? A int token ?
    <long>                  ::= ? A int or long token ?
    <uint>                  ::= ? An unsigned int token ?
    <ulong>                 ::= ? An unsigned int or unsigned long token ?
    <double>                ::= ? A floating-point constant token ?
*/

#include "ASTParser.hpp"
#include "Operators.hpp"
#include "Declarator.hpp"
#include "TokenStore.hpp"
#include "Error.hpp"

#include <vector>

namespace Parsing {

class Parser {
    using TokenType = Lexing::Token::Type;
    using Storage = Declaration::StorageClass;

    bool m_atFileScope = true;
    const TokenStore& c_tokenStore;
    size_t m_current = 0;
    std::vector<Error> m_errors;
public:
    Parser() = delete;
    explicit Parser(const TokenStore& tokenStore)
        : c_tokenStore(tokenStore) {}
    std::vector<Error> programParse(Program& program);
    [[nodiscard]] std::unique_ptr<Declaration> declarationParse();
    [[nodiscard]] std::unique_ptr<VarDecl> varDeclParse(const std::string& iden,
                                                        std::unique_ptr<TypeBase>&& type,
                                                        Storage storage);
    [[nodiscard]] std::unique_ptr<FunDecl> funDeclParse(
            const std::string& iden,
            std::unique_ptr<TypeBase>&& type,
            Storage storage,
            std::vector<std::string>&& params
        );

    [[nodiscard]] std::unique_ptr<Declarator> declaratorParse();
    [[nodiscard]] std::unique_ptr<Declarator> directDeclaratorParse();
    [[nodiscard]] std::unique_ptr<Declarator> simpleDeclaratorParse();
    [[nodiscard]] std::unique_ptr<Declarator> arrayDeclaratorParse(std::unique_ptr<Declarator>&& declarator);
    [[nodiscard]] std::unique_ptr<std::vector<ParamInfo>> paramsListParse();
    [[nodiscard]] std::unique_ptr<ParamInfo> paramParse();

    [[nodiscard]] static std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>>
        declaratorProcess(std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase);
    [[nodiscard]] static std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>>
        processFunctionDeclarator(std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase);

    [[nodiscard]] std::unique_ptr<Block> blockParse();
    [[nodiscard]] std::unique_ptr<BlockItem> blockItemParse();
    [[nodiscard]] std::unique_ptr<Initializer> initializerParse();
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
    [[nodiscard]] std::unique_ptr<Expr> ternaryExprParse(std::unique_ptr<Expr>& condition);
    [[nodiscard]] std::unique_ptr<Expr> assignmentExprParse(std::unique_ptr<Expr>& left, const Lexing::Token& nextToken);
    [[nodiscard]] std::unique_ptr<Expr> binaryExprParse(std::unique_ptr<Expr>& left, const Lexing::Token& nextToken);
    [[nodiscard]] std::unique_ptr<Expr> unaryExprParse();
    [[nodiscard]] std::unique_ptr<Expr> addrOFExprParse();
    [[nodiscard]] std::unique_ptr<Expr> dereferenceExprParse();
    [[nodiscard]] std::unique_ptr<Expr> subscriptExprParse(std::unique_ptr<Expr>&& expr);
    [[nodiscard]] std::unique_ptr<Expr> castExprParse();
    [[nodiscard]] std::unique_ptr<Expr> exprPostfixParse();
    [[nodiscard]] std::unique_ptr<Expr> factorParse();
    [[nodiscard]] std::unique_ptr<Expr> constExprParse();

    [[nodiscard]] std::unique_ptr<AbstractDeclarator> abstractDeclaratorParse();
    [[nodiscard]] std::unique_ptr<AbstractDeclarator> directAbstractDeclaratorParse();

    [[nodiscard]] static std::unique_ptr<TypeBase> abstractDeclaratorProcess(
        std::unique_ptr<AbstractDeclarator>&& abstractDeclarator, std::unique_ptr<TypeBase>&& type);

    [[nodiscard]] std::unique_ptr<std::vector<std::unique_ptr<Expr>>> argumentListParse();
    [[nodiscard]] Type typeParse();
    [[nodiscard]] std::tuple<Type, TokenType> specifierParse();
    [[nodiscard]] static Type typeResolve(std::vector<TokenType>& tokens);
private:
    bool match(const TokenType& type);
    Lexing::Token advance() { return c_tokenStore.getToken(m_current++); }
    [[nodiscard]] bool isAtEnd() const { return peekTokenType() == TokenType::EndOfFile; }
    [[nodiscard]] static bool continuePrecedenceClimbing(i32 minPrecedence, TokenType nextToken);
    [[nodiscard]] Lexing::Token peek() const { return c_tokenStore.getToken(m_current); }
    [[nodiscard]] TokenType peekTokenType() const;
    [[nodiscard]] TokenType peekNextTokenType() const;
    [[nodiscard]] TokenType peekNextNextTokenType() const;
    [[nodiscard]] bool expect(TokenType type);
    void addError(std::string message);
    void addError(std::string message, size_t index);
};

bool containsSameTwice(std::vector<Lexing::Token::Type>& tokens);
inline bool Parser::continuePrecedenceClimbing(const i32 minPrecedence, const TokenType nextToken)
{
    return (Operators::isBinaryOperator(nextToken) ||
            Operators::isAssignmentOperator(nextToken) ||
            nextToken == TokenType::QuestionMark)
        && minPrecedence <= Operators::precedence(nextToken);
}
} // namespace Parsing