#include "Parser.hpp"
#include "IR/ASTIr.hpp"
#include "Frontend/Lexing/Token.hpp"

#include <cassert>

namespace Parsing {

bool Parser::programParse(Program& program)
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

std::unique_ptr<Function> Parser::functionParse()
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

std::unique_ptr<Block> Parser::blockParse()
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

std::unique_ptr<BlockItem> Parser::blockItemParse()
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

std::unique_ptr<Declaration> Parser::declarationParse()
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

std::unique_ptr<ForInit> Parser::forInitParse()
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

std::unique_ptr<Stmt> Parser::stmtParse()
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
        case TokenType::Switch:
            return switchStmtParse();
        case TokenType::Case:
            return caseStmtParse();
        case TokenType::Default:
            return defaultStmtParse();
        default:
            if (peekNextTokenType() == TokenType::Colon)
                return labelStmtParse();
            return exprStmtParse();
    }
    assert("unreachable stmtParse()");
}

std::unique_ptr<Stmt> Parser::returnStmtParse()
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

std::unique_ptr<Stmt> Parser::exprStmtParse()
{
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    auto statement = std::make_unique<ExprStmt>(std::move(expr));
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return statement;
}

std::unique_ptr<Stmt> Parser::ifStmtParse()
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

std::unique_ptr<Stmt> Parser::gotoStmtParse()
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

std::unique_ptr<Stmt> Parser::breakStmtParse()
{
    if (!expect(TokenType::Break))
        return nullptr;
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<BreakStmt>(m_breakLabel);
}

std::unique_ptr<Stmt> Parser::continueStmtParse()
{
    if (!expect(TokenType::Continue))
        return nullptr;
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<ContinueStmt>(m_continueLabel);
}

std::unique_ptr<Stmt> Parser::labelStmtParse()
{
    Lexing::Token lexeme = peek();
    if (!expect(TokenType::Identifier))
        return nullptr;
    if (!expect(TokenType::Colon))
        return nullptr;
    auto stmt = stmtParse();
    if (stmt == nullptr)
        return nullptr;
    return std::make_unique<LabelStmt>(lexeme.m_lexeme, std::move(stmt));
}

std::unique_ptr<Stmt> Parser::caseStmtParse()
{
    if (!expect(TokenType::Case))
        return nullptr;
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    if (!expect(TokenType::Colon))
        return nullptr;
    std::unique_ptr<Stmt> stmt = stmtParse();
    if (stmt == nullptr)
        return nullptr;
    return std::make_unique<CaseStmt>(m_switchLabel, std::move(expr), std::move(stmt));
}

std::unique_ptr<Stmt> Parser::defaultStmtParse()
{
    if (!expect(TokenType::Default))
        return nullptr;
    if (!expect(TokenType::Colon))
        return nullptr;
    std::unique_ptr<Stmt> stmt = stmtParse();
    if (stmt == nullptr)
        return nullptr;
    return std::make_unique<DefaultStmt>(m_switchLabel, std::move(stmt));
}

std::unique_ptr<Stmt> Parser::whileStmtParse()
{
    const std::string continueTemp = m_continueLabel;
    const std::string breakTemp = m_breakLabel;
    const std::string whileLabel = makeTemporary("while");
    m_continueLabel = whileLabel;
    m_breakLabel = whileLabel;
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
    m_continueLabel = continueTemp;
    m_breakLabel = breakTemp;
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body), whileLabel);
}

std::unique_ptr<Stmt> Parser::doWhileStmtParse()
{
    const std::string continueTemp = m_continueLabel;
    const std::string breakTemp = m_breakLabel;
    const std::string doLabel = makeTemporary("do.While");
    m_continueLabel = doLabel;
    m_breakLabel = doLabel;
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
    m_continueLabel = continueTemp;
    m_breakLabel = breakTemp;
    return std::make_unique<DoWhileStmt>(std::move(body), std::move(condition), doLabel);
}

