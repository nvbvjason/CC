#include "ConcreteTree.hpp"
#include "../Lexing/Token.hpp"

namespace Parsing {

i32 Parse::programParse(Program& program)
{
    auto[functionNode, err] = functionParse();
    program.function = static_cast<std::unique_ptr<Function>>(functionNode);
    if (err != 0)
        return err;
    if (program.function->name != "main")
        return -1;
    if (c_tokens[m_current].m_type != Lexing::TokenType::EndOfFile)
        return -2;
    return 0;
}

std::pair<Function*, i32> Parse::functionParse()
{
    const auto function = new Function();
    if (!expect(Lexing::TokenType::IntKeyword))
        return {function, m_current + 1};
    const Lexing::Token lexeme = advance();
    if (lexeme.m_type != Lexing::TokenType::Identifier)
        return {function, m_current + 1};
    function->name = lexeme.lexeme;
    if (!expect(Lexing::TokenType::OpenParen))
        return {function, m_current + 1};
    if (!expect(Lexing::TokenType::Void))
        return {function, m_current + 1};
    if (!expect(Lexing::TokenType::CloseParen))
        return {function, m_current + 1};
    if (!expect(Lexing::TokenType::OpenBrace))
        return {function, m_current + 1};
    auto[statement, errStatement] = statementParse();
    function->body = static_cast<std::unique_ptr<Statement>>(statement);
    if (errStatement != 0)
        return {function, errStatement};
    if (!expect(Lexing::TokenType::CloseBrace))
        return {function, m_current + 1};
    return {function, 0};
}

std::pair<Statement*, i32> Parse::statementParse()
{
    auto statement = new Statement();
    if (!expect(Lexing::TokenType::Return))
        return {statement, m_current + 1};
    auto [expression, errExpr] = expressionParse();
    statement->expression = static_cast<std::unique_ptr<Expression>>(expression);
    if (errExpr != 0)
        return {statement, m_current + 1};
    if (!expect(Lexing::TokenType::Semicolon))
        return {statement, m_current + 1};
    return {statement, 0};
}

std::pair<Expression*, i32> Parse::expressionParse()
{
    auto expression = new Expression();
    switch (const Lexing::Token lexeme = advance(); lexeme.m_type) {
        case Lexing::TokenType::Integer:
            expression->type = ExpressionType::Constant;
            --m_current;
            expression->value = std::stoi(lexeme.lexeme);
            ++m_current;
            break;
        case Lexing::TokenType::Minus:
        case Lexing::TokenType::Tilde: {
            --m_current;
            expression->type = ExpressionType::Unary;
            auto[unaryNode, errUnary] = unaryParse();
            expression->value = static_cast<std::unique_ptr<Unary>>(unaryNode);
            if (errUnary != 0)
                return {expression, errUnary};
            break;
        }
        case Lexing::TokenType::OpenParen: {
            auto[innerExpression, errExpression] = expressionParse();
            if (errExpression != 0) {
                delete innerExpression;
                return {expression, errExpression};
            }
            delete expression;
            expression = innerExpression;
            if (!expect(Lexing::TokenType::CloseParen))
                return {expression, m_current + 1};
            break;
        }
        default:
            return {expression, -1};
    }
    return {expression, 0};
}

std::pair<Unary*, i32> Parse::unaryParse()
{
    auto unaryNode = new Unary();
    auto[unaryOperator, errUnaryOperator] = unaryOperatorParse();
    if (errUnaryOperator != 0)
        return {unaryNode, errUnaryOperator};
    unaryNode->unaryOperator = unaryOperator;
    auto[expression, errExpression] = expressionParse();
    unaryNode->expression = static_cast<std::unique_ptr<Expression>>(expression);
    if (errExpression != 0)
        return  {unaryNode, errExpression};
    return {unaryNode, 0};
}

std::pair<UnaryOperator, i32> Parse::unaryOperatorParse()
{
    UnaryOperator unaryOperator;
    switch (const auto type = advance(); type.m_type) {
        case Lexing::TokenType::Minus:
            unaryOperator = UnaryOperator::Negate;
            break;
        case Lexing::TokenType::Tilde:
            unaryOperator = UnaryOperator::Complement;
            break;
        default:
            return {unaryOperator, m_current + 1};
    }
    return {unaryOperator, 0};
}

i32 Parse::match(const Lexing::TokenType &type)
{
    if (type == c_tokens[m_current].m_type) {
        ++m_current;
        return 0;
    }
    return m_current + 1;
}
} // Parsing