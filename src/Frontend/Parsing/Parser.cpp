#include "Parser.hpp"
#include "IR/ASTIr.hpp"
#include "Frontend/Lexing/Token.hpp"

#include <cassert>

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
    auto function = std::make_unique<Function>(iden);
    auto block = blockParse();
    if (block == nullptr)
        return nullptr;
    function->body = std::move(block);
    return function;
}

std::unique_ptr<Block> Parse::blockParse()
{
    auto block = std::make_unique<Block>();
    if (!expect(TokenType::OpenBrace))
        return nullptr;
    if (expect(TokenType::CloseBrace))
        return block;
    while (true) {
        std::unique_ptr<BlockItem> blockItem = blockItemParse();
        if (blockItem == nullptr)
            return nullptr;
        block->body.push_back(std::move(blockItem));
        if (expect(TokenType::CloseBrace))
            break;
    }
    return block;
}

std::unique_ptr<BlockItem> Parse::blockItemParse()
{
    if (peek().m_type == TokenType::IntKeyword) {
        std::unique_ptr<Declaration> declaration = declarationParse();
        if (declaration == nullptr)
            return nullptr;
        return std::make_unique<DeclBlockItem>(std::move(declaration));
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

std::unique_ptr<ForInit> Parse::forInitParse()
{
    if (peek().m_type == TokenType::Semicolon)
        return nullptr;
    if (expect(TokenType::IntKeyword)) {
        const Lexing::Token lexeme = advance();
        if (lexeme.m_type != TokenType::Identifier)
            return nullptr;
        auto decl = std::make_unique<Declaration>(lexeme.m_lexeme);
        if (peek().m_type != TokenType::Equal)
            return std::make_unique<DeclForInit>(std::move(decl));
        advance();
        std::unique_ptr<Expr> expr = exprParse(0);
        if (expr == nullptr)
            return nullptr;
        decl->init = std::move(expr);
        return std::make_unique<DeclForInit>(std::move(decl));
    }
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<ExprForInit>(std::move(expr));
}

std::unique_ptr<Stmt> Parse::stmtParse()
{
    switch (peek().m_type) {
        case TokenType::Return:
            return returnStmtParse();
        case TokenType::Semicolon:
            return nullStmtParse();
        case TokenType::If:
            return ifStmtParse();
        case TokenType::Goto:
            return gotoStmtParse();
        case TokenType::OpenBrace:
            return std::make_unique<CompoundStmt>(blockParse());
        case TokenType::Break:
            return breakStmtParse();
        case TokenType::Continue:
            return continueStmtParse();
        case TokenType::While:
            return whileStmtParse();
        case TokenType::Do:
            return doWhileStmtParse();
        case TokenType::For:
            return forStmtParse();
        default:
            if (peekNextTokenType() == TokenType::Colon)
                return labelStmtParse();
            return exprStmtParse();
    }
    assert("unreachable stmtParse()");
}

std::unique_ptr<Stmt> Parse::returnStmtParse()
{
    if (!expect(TokenType::Return))
        return nullptr;
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    auto statement = std::make_unique<ReturnStmt>(std::move(expr));
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return statement;
}

std::unique_ptr<Stmt> Parse::exprStmtParse()
{
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    auto statement = std::make_unique<ExprStmt>(std::move(expr));
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return statement;
}

std::unique_ptr<Stmt> Parse::ifStmtParse()
{
    if (!expect(TokenType::If))
        return nullptr;
    if (!expect(TokenType::OpenParen))
        return nullptr;
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    if (!expect(TokenType::CloseParen))
        return nullptr;
    std::unique_ptr<Stmt> thenStmt = stmtParse();
    if (thenStmt == nullptr)
        return nullptr;
    if (expect(TokenType::Else)) {
        std::unique_ptr<Stmt> elseStmt = stmtParse();
        if (elseStmt == nullptr)
            return nullptr;
        return std::make_unique<IfStmt>(std::move(expr), std::move(thenStmt), std::move(elseStmt));
    }
    return std::make_unique<IfStmt>(std::move(expr), std::move(thenStmt));
}

std::unique_ptr<Stmt> Parse::gotoStmtParse()
{
    if (!expect(TokenType::Goto))
        return nullptr;
    Lexing::Token lexeme = peek();
    if (!expect(TokenType::Identifier))
        return nullptr;
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<GotoStmt>(lexeme.m_lexeme);
}

std::unique_ptr<Stmt> Parse::breakStmtParse()
{
    if (!expect(TokenType::Break))
        return nullptr;
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<BreakStmt>(m_label);
}

std::unique_ptr<Stmt> Parse::continueStmtParse()
{
    if (!expect(TokenType::Continue))
        return nullptr;
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<ContinueStmt>(m_label);
}

std::unique_ptr<Stmt> Parse::labelStmtParse()
{
    Lexing::Token lexeme = peek();
    if (!expect(TokenType::Identifier))
        return nullptr;
    if (!expect(TokenType::Colon))
        return nullptr;
    return std::make_unique<LabelStmt>(lexeme.m_lexeme);
}

std::unique_ptr<Stmt> Parse::whileStmtParse()
{
    const std::string labelTemp = m_label;
    m_label = makeTemporary("while");
    if (!expect(TokenType::While))
        return nullptr;
    if (!expect(TokenType::OpenParen))
        return nullptr;
    std::unique_ptr<Expr> condition = exprParse(0);
    if (condition == nullptr)
        return nullptr;
    if (!expect(TokenType::CloseParen))
        return nullptr;
    std::unique_ptr<Stmt> body = stmtParse();
    if (body == nullptr)
        return nullptr;
    auto result = std::make_unique<WhileStmt>(std::move(condition), std::move(body), m_label);
    m_label = labelTemp;
    return result;
}

std::unique_ptr<Stmt> Parse::doWhileStmtParse()
{
    const std::string labelTemp = m_label;
    m_label = makeTemporary("do.While");
    if (!expect(TokenType::Do))
        return nullptr;
    std::unique_ptr<Stmt> body = stmtParse();
    if (body == nullptr)
        return nullptr;
    if (!expect(TokenType::While))
        return nullptr;
    if (!expect(TokenType::OpenParen))
        return nullptr;
    std::unique_ptr<Expr> condition = exprParse(0);
    if (condition == nullptr)
        return nullptr;
    if (!expect(TokenType::CloseParen))
        return nullptr;
    if (!expect(TokenType::Semicolon))
        return nullptr;
    auto result = std::make_unique<DoWhileStmt>(std::move(body), std::move(condition), m_label);
    m_label = labelTemp;
    return result;
}

std::unique_ptr<Stmt> Parse::forStmtParse()
{
    const std::string labelTemp = m_label;
    m_label = makeTemporary("for");
    if (!expect(TokenType::For))
        return nullptr;
    if (!expect(TokenType::OpenParen))
        return nullptr;
    std::unique_ptr<ForInit> init = forInitParse();
    if (!expect(TokenType::Semicolon))
        return nullptr;
    std::unique_ptr<Expr> condition = exprParse(0);
    if (!expect(TokenType::Semicolon))
        return nullptr;
    std::unique_ptr<Expr> post = exprParse(0);
    if (!expect(TokenType::CloseParen))
        return nullptr;
    std::unique_ptr<Stmt> body = stmtParse();
    if (body == nullptr)
        return nullptr;
    auto result = std::make_unique<ForStmt>(std::move(body), m_label);
    if (init != nullptr)
        result->init = std::move(init);
    if (condition != nullptr)
        result->condition = std::move(condition);
    if (post != nullptr)
        result->post = std::move(post);
    m_label = labelTemp;
    return result;
}

std::unique_ptr<Stmt> Parse::nullStmtParse()
{
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<NullStmt>();
}

std::unique_ptr<Expr> Parse::exprParse(const i32 minPrecedence)
{
    auto left = unaryExprParse();
    if (left == nullptr)
        return nullptr;
    Lexing::Token nextToken = peek();
    while ((isBinaryOperator(nextToken.m_type)
            || isAssignmentOperator(nextToken.m_type)
            || nextToken.m_type == TokenType::QuestionMark)
        && minPrecedence <= precedence(nextToken.m_type)) {
        advance();
        if (nextToken.m_type == TokenType::QuestionMark) {
            auto first = exprParse(0);
            if (first == nullptr)
                return nullptr;
            if (!expect(TokenType::Colon))
                return nullptr;
            auto second = exprParse(precedence(TokenType::Colon));
            if (second == nullptr)
                return nullptr;
            left = std::make_unique<ConditionalExpr>(
                std::move(left), std::move(first), std::move(second)
            );
        }
        if (isAssignmentOperator(nextToken.m_type)) {
            AssignmentExpr::Operator op = assignOperator(nextToken.m_type);
            auto right = exprParse(precedence(nextToken.m_type));
            if (right == nullptr)
                return nullptr;
            left = std::make_unique<AssignmentExpr>(op, std::move(left), std::move(right));
        }
        if (isBinaryOperator(nextToken.m_type)) {
            BinaryExpr::Operator op = binaryOperator(nextToken.m_type);
            auto right = exprParse(precedence(nextToken.m_type) + 1);
            if (right == nullptr)
                return nullptr;
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        nextToken = peek();
    }
    return left;
}

std::unique_ptr<Expr> Parse::unaryExprParse()
{
    if (!isUnaryOperator(peek().m_type))
        return exprPostfix();
    UnaryExpr::Operator oper = unaryOperator(peek().m_type);
    advance();
    std::unique_ptr<Expr> expr = unaryExprParse();
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<UnaryExpr>(oper, std::move(expr));
}

std::unique_ptr<Expr> Parse::exprPostfix()
{
    auto expr = factorParse();
    while (true) {
        if (expect(TokenType::Increment))
            expr = std::make_unique<UnaryExpr>(UnaryExpr::Operator::PostFixIncrement, std::move(expr));
        else if (expect(TokenType::Decrement))
            expr = std::make_unique<UnaryExpr>(UnaryExpr::Operator::PostFixDecrement, std::move(expr));
        else
            break;
    }
    return expr;
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
        case TokenType::Identifier: {
            advance();
            return std::make_unique<VarExpr>(lexeme.m_lexeme);
        }
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

bool Parse::match(const TokenType &type)
{
    if (type == peek().m_type) {
        if (advance().m_type == TokenType::EndOfFile)
            return false;
        return true;
    }
    return false;
}

bool Parse::expect(const TokenType type)
{
    if (peek().m_type == type) {
        if (advance().m_type == TokenType::EndOfFile)
            return false;
        return true;
    }
    return false;
}

Lexing::Token::Type Parse::peekNextTokenType() const
{
    if (c_tokens.size() <= m_current + 1)
        return TokenType::EndOfFile;
    return c_tokens[m_current + 1].m_type;
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
        case TokenType::Increment:
            return UnaryExpr::Operator::PrefixIncrement;
        case TokenType::Decrement:
            return UnaryExpr::Operator::PrefixDecrement;
        default:
            assert("Invalid unary operator unaryOperator");
    }
}

BinaryExpr::Operator Parse::binaryOperator(const TokenType type)
{
    using Operator = BinaryExpr::Operator;
    switch (type) {
        // Arithmetic operators
        case TokenType::Plus:               return Operator::Add;
        case TokenType::Minus:              return Operator::Subtract;
        case TokenType::Asterisk:           return Operator::Multiply;
        case TokenType::ForwardSlash:       return Operator::Divide;
        case TokenType::Percent:            return Operator::Remainder;

        // Bitwise operators
        case TokenType::Ampersand:          return Operator::BitwiseAnd;
        case TokenType::Pipe:               return Operator::BitwiseOr;
        case TokenType::Circumflex:         return Operator::BitwiseXor;
        case TokenType::LeftShift:          return Operator::LeftShift;
        case TokenType::RightShift:         return Operator::RightShift;

        // Logical/comparison operators
        case TokenType::LogicalAnd:         return Operator::And;
        case TokenType::LogicalOr:          return Operator::Or;
        case TokenType::LogicalEqual:       return Operator::Equal;
        case TokenType::LogicalNotEqual:    return Operator::NotEqual;
        case TokenType::Greater:            return Operator::GreaterThan;
        case TokenType::Less:               return Operator::LessThan;
        case TokenType::LessOrEqual:        return Operator::LessOrEqual;
        case TokenType::GreaterOrEqual:     return Operator::GreaterOrEqual;

        default:
            assert("Invalid binary operator: binaryOperator(Token::Type)");
    }
}

AssignmentExpr::Operator Parse::assignOperator(TokenType type)
{
    using Operator = AssignmentExpr::Operator;
    switch (type) {
        case TokenType::Equal:              return Operator::Assign;
        case TokenType::PlusAssign:         return Operator::PlusAssign;
        case TokenType::MinusAssign:        return Operator::MinusAssign;
        case TokenType::MultiplyAssign:     return Operator::MultiplyAssign;
        case TokenType::DivideAssign:       return Operator::DivideAssign;
        case TokenType::ModuloAssign:       return Operator::ModuloAssign;
        case TokenType::BitwiseAndAssign:   return Operator::BitwiseAndAssign;
        case TokenType::BitwiseOrAssign:    return Operator::BitwiseOrAssign;
        case TokenType::BitwiseXorAssign:   return Operator::BitwiseXorAssign;
        case TokenType::LeftShiftAssign:    return Operator::LeftShiftAssign;
        case TokenType::RightShiftAssign:   return Operator::RightShiftAssign;

        default:
            assert("Invalid binary operator: assignOperator(Token::Type)");
    }
}
// https://en.cppreference.com/w/c/language/operator_precedence
i32 Parse::getPrecedenceLevel(const UnaryExpr::Operator oper)
{
    using Operator = UnaryExpr::Operator;
    switch (oper) {
        case Operator::PostFixIncrement:
        case Operator::PostFixDecrement:
            return 1;
        case Operator::PrefixIncrement:
        case Operator::PrefixDecrement:
        case Operator::Complement:
        case Operator::Negate:
        case Operator::Not:
            return 2;
        default:
            return 0;
    }
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
        default:
            assert("Invalid binary operator getPrecedenceLevel");
    }
}

// https://en.cppreference.com/w/c/language/operator_precedence
i32 Parse::getPrecedenceLevel(const AssignmentExpr::Operator oper)
{
    return 14;
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

bool Parse::isUnaryOperator(const TokenType type)
{
    switch (type) {
        case TokenType::Minus:
        case TokenType::Tilde:
        case TokenType::ExclamationMark:
        case TokenType::Increment:
        case TokenType::Decrement:
            return true;
        default:
            return false;
    }
}

bool Parse::isAssignmentOperator(TokenType type)
{
    switch (type) {
        case TokenType::Equal:
        case TokenType::PlusAssign:
        case TokenType::MinusAssign:
        case TokenType::DivideAssign:
        case TokenType::MultiplyAssign:
        case TokenType::ModuloAssign:
        case TokenType::BitwiseAndAssign:
        case TokenType::BitwiseOrAssign:
        case TokenType::BitwiseXorAssign:
        case TokenType::LeftShiftAssign:
        case TokenType::RightShiftAssign:
            return true;
        default:
            return false;
    }
}

} // Parsing