#include "ConcreteTree.hpp"
#include "IR/AbstractTree.hpp"
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
    if (peek().m_type != TokenType::EndOfFile)
        return false;
    return true;
}

bool Parse::functionParse(Function& function)
{
    if (!expect(TokenType::IntKeyword))
        return false;
    const Lexing::Token lexeme = advance();
    if (lexeme.m_type != TokenType::Identifier)
        return false;
    function.name = lexeme.m_lexeme;
    if (!expect(TokenType::OpenParen))
        return false;
    if (!expect(TokenType::Void))
        return false;
    if (!expect(TokenType::CloseParen))
        return false;
    if (!expect(TokenType::OpenBrace))
        return false;
    Statement statement;
    if (!stmtParse(statement))
        return false;
    function.body = std::make_shared<Statement>(statement);
    if (!expect(TokenType::CloseBrace))
        return false;
    return true;
}

bool Parse::stmtParse(Statement& statement)
{
    if (!expect(TokenType::Return))
        return false;
    const std::shared_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return false;
    statement.expression = expr;
    if (!expect(TokenType::Semicolon))
        return false;
    return true;
}

std::shared_ptr<Expr> Parse::exprParse(const i32 minPrecedence)
{
    auto left = factorParse();
    if (left == nullptr)
        return nullptr;
    Lexing::Token nextToken = peek();
    i32 nextPrecedence = precedence(nextToken.m_type);
    while (isBinaryOperator(nextToken.m_type) && minPrecedence <= nextPrecedence) {
        BinaryExpr::Operator op;
        if (!binaryOperatorParse(op))
            return nullptr;
        auto right = exprParse(nextPrecedence + 1);
        if (right == nullptr)
            return nullptr;
        left = std::make_shared<BinaryExpr>(op, left, right);
        nextToken = peek();
        nextPrecedence = precedence(nextToken.m_type);
    }
    return left;
}

