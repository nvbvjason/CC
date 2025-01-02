#include "Parser.hpp"
#include "../ShortTypes.hpp"

namespace Parsing {

using LexTokenType = Lexing::TokenType;

i32 Parser::parseProgram(::Parsing::ProgramNode& program)
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
    if (const i32 err = expect(Lexing::TokenType::INT_KEYWORD) != 0)
        return err;
    function.name = peek().lexeme;
    if (const i32 err = expect(LexTokenType::IDENTIFIER) != 0)
        return err;
    if (const i32 err = expect(LexTokenType::OPEN_PAREN) != 0)
        return err;
    if (const i32 err = expect(LexTokenType::VOID) != 0)
        return err;
    if (const i32 err = expect(LexTokenType::CLOSE_PAREN) != 0)
        return err;
    if (const i32 err = expect(LexTokenType::OPEN_BRACE) != 0)
        return err;
    StatementNode statement;
    if (const i32 err = parseStatement(statement))
        return err;
    function.body = statement;
    if (const i32 err = expect(LexTokenType::CLOSE_BRACE) != 0)
        return err;
    return 0;
}

i32 Parser::parseStatement(StatementNode& statement)
{
    if (const i32 err = expect(LexTokenType::RETURN) != 0)
        return err;
    ExpressionNode expression;
    if (const i32 err = parseExpression(expression); err != 0)
        return err;
    statement.expression = expression;
    if (const i32 err = expect(LexTokenType::SEMICOLON) != 0)
        return err;
    return 0;
}

i32 Parser::parseExpression(ExpressionNode& expression)
{
    if (c_tokens[m_current].type != LexTokenType::INTEGER)
        return 1;
    expression.constant = std::stoi(peek().lexeme);
    ++m_current;
    return 0;
}

i32 Parser::expect(const LexTokenType type)
{
    if (c_tokens[m_current].type != type)
        return 1;
    ++m_current;
    return 0;
}


void Parser::advance()
{
    if (c_tokens.size() <= m_current)
        return;
    ++m_current;
}
}