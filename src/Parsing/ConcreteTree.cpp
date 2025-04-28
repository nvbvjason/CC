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
    if (c_tokens[m_current].m_type != Lexing::Token::Type::EndOfFile)
        return false;
    return true;
}

bool Parse::functionParse(Function& function)
{
    if (!expect(Lexing::Token::Type::IntKeyword))
        return false;
    const Lexing::Token lexeme = advance();
    if (lexeme.m_type != Lexing::Token::Type::Identifier)
        return false;
    function.name = lexeme.m_lexeme;
    if (!expect(Lexing::Token::Type::OpenParen))
        return false;
    if (!expect(Lexing::Token::Type::Void))
        return false;
    if (!expect(Lexing::Token::Type::CloseParen))
        return false;
    if (!expect(Lexing::Token::Type::OpenBrace))
        return false;
    Statement statement;
    if (!stmtParse(statement))
        return false;
    function.body = std::make_shared<Statement>(statement);
    if (!expect(Lexing::Token::Type::CloseBrace))
        return false;
    return true;
}

bool Parse::stmtParse(Statement& statement)
{
    if (!expect(Lexing::Token::Type::Return))
        return false;
    const std::shared_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return false;
    statement.expression = expr;
    if (!expect(Lexing::Token::Type::Semicolon))
        return false;
    return true;
}

std::shared_ptr<Expr> Parse::exprParse(const i32 minPrecedence)
{
    auto left = factorParse();
    if (left == nullptr)
        return nullptr;
    Lexing::Token token = peek();
    const i32 nextPrecedence = getPrecedenceLevel(token.m_type);
    while (isBinaryOperator(token.m_type) && minPrecedence <= nextPrecedence) {
        BinaryExpr::Operator op;
        if (!binaryOperatorParse(op))
            return nullptr;
        auto right = exprParse(nextPrecedence + 1);
        if (right == nullptr)
            return nullptr;
        left = std::make_shared<BinaryExpr>(op, left, right);
        token = peek();
    }
    return left;
}