std::shared_ptr<Expr> Parse::factorParse()
{
    switch (const Lexing::Token lexeme = peek(); lexeme.m_type) {
        case TokenType::Integer: {
            auto constantExpr = std::make_shared<ConstantExpr>(std::stoi(lexeme.m_lexeme));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::Minus:
        case TokenType::Tilde:
        case TokenType::ExclamationMark:
            return unaryExprParse();
        case TokenType::OpenParen: {
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            auto expr = exprParse(0);
            if (!expect(TokenType::CloseParen))
                return nullptr;
            return expr;
        }
        default:
            return nullptr;
    }
}

std::shared_ptr<Expr> Parse::unaryExprParse()
{
    UnaryExpr::Operator unaryOperator;
    if (!unaryOperatorParse(unaryOperator))
        return nullptr;
    std::shared_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<UnaryExpr>(unaryOperator, std::move(expr));
}

bool Parse::unaryOperatorParse(UnaryExpr::Operator& unaryOperator)
{
    switch (const auto type = advance(); type.m_type) {
        case TokenType::Minus:
            unaryOperator = UnaryExpr::Operator::Negate;
            return true;
        case TokenType::Tilde:
            unaryOperator = UnaryExpr::Operator::Complement;
            return true;
        case TokenType::ExclamationMark:
            unaryOperator = UnaryExpr::Operator::Not;
            return true;
        default:
            return false;
    }
}

bool Parse::binaryOperatorParse(BinaryExpr::Operator& binaryOperator)
{
    switch (const auto type = advance(); type.m_type) {
         case TokenType::Plus:
            binaryOperator = BinaryExpr::Operator::Add;
            return true;
        case TokenType::Minus:
            binaryOperator = BinaryExpr::Operator::Subtract;
            return true;
        case TokenType::Asterisk:
            binaryOperator = BinaryExpr::Operator::Multiply;
            return true;
        case TokenType::ForwardSlash:
            binaryOperator = BinaryExpr::Operator::Divide;
            return true;
        case TokenType::Percent:
            binaryOperator = BinaryExpr::Operator::Remainder;
            return true;

        case TokenType::Ampersand:
            binaryOperator = BinaryExpr::Operator::BitwiseAnd;
            return true;
        case TokenType::Pipe:
            binaryOperator = BinaryExpr::Operator::BitwiseOr;
            return true;
        case TokenType::Circumflex:
            binaryOperator = BinaryExpr::Operator::BitwiseXor;
            return true;
        case TokenType::LeftShift:
            binaryOperator = BinaryExpr::Operator::LeftShift;
            return true;
        case TokenType::RightShift:
            binaryOperator = BinaryExpr::Operator::RightShift;
            return true;

        case TokenType::LogicalAnd:
            binaryOperator = BinaryExpr::Operator::And;
            return true;
        case TokenType::LogicalOr:
            binaryOperator = BinaryExpr::Operator::Or;
            return true;
        case TokenType::LogicalEqual:
            binaryOperator = BinaryExpr::Operator::Equal;
            return true;
        case TokenType::LogicalNotEqual:
            binaryOperator = BinaryExpr::Operator::NotEqual;
            return true;
        case TokenType::Greater:
            binaryOperator = BinaryExpr::Operator::GreaterThan;
            return true;
        case TokenType::Less:
            binaryOperator = BinaryExpr::Operator::LessThan;
            return true;
        case TokenType::LessOrEqual:
            binaryOperator = BinaryExpr::Operator::LessOrEqual;
            return true;
        case TokenType::GreaterOrEqual:
            binaryOperator = BinaryExpr::Operator::GreaterOrEqual;
            return true;
        default:
            return false;
    }
}

bool Parse::match(const TokenType &type)
{
    if (type == peek().m_type) {
        if (advance().m_type == TokenType::EndOfFile)
            return false;
        return true;
    }
    return false;
}

// https://en.cppreference.com/w/c/language/operator_precedence
i32 Parse::getPrecedenceLevel(const TokenType type)
{
    switch (type) {
        case TokenType::Decrement:
            return 1;
        case TokenType::ExclamationMark:
            return 2;
        case TokenType::Asterisk:
        case TokenType::ForwardSlash:
        case TokenType::Percent:
            return 3;
        case TokenType::Plus:
        case TokenType::Minus:
            return 4;
        case TokenType::LeftShift:
        case TokenType::RightShift:
            return 5;
        case TokenType::Less:
        case TokenType::LessOrEqual:
        case TokenType::Greater:
        case TokenType::GreaterOrEqual:
            return 6;
        case TokenType::LogicalEqual:
        case TokenType::LogicalNotEqual:
            return 7;
        case TokenType::Ampersand:
            return 8;
        case TokenType::Circumflex:
            return 9;
        case TokenType::Pipe:
            return 10;
        case TokenType::LogicalAnd:
            return 11;
        case TokenType::LogicalOr:
            return 12;
        default:
            return 0;
    }
}

i32 Parse::precedence(const TokenType type)
{
    constexpr i32 precedenceMult = 1024;
    constexpr i32 precedenceLevels = 16;
    return (precedenceLevels - getPrecedenceLevel(type)) * precedenceMult;
}

bool Parse::isBinaryOperator(const TokenType type)
{
    switch (type) {
        case TokenType::Plus:
        case TokenType::Minus:
        case TokenType::ForwardSlash:
        case TokenType::Percent:
        case TokenType::Asterisk:
        case TokenType::LeftShift:
        case TokenType::RightShift:
        case TokenType::Ampersand:
        case TokenType::Pipe:
        case TokenType::Circumflex:

        case TokenType::LogicalAnd:
        case TokenType::LogicalOr:
        case TokenType::LogicalEqual:
        case TokenType::LogicalNotEqual:
        case TokenType::Greater:
        case TokenType::Less:
        case TokenType::LessOrEqual:
        case TokenType::GreaterOrEqual:
            return true;
        default:
            return false;
    }
}
} // Parsing