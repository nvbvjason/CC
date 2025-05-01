#include "ConcreteTree.hpp"
#include "IR/AbstractTree.hpp"
#include "Lexing/Token.hpp"

namespace Parsing {

bool Parse::programParse(Program& program)
{
    std::unique_ptr<Function> function = functionParse();
    if (function == nullptr)
        return false;
    program.function = std::move(function);
    if (program.function->name != "main")
        return false;
    if (peek().m_type != TokenType::EndOfFile)
        return false;
    return true;
}

std::unique_ptr<Function> Parse::functionParse()
{
    if (!expect(TokenType::IntKeyword))
        return nullptr;
    const Lexing::Token lexeme = advance();
    if (lexeme.m_type != TokenType::Identifier)
        return nullptr;
    std::string iden = lexeme.m_lexeme;
    if (!expect(TokenType::OpenParen))
        return nullptr;
    if (!expect(TokenType::Void))
        return nullptr;
    if (!expect(TokenType::CloseParen))
        return nullptr;
    if (!expect(TokenType::OpenBrace))
        return nullptr;
    auto function = std::make_unique<Function>(iden);
    while (!expect(TokenType::CloseBrace)) {
        std::unique_ptr<BlockItem> blockItem = blockItemParse();
        if (blockItem == nullptr)
            return nullptr;
        function->body.push_back(std::move(blockItem));
    }
    return function;
}

std::unique_ptr<BlockItem> Parse::blockItemParse()
{
    if (peek().m_type == TokenType::IntKeyword) {
        std::unique_ptr<Declaration> declaration = declarationParse();
        if (declaration == nullptr)
            return nullptr;
        return std::make_unique<DeclarationBlockItem>(std::move(declaration));
    }
    std::unique_ptr<Stmt> statement = stmtParse();
    if (statement == nullptr)
        return nullptr;
    return std::make_unique<StmtBlockItem>(std::move(statement));
}

std::unique_ptr<Declaration> Parse::declarationParse()
{
    if (!expect(TokenType::IntKeyword))
        return nullptr;
    const Lexing::Token lexeme = advance();
    if (lexeme.m_type != TokenType::Identifier)
        return nullptr;
    auto declaration = std::make_unique<Declaration>(lexeme.m_lexeme);
    if (expect(TokenType::Equal)) {
        std::unique_ptr<Expr> expr = exprParse(0);
        if (expr == nullptr)
            return nullptr;
        declaration->init = std::move(expr);
    }
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return declaration;
}

std::unique_ptr<Stmt> Parse::stmtParse()
{
    if (expect(TokenType::Semicolon))
        return std::make_unique<NullStmt>();
    if (expect(TokenType::Return)) {
        std::unique_ptr<Expr> expr = exprParse(0);
        if (expr == nullptr)
            return nullptr;
        auto statement = std::make_unique<ReturnStmt>(std::move(expr));
        if (!expect(TokenType::Semicolon))
            return nullptr;
        return statement;
    }
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    auto statement = std::make_unique<ExprStmt>(std::move(expr));
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return statement;
}

std::unique_ptr<Expr> Parse::exprParse(const i32 minPrecedence)
{
    auto left = factorParse();
    if (left == nullptr)
        return nullptr;
    Lexing::Token nextToken = peek();
    while (isBinaryOperator(nextToken.m_type) && minPrecedence <= precedence(binaryOperator(nextToken.m_type))) {
        BinaryExpr::Operator op = binaryOperator(nextToken.m_type);
        advance();
        if (op == BinaryExpr::Operator::Assign) {
            auto right = exprParse(precedence(op));
            if (right == nullptr)
                return nullptr;
            left = std::make_unique<AssignmentExpr>(std::move(left), std::move(right));
            nextToken = peek();
        }
        else {
            auto right = exprParse(precedence(op) + 1);
            if (right == nullptr)
                return nullptr;
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
            nextToken = peek();
        }
    }
    return left;
}

std::unique_ptr<Expr> Parse::factorParse()
{
    switch (const Lexing::Token lexeme = peek(); lexeme.m_type) {
        case TokenType::Integer: {
            auto constantExpr = std::make_unique<ConstExpr>(std::stoi(lexeme.m_lexeme));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::Identifier:
            advance();
            return std::make_unique<VarExpr>(lexeme.m_lexeme);
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

std::unique_ptr<Expr> Parse::unaryExprParse()
{
    if (!isUnaryOperator(peek().m_type))
        return nullptr;
    UnaryExpr::Operator oper = unaryOperator(peek().m_type);
    advance();
    std::unique_ptr<Expr> expr = exprParse(precedence(oper));
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<UnaryExpr>(oper, std::move(expr));
}

UnaryExpr::Operator Parse::unaryOperator(const TokenType type)
{
    switch (type) {
        case TokenType::Minus:
            return UnaryExpr::Operator::Negate;
        case TokenType::Tilde:
            return UnaryExpr::Operator::Complement;
        case TokenType::ExclamationMark:
            return UnaryExpr::Operator::Not;
        default:
            throw std::runtime_error("Invalid unary operator unaryOperator");
    }
}

BinaryExpr::Operator Parse::binaryOperator(const TokenType type)
{
    switch (type) {
         case TokenType::Plus:
            return BinaryExpr::Operator::Add;
        case TokenType::Minus:
            return BinaryExpr::Operator::Subtract;
        case TokenType::Asterisk:
            return BinaryExpr::Operator::Multiply;
        case TokenType::ForwardSlash:
            return BinaryExpr::Operator::Divide;
        case TokenType::Percent:
            return BinaryExpr::Operator::Remainder;

        case TokenType::Ampersand:
            return BinaryExpr::Operator::BitwiseAnd;
        case TokenType::Pipe:
            return BinaryExpr::Operator::BitwiseOr;
        case TokenType::Circumflex:
            return BinaryExpr::Operator::BitwiseXor;
        case TokenType::LeftShift:
            return BinaryExpr::Operator::LeftShift;
        case TokenType::RightShift:
            return BinaryExpr::Operator::RightShift;

        case TokenType::LogicalAnd:
            return BinaryExpr::Operator::And;
        case TokenType::LogicalOr:
            return BinaryExpr::Operator::Or;
        case TokenType::LogicalEqual:
            return BinaryExpr::Operator::Equal;
        case TokenType::LogicalNotEqual:
            return BinaryExpr::Operator::NotEqual;
        case TokenType::Greater:
            return BinaryExpr::Operator::GreaterThan;
        case TokenType::Less:
            return BinaryExpr::Operator::LessThan;
        case TokenType::LessOrEqual:
            return BinaryExpr::Operator::LessOrEqual;
        case TokenType::GreaterOrEqual:
            return BinaryExpr::Operator::GreaterOrEqual;

        case TokenType::Equal:
            return BinaryExpr::Operator::Assign;
        default:
            throw std::runtime_error("Invalid binary operator binaryOperator");
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
i32 Parse::getPrecedenceLevel(const UnaryExpr::Operator oper)
{
    using Operator = UnaryExpr::Operator;
    switch (oper) {
        case Operator::Complement:
        case Operator::Negate:
        case Operator::Not:
            return 2;
        default:
            return 0;
    }
}

i32 Parse::precedence(const UnaryExpr::Operator oper)
{
    constexpr i32 precedenceMult = 1024;
    constexpr i32 precedenceLevels = 16;
    return (precedenceLevels - getPrecedenceLevel(oper)) * precedenceMult;
}

// https://en.cppreference.com/w/c/language/operator_precedence
i32 Parse::getPrecedenceLevel(const BinaryExpr::Operator oper)
{
    using Operator = BinaryExpr::Operator;
    switch (oper) {
        case Operator::Multiply:
        case Operator::Divide:
        case Operator::Remainder:
            return 3;
        case Operator::Add:
        case Operator::Subtract:
            return 4;
        case Operator::LeftShift:
        case Operator::RightShift:
            return 5;
        case Operator::LessThan:
        case Operator::LessOrEqual:
        case Operator::GreaterThan:
        case Operator::GreaterOrEqual:
            return 6;
        case Operator::Equal:
        case Operator::NotEqual:
            return 7;
        case Operator::BitwiseAnd:
            return 8;
        case Operator::BitwiseXor:
            return 9;
        case Operator::BitwiseOr:
            return 10;
        case Operator::And:
            return 11;
        case Operator::Or:
            return 12;
        case Operator::Assign:
            return 14;
        default:
            throw std::runtime_error("Invalid binary operator getPrecedenceLevel");
    }
}

i32 Parse::precedence(const BinaryExpr::Operator oper)
{
    constexpr i32 precedenceMult = 1024;
    constexpr i32 precedenceLevels = 16;
    return (precedenceLevels - getPrecedenceLevel(oper)) * precedenceMult;
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

        case TokenType::Equal:
            return true;
        default:
            return false;
    }
}

bool Parse::isUnaryOperator(const TokenType type)
{
    switch (type) {
        case TokenType::Minus:
        case TokenType::Tilde:
        case TokenType::ExclamationMark:
            return true;
        default:
            return false;
    }
}
} // Parsing