std::shared_ptr<Expr> Parse::factorParse()
{
    switch (const Lexing::Token lexeme = peek(); lexeme.m_type) {
        case Lexing::Token::Type::Integer: {
            auto constantExpr = std::make_shared<ConstantExpr>(std::stoi(lexeme.m_lexeme));
            if (advance().m_type == Lexing::Token::Type::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case Lexing::Token::Type::Minus:
        case Lexing::Token::Type::Tilde:
        case Lexing::Token::Type::ExclamationMark:
            return unaryExprParse();
        case Lexing::Token::Type::OpenParen: {
            if (advance().m_type == Lexing::Token::Type::EndOfFile)
                return nullptr;
            auto expr = exprParse(0);
            if (!expect(Lexing::Token::Type::CloseParen))
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
        case Lexing::Token::Type::Minus:
            unaryOperator = UnaryExpr::Operator::Negate;
            return true;
        case Lexing::Token::Type::Tilde:
            unaryOperator = UnaryExpr::Operator::Complement;
            return true;
        case Lexing::Token::Type::ExclamationMark:
            unaryOperator = UnaryExpr::Operator::Not;
            return true;
        default:
            return false;
    }

}

bool Parse::binaryOperatorParse(BinaryExpr::Operator& binaryOperator)
{
    switch (const auto type = advance(); type.m_type) {
         case Lexing::Token::Type::Plus:
            binaryOperator = BinaryExpr::Operator::Add;
            return true;
        case Lexing::Token::Type::Minus:
            binaryOperator = BinaryExpr::Operator::Subtract;
            return true;
        case Lexing::Token::Type::Asterisk:
            binaryOperator = BinaryExpr::Operator::Multiply;
            return true;
        case Lexing::Token::Type::ForwardSlash:
            binaryOperator = BinaryExpr::Operator::Divide;
            return true;
        case Lexing::Token::Type::Percent:
            binaryOperator = BinaryExpr::Operator::Remainder;
            return true;

        case Lexing::Token::Type::Ampersand:
            binaryOperator = BinaryExpr::Operator::BitwiseAnd;
            return true;
        case Lexing::Token::Type::Pipe:
            binaryOperator = BinaryExpr::Operator::BitwiseOr;
            return true;
        case Lexing::Token::Type::Circumflex:
            binaryOperator = BinaryExpr::Operator::BitwiseXor;
            return true;
        case Lexing::Token::Type::LeftShift:
            binaryOperator = BinaryExpr::Operator::LeftShift;
            return true;
        case Lexing::Token::Type::RightShift:
            binaryOperator = BinaryExpr::Operator::RightShift;
            return true;

        case Lexing::Token::Type::LogicalAnd:
            binaryOperator = BinaryExpr::Operator::And;
            return true;
        case Lexing::Token::Type::LogicalOr:
            binaryOperator = BinaryExpr::Operator::Or;
            return true;
        case Lexing::Token::Type::LogicalEqual:
            binaryOperator = BinaryExpr::Operator::Equal;
            return true;
        case Lexing::Token::Type::LogicalNotEqual:
            binaryOperator = BinaryExpr::Operator::NotEqual;
            return true;
        case Lexing::Token::Type::Greater:
            binaryOperator = BinaryExpr::Operator::Greater;
            return true;
        case Lexing::Token::Type::Less:
            binaryOperator = BinaryExpr::Operator::Less;
            return true;
        case Lexing::Token::Type::LessEqual:
            binaryOperator = BinaryExpr::Operator::LessThan;
            return true;
        case Lexing::Token::Type::GreaterEqual:
            binaryOperator = BinaryExpr::Operator::GreaterThan;
            return true;
        default:
            return false;
    }
}

bool Parse::match(const Lexing::Token::Type &type)
{
    if (type == c_tokens[m_current].m_type) {
        if (advance().m_type == Lexing::Token::Type::EndOfFile)
            return false;
        return true;
    }
    return false;
}

// https://en.cppreference.com/w/c/language/operator_precedence
i32 Parse::getPrecedenceLevel(const Lexing::Token::Type type)
{
    switch (type) {
        case Lexing::Token::Type::Decrement:
            return 1;
        case Lexing::Token::Type::ExclamationMark:
            return 2;
        case Lexing::Token::Type::Asterisk:
        case Lexing::Token::Type::ForwardSlash:
        case Lexing::Token::Type::Percent:
            return 3;
        case Lexing::Token::Type::Plus:
        case Lexing::Token::Type::Minus:
            return 4;
        case Lexing::Token::Type::LeftShift:
        case Lexing::Token::Type::RightShift:
            return 5;
        case Lexing::Token::Type::Less:
        case Lexing::Token::Type::LessEqual:
        case Lexing::Token::Type::Greater:
        case Lexing::Token::Type::GreaterEqual:
            return 6;
        case Lexing::Token::Type::LogicalEqual:
        case Lexing::Token::Type::LogicalNotEqual:
            return 7;
        case Lexing::Token::Type::Ampersand:
            return 8;
        case Lexing::Token::Type::Circumflex:
            return 9;
        case Lexing::Token::Type::Pipe:
            return 10;
        case Lexing::Token::Type::LogicalAnd:
            return 11;
        case Lexing::Token::Type::LogicalOr:
            return 12;
        default:
            return 0;
    }
}

i32 Parse::precedence(const Lexing::Token::Type type)
{
    constexpr i32 precedenceMult = 1024;
    constexpr i32 precedenceLevels = 16;
    return (precedenceLevels - getPrecedenceLevel(type)) * precedenceMult;
}

bool Parse::isBinaryOperator(const Lexing::Token::Type type)
{
    switch (type) {
        case Lexing::Token::Type::Plus:
        case Lexing::Token::Type::Minus:
        case Lexing::Token::Type::ForwardSlash:
        case Lexing::Token::Type::Percent:
        case Lexing::Token::Type::Asterisk:
        case Lexing::Token::Type::LeftShift:
        case Lexing::Token::Type::RightShift:
        case Lexing::Token::Type::Ampersand:
        case Lexing::Token::Type::Pipe:
        case Lexing::Token::Type::Circumflex:

        case Lexing::Token::Type::LogicalAnd:
        case Lexing::Token::Type::LogicalOr:
        case Lexing::Token::Type::LogicalEqual:
        case Lexing::Token::Type::LogicalNotEqual:
        case Lexing::Token::Type::Greater:
        case Lexing::Token::Type::Less:
        case Lexing::Token::Type::LessEqual:
        case Lexing::Token::Type::GreaterEqual:
            return true;
        default:
            return false;
    }
}
} // Parsing