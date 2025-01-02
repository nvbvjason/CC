#include "Parser.hpp"
#include "../ShortTypes.hpp"

namespace Parsing {

using LexTokenType = Lexing::TokenType;

i32 Parser::parseProgram(ProgramNode& program)
{
    FunctionNode function;
    if(const i32 err = parseFunction(function))
        return err;
    program.function = function;
    if (program.function.name != "main")
        return 5;
    if (m_current < c_tokens.size())
        return 6;
    return 0;
}

i32 Parser::parseFunction(FunctionNode& function)
{
    if (const i32 err = expect(Lexing::TokenType::IntKeyword) != 0)
        return err;
    function.name = peek().lexeme;
    if (const i32 err = expect(LexTokenType::Identifier) != 0)
        return err;
    if (const i32 err = expect(LexTokenType::OpenParen) != 0)
        return err;
    if (const i32 err = expect(LexTokenType::Void) != 0)
        return err;
    if (const i32 err = expect(LexTokenType::CloseParen) != 0)
        return err;
    if (const i32 err = expect(LexTokenType::OpenBrace) != 0)
        return err;
    StatementNode statement;
    if (const i32 err = parseStatement(statement))
        return err;
    function.body = statement;
    if (const i32 err = expect(LexTokenType::CloseBrace) != 0)
        return err;
    return 0;
}

i32 Parser::parseStatement(StatementNode& statement)
{
    if (const i32 err = expect(LexTokenType::Return) != 0)
        return err;
    ExpressionNode expression;
    if (const i32 err = parseExpression(expression); err != 0)
        return err;
    statement.constant = expression;
    if (const i32 err = expect(LexTokenType::Semicolon) != 0)
        return err;
    return 0;
}

i32 Parser::parseExpression(ExpressionNode& expression)
{
    if (c_tokens[m_current].m_type != LexTokenType::Integer)
        return 1;
    expression.value.constant = std::stoi(peek().lexeme);
    ++m_current;
    return 0;
}

i32 Parser::expect(const LexTokenType type)
{
    if (c_tokens[m_current].m_type != type)
        return 1;
    ++m_current;
    return 0;
}
}