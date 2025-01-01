#include "Parser.hpp"

#include <iostream>

namespace Parsing {

using Lexing::TokenType;

ProgramNode Parser::parseProgram()
{
    ProgramNode program{.function = parseFunction()};
    if (m_current < c_tokens.size())
        throw std::runtime_error("too many tokens");
    return program;
}

FunctionNode Parser::parseFunction()
{
    expect(TokenType::INT_KEYWORD);
    FunctionNode function;
    function.name = peek().lexeme;
    expect(TokenType::IDENTIFIER);
    expect(TokenType::OPEN_PAREN);
    expect(TokenType::VOID);
    expect(TokenType::CLOSE_PAREN);
    expect(TokenType::OPEN_BRACE);
    function.body = parseStatement();
    expect(TokenType::CLOSE_BRACE);
    return function;
}

StatementNode Parser::parseStatement()
{
    expect(TokenType::RETURN);
    const StatementNode statement{.expression = parseExpression()};
    expect(TokenType::SEMICOLON);
    return statement;
}

ExpressionNode Parser::parseExpression()
{
    const ExpressionNode expression{.constant = std::stoi(peek().lexeme)};
    expect(TokenType::INTEGER);
    return expression;
}

void Parser::expect(const TokenType expected)
{
    if (expected == c_tokens[m_current].type) {
        advance();
        return;
    }
    throw std::runtime_error("Unexpected token");
}

void Parser::advance()
{
    if (c_tokens.size() <= m_current)
        return;
    ++m_current;
}
}
