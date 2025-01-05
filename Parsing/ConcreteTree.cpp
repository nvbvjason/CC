#include "ConcreteTree.hpp"
#include "../Lexing/Token.hpp"

namespace Parsing {

i32 Parse::programParse(ProgramNode& program)
{
    const auto functionNode = new FunctionNode();
    if (const i32 err = functionParse(functionNode) != 0)
        return err;
    program.function = static_cast<std::unique_ptr<FunctionNode>>(functionNode);
    if (program.function->name != "main")
        return -1;
    if (constexpr int endOfFileToken = 1; m_current < c_tokens.size() - endOfFileToken)
        return -2;
    return 0;
}

i32 Parse::functionParse(FunctionNode* function)
{
    if (!expect(Lexing::TokenType::IntKeyword))
        return m_current + 1;
    const Lexing::Token lexeme = advance();
    if (lexeme.m_type != Lexing::TokenType::Identifier)
        return m_current + 1;
    function->name = lexeme.lexeme;
    if (!expect(Lexing::TokenType::OpenParen))
        return m_current + 1;
    if (!expect(Lexing::TokenType::Void))
        return m_current + 1;
    if (!expect(Lexing::TokenType::CloseParen))
        return m_current + 1;
    if (!expect(Lexing::TokenType::OpenBrace))
        return m_current + 1;
    const auto statement = new StatementNode();
    if (const i32 err = statementParse(statement) != 0)
        return err;
    function->body = static_cast<std::unique_ptr<StatementNode>>(statement);
    if (!expect(Lexing::TokenType::CloseBrace))
        return m_current + 1;
    return 0;
}

i32 Parse::statementParse(StatementNode* statement)
{
    if (!expect(Lexing::TokenType::Return))
        return m_current + 1;
    const auto expression = new ExpressionNode();
    if (expressionParse(expression))
        return m_current + 1;
    statement->expression = static_cast<std::unique_ptr<ExpressionNode>>(expression);
    if (!expect(Lexing::TokenType::Semicolon))
        return m_current + 1;
    return 0;
}

i32 Parse::expressionParse(ExpressionNode* expression)
{
    switch (Lexing::Token lexeme = advance(); lexeme.m_type) {
        case Lexing::TokenType::Integer:
            expression->type = ExpressionNodeType::Constant;
            expression->value = std::get<i32>(lexeme.m_data);
            break;
        case Lexing::TokenType::Minus:
        case Lexing::TokenType::Tilde: {
            --m_current;
            expression->type = ExpressionNodeType::Unary;
            const auto unaryNode = new UnaryNode();
            if (const i32 err = unaryParse(unaryNode))
                return err;
            expression->value = static_cast<std::unique_ptr<UnaryNode>>(unaryNode);
            break;
        }
        case Lexing::TokenType::OpenParen:
            if (const i32 err = expressionParse(expression) != 0)
                return err;
            if (!expect(Lexing::TokenType::CloseParen))
                return m_current + 1;
            break;
        default:
            return -1;
    }
    return 0;
}

i32 Parse::unaryParse(UnaryNode* unary)
{
    UnaryOperator unaryOperator;
    if (const i32 err = unaryOperatorParse(unaryOperator) != 0)
        return err;
    unary->unaryOperator = unaryOperator;
    const auto expression = new ExpressionNode();
    if (const i32 err = expressionParse(expression))
        return err;
    unary->expression = static_cast<std::unique_ptr<ExpressionNode>>(expression);
    return 0;
}

i32 Parse::unaryOperatorParse(UnaryOperator& unaryOperator)
{
    switch (const auto type = advance(); type.m_type) {
        case Lexing::TokenType::Minus:
            unaryOperator = UnaryOperator::Negate;
            break;
        case Lexing::TokenType::Tilde:
            unaryOperator = UnaryOperator::Complement;
            break;
        default:
            return m_current + 1;
    }
    return 0;
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