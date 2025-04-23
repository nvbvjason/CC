#include "ConcreteTree.hpp"
#include "Lexing/Token.hpp"

namespace Parsing {

bool Parse::programParse(Program& program)
{
    Function function;
    if (!functionParse(function))
        return false;
    program.function = std::make_unique<Function>(std::move(function));
    if (program.function->name != "main")
        return false;
    if (c_tokens[m_current].m_type != Lexing::TokenType::EndOfFile)
        return false;
    return true;
}

bool Parse::functionParse(Function& function)
{
    if (!expect(Lexing::TokenType::IntKeyword))
        return false;
    const Lexing::Token lexeme = advance();
    if (lexeme.m_type != Lexing::TokenType::Identifier)
        return false;
    function.name = lexeme.m_lexeme;
    if (!expect(Lexing::TokenType::OpenParen))
        return false;
    if (!expect(Lexing::TokenType::Void))
        return false;
    if (!expect(Lexing::TokenType::CloseParen))
        return false;
    if (!expect(Lexing::TokenType::OpenBrace))
        return false;
    Statement statement;
    if (!statementParse(statement))
        return false;
    function.body = std::make_unique<Statement>(std::move(statement));
    if (!expect(Lexing::TokenType::CloseBrace))
        return false;
    return true;
}

bool Parse::statementParse(Statement& statement)
{
    if (!expect(Lexing::TokenType::Return))
        return false;
    Expression expression;
    if (!expressionParse(expression))
        return false;
    statement.expression = std::make_unique<Expression>(std::move(expression));
    if (!expect(Lexing::TokenType::Semicolon))
        return false;
    return true;
}

bool Parse::expressionParse(Expression& expression)
{
    switch (const Lexing::Token lexeme = peek(); lexeme.m_type) {
        case Lexing::TokenType::Integer:
            expression.type = ExpressionType::Constant;
            expression.value = std::stoi(lexeme.m_lexeme);
            if (advance().m_type == Lexing::TokenType::EndOfFile)
                return false;
            break;
        case Lexing::TokenType::Minus:
        case Lexing::TokenType::Tilde: {
            expression.type = ExpressionType::Unary;
            Unary unaryNode;
            if (!unaryParse(unaryNode))
                return false;
            expression.value = std::make_unique<Unary>(std::move(unaryNode));
            break;
        }
        case Lexing::TokenType::OpenParen: {
            if (advance().m_type == Lexing::TokenType::EndOfFile)
                return false;
            Expression innerExpression;
            if (!expressionParse(innerExpression))
                return false;
            expression = std::move(innerExpression);
            if (!expect(Lexing::TokenType::CloseParen))
                return false;
            break;
        }
        default:
            return false;
    }
    return true;
}

bool Parse::unaryParse(Unary& unary)
{
    UnaryOperator unaryOperator;
    if (!unaryOperatorParse(unaryOperator))
        return false;
    if (unaryOperator == UnaryOperator::Invalid)
        return false;
    unary.unaryOperator = unaryOperator;
    Expression expression;
    if (!expressionParse(expression))
        return false;
    unary.expression = std::make_unique<Expression>(std::move(expression));
    return true;
}

bool Parse::unaryOperatorParse(UnaryOperator& unaryOperator)
{
    switch (const auto type = advance(); type.m_type) {
        case Lexing::TokenType::Minus:
            unaryOperator = UnaryOperator::Negate;
            break;
        case Lexing::TokenType::Tilde:
            unaryOperator = UnaryOperator::Complement;
            break;
        default:
            return false;
    }
    return true;
}

bool Parse::match(const Lexing::TokenType &type)
{
    if (type == c_tokens[m_current].m_type) {
        if (advance().m_type == Lexing::TokenType::EndOfFile)
            return false;
        return true;
    }
    return false;
}
} // Parsing