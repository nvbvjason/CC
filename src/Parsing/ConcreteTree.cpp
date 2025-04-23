#include "ConcreteTree.hpp"
#include "../Lexing/Token.hpp"

namespace Parsing {

i32 Parse::programParse(Program& program)
{
    auto[functionNode, err] = functionParse();
    program.function = std::unique_ptr<Function>(functionNode);
    if (err != 0)
        return err;
    if (program.function->name != "main")
        return ErrorCodes::MAIN_FUNCTION_EXPECTED;
    if (c_tokens[m_current].m_type != Lexing::TokenType::EndOfFile)
        return ErrorCodes::UNEXPECTED_TOKEN;
    return 0;
}

std::pair<Function*, i32> Parse::functionParse()
{
    auto function = std::make_unique<Function>();
    if (!expect(Lexing::TokenType::IntKeyword))
        return {function.release(), m_current + 1};
    const Lexing::Token lexeme = advance();
    if (lexeme.m_type != Lexing::TokenType::Identifier)
        return {function.release(), m_current + 1};
    function->name = lexeme.m_lexeme;
    if (!expect(Lexing::TokenType::OpenParen))
        return {function.release(), m_current + 1};
    if (!expect(Lexing::TokenType::Void))
        return {function.release(), m_current + 1};
    if (!expect(Lexing::TokenType::CloseParen))
        return {function.release(), m_current + 1};
    if (!expect(Lexing::TokenType::OpenBrace))
        return {function.release(), m_current + 1};
    auto[statement, errStatement] = statementParse();
    function->body = static_cast<std::unique_ptr<Statement>>(statement);
    if (errStatement != 0)
        return {function.release(), errStatement};
    if (!expect(Lexing::TokenType::CloseBrace))
        return {function.release(), m_current + 1};
    return {function.release(), ErrorCodes::SUCCESS};
}

std::pair<Statement*, i32> Parse::statementParse()
{
    auto statement = std::make_unique<Statement>();
    if (!expect(Lexing::TokenType::Return))
        return {statement.release(), m_current + 1};
    auto [expression, errExpr] = expressionParse(0);
    statement->expression = static_cast<std::unique_ptr<Expression>>(expression);
    if (errExpr != 0)
        return {statement.release(), m_current + 1};
    if (!expect(Lexing::TokenType::Semicolon))
        return {statement.release(), m_current + 1};
    return {statement.release(), 0};
}

std::pair<Expression*, i32> Parse::expressionParse(const u16 depth)
{
    if (constexpr u16 maxDepth = 1024; maxDepth <= depth) {
#ifdef DEBUG
        std::cerr << "Recursion limit exceeded at token "
                  << m_current << "\n";
#endif
        return {nullptr, ErrorCodes::RECURSION_DEPTH_EXCEEDED};
    }
    auto expression = std::make_unique<Expression>();
    switch (const Lexing::Token lexeme = peek(); lexeme.m_type) {
        case Lexing::TokenType::Integer:
            expression->type = ExpressionType::Constant;
            expression->value = std::stoi(lexeme.m_lexeme);
            advance();
            break;
        case Lexing::TokenType::Minus:
        case Lexing::TokenType::Tilde: {
            expression->type = ExpressionType::Unary;
            auto[unaryNode, errUnary] = unaryParse();
            expression->value = static_cast<std::unique_ptr<Unary>>(unaryNode);
            if (errUnary != 0)
                return {expression.release(), errUnary};
            break;
        }
        case Lexing::TokenType::OpenParen: {
            advance();
            auto[innerExpression, errExpression] = expressionParse(depth + 1);
            if (errExpression != 0) {
                delete innerExpression;
                return {expression.release(), errExpression};
            }
            expression.reset(innerExpression);
            if (!expect(Lexing::TokenType::CloseParen))
                return {expression.release(), m_current + 1};
            break;
        }
        default:
            return {expression.release(), -1};
    }
    return {expression.release(), ErrorCodes::SUCCESS};
}

std::pair<Unary*, i32> Parse::unaryParse()
{
    auto unaryNode = std::make_unique<Unary>();
    auto[unaryOperator, errUnaryOperator] = unaryOperatorParse();
    if (errUnaryOperator != 0)
        return {unaryNode.release(), errUnaryOperator};
    unaryNode->unaryOperator = unaryOperator;
    auto[expression, errExpression] = expressionParse(0);
    unaryNode->expression = static_cast<std::unique_ptr<Expression>>(expression);
    if (errExpression != 0)
        return  {unaryNode.release(), errExpression};
    return {unaryNode.release(), ErrorCodes::SUCCESS};
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
    return {unaryOperator, ErrorCodes::SUCCESS};
}

size_t Parse::match(const Lexing::TokenType &type)
{
    if (type == c_tokens[m_current].m_type) {
        advance();
        return 0;
    }
    return m_current + 1uz;
}
} // Parsing