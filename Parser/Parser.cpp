#include "Parser.hpp"

#include <iostream>

namespace Parsing {

using Lexing::TokenType;

FunctionDefinition Parser::parseProgram()
{
    FunctionDefinition function = parseFunction();
    if (m_current < c_tokens.size())
        throw std::runtime_error("too many tokens");
    return function;
}

FunctionDefinition Parser::parseFunction()
{
    expect(TokenType::INT_KEYWORD);
    FunctionDefinition function;
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

Statement Parser::parseStatement()
{
    expect(TokenType::RETURN);
    Statement statement;
    statement.expression = parseExpression();
    expect(TokenType::SEMICOLON);
    return statement;
}

Expression Parser::parseExpression()
{
    Expression expression;
    expression.constant = std::stoi(peek().lexeme);
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