std::unique_ptr<Stmt> Parser::forStmtParse()
{
    const std::string continueTemp = m_continueLabel;
    const std::string breakTemp = m_breakLabel;
    const std::string forLabel = makeTemporary("for");
    m_continueLabel = forLabel;
    m_breakLabel = forLabel;
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
    auto result = std::make_unique<ForStmt>(std::move(body), forLabel);
    if (init != nullptr)
        result->init = std::move(init);
    if (condition != nullptr)
        result->condition = std::move(condition);
    if (post != nullptr)
        result->post = std::move(post);
    m_continueLabel = continueTemp;
    m_breakLabel = breakTemp;
    return result;
}

std::unique_ptr<Stmt> Parser::switchStmtParse()
{
    const std::string breakTemp = m_breakLabel;
    const std::string switchTemp = m_switchLabel;
    const std::string switchLabel = makeTemporary("switch");
    m_breakLabel = switchLabel;
    m_switchLabel = switchLabel;
    if (!expect(TokenType::Switch))
        return nullptr;
    if (!expect(TokenType::OpenParen))
        return nullptr;
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    if (!expect(TokenType::CloseParen))
        return nullptr;
    std::unique_ptr<Stmt> body = stmtParse();
    if (body == nullptr)
        return nullptr;
    m_breakLabel = breakTemp;
    m_switchLabel = switchTemp;
    return std::make_unique<SwitchStmt>(switchLabel, std::move(expr), std::move(body));
}

std::unique_ptr<Stmt> Parser::nullStmtParse()
{
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<NullStmt>();
}

std::unique_ptr<Expr> Parser::exprParse(const i32 minPrecedence)
{
    auto left = unaryExprParse();
    if (left == nullptr)
        return nullptr;
    Lexing::Token nextToken = peek();
    while (continuePrecedenceClimbing(minPrecedence, nextToken.m_type)) {
        advance();
        if (nextToken.m_type == TokenType::QuestionMark) {
            auto first = exprParse(0);
            if (first == nullptr)
                return nullptr;
            if (!expect(TokenType::Colon))
                return nullptr;
            auto second = exprParse(Operators::precedence(TokenType::Colon));
            if (second == nullptr)
                return nullptr;
            left = std::make_unique<ConditionalExpr>(
                std::move(left), std::move(first), std::move(second)
            );
        }
        if (Operators::isAssignmentOperator(nextToken.m_type)) {
            AssignmentExpr::Operator op = Operators::assignOperator(nextToken.m_type);
            auto right = exprParse(Operators::precedence(nextToken.m_type));
            if (right == nullptr)
                return nullptr;
            left = std::make_unique<AssignmentExpr>(op, std::move(left), std::move(right));
        }
        if (Operators::isBinaryOperator(nextToken.m_type)) {
            BinaryExpr::Operator op = Operators::binaryOperator(nextToken.m_type);
            auto right = exprParse(Operators::precedence(nextToken.m_type) + 1);
            if (right == nullptr)
                return nullptr;
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        nextToken = peek();
    }
    return left;
}

std::unique_ptr<Expr> Parser::unaryExprParse()
{
    if (!Operators::isUnaryOperator(peek().m_type))
        return exprPostfix();
    UnaryExpr::Operator oper = Operators::unaryOperator(peek().m_type);
    advance();
    std::unique_ptr<Expr> expr = unaryExprParse();
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<UnaryExpr>(oper, std::move(expr));
}

std::unique_ptr<Expr> Parser::exprPostfix()
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

std::unique_ptr<Expr> Parser::factorParse()
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

bool Parser::match(const TokenType &type)
{
    if (type == peek().m_type) {
        if (advance().m_type == TokenType::EndOfFile)
            return false;
        return true;
    }
    return false;
}

bool Parser::expect(const TokenType type)
{
    if (peek().m_type == type) {
        if (advance().m_type == TokenType::EndOfFile)
            return false;
        return true;
    }
    return false;
}

Lexing::Token::Type Parser::peekNextTokenType() const
{
    if (c_tokens.size() <= m_current + 1)
        return TokenType::EndOfFile;
    return c_tokens[m_current + 1].m_type;
}
} // Parsing