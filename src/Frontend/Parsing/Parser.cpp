#include "Parser.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "TypeConversion.hpp"

#include <algorithm>
#include <cassert>
#include <tuple>

namespace Parsing {

std::vector<Error> Parser::programParse(Program& program)
{
    while (!isAtEnd()) {
        std::unique_ptr<Declaration> declaration = declarationParse();
        if (declaration == nullptr) {
            addError("Could not parse declarator");
            return m_errors;
        }
        program.declarations.push_back(std::move(declaration));
    }
    return m_errors;
}

std::unique_ptr<Declaration> Parser::declarationParse()
{
    auto [type, storageClass] = specifierParse();
    if (type == Type::Invalid)
        return nullptr;
    const Declaration::StorageClass storage = Operators::getStorageClass(storageClass);
    std::unique_ptr<Declarator> declarator = declaratorParse();
    if (declarator == nullptr)
        return nullptr;
    auto varType = std::make_unique<VarType>(type);
    auto [iden, typeBase, params] =
            declaratorProcess(std::move(declarator), std::move(varType));
    if (iden.empty())
        return nullptr;
    if (typeBase->type == Type::Function)
        return funDeclParse(iden, std::move(typeBase), storage, std::move(params));
    return varDeclParse(iden, std::move(typeBase), storage);
}

std::unique_ptr<VarDecl> Parser::varDeclParse(const std::string& iden,
                                              std::unique_ptr<TypeBase>&& type,
                                              const Storage storage)
{
    std::unique_ptr<Initializer> init = nullptr;
    if (expect(TokenType::Equal)) {
        init = initializerParse();
        if (init == nullptr) {
            addError("no initialization expression for variable declaration after =");
            return nullptr;
        }
    }
    if (!expect(TokenType::Semicolon)) {
        addError("Expected semicolon after variable declaration");
        return nullptr;
    }
    auto varDecl = std::make_unique<VarDecl>(m_current, storage, iden, std::move(type));
    if (init)
        varDecl->init = std::move(init);
    return varDecl;
}

std::unique_ptr<FunDeclaration> Parser::funDeclParse(
        const std::string& iden,
        std::unique_ptr<TypeBase>&& type,
        const Storage storage,
        std::vector<std::string>&& params
    )
{
    auto result = std::make_unique<FunDeclaration>(m_current, storage, iden, std::move(params), std::move(type));
    if (expect(TokenType::Semicolon))
        return result;
    const size_t before = m_current;
    auto block = blockParse();
    if (block == nullptr) {
        addError("Expected block after function declaration", before);
        return nullptr;
    }
    result->body = std::move(block);
    return result;
}

std::unique_ptr<Declarator> Parser::declaratorParse()
{
    if (expect(TokenType::Asterisk)) {
        std::unique_ptr<Declarator> result = declaratorParse();
        if (result == nullptr)
            return nullptr;
        return std::make_unique<PointerDeclarator>(std::move(result));
    }
    return directDeclaratorParse();
}

std::unique_ptr<Declarator> Parser::directDeclaratorParse()
{
    std::unique_ptr<Declarator> result = simpleDeclaratorParse();
    if (result == nullptr)
        return nullptr;
    if (peekTokenType() == TokenType::OpenSqBracket)
        return arrayDeclaratorParse(std::move(result));
    if (peekTokenType() == TokenType::OpenParen) {
        const std::unique_ptr<std::vector<ParamInfo>> paramList = paramsListParse();
        if (paramList == nullptr)
            return nullptr;
        return std::make_unique<FunctionDeclarator>(std::move(result), std::move(*paramList));
    }
    return result;
}

std::unique_ptr<Declarator> Parser::arrayDeclaratorParse(std::unique_ptr<Declarator>&& declarator)
{
    if (peekTokenType() != TokenType::OpenSqBracket)
        return nullptr;
    while (expect(TokenType::OpenSqBracket)) {
        auto expr = constExprParse();
        if (expr == nullptr) {
            addError("Unexpected token in array declaration", m_current - 1);
            return nullptr;
        }
        if (expr->type->type == Type::Double) {
            addError("Double cannot be an array size", m_current - 1);
            return nullptr;
        }
        const auto constExpr = dynCast<ConstExpr>(expr.get());
        const i64 size = constExpr->getValue<i64>();
        declarator = std::make_unique<ArrayDeclarator>(std::move(declarator), size);
        if (!expect(TokenType::CloseSqBracket)) {
            addError("Expected closing bracket", m_current);
            return nullptr;
        }
    }
    return declarator;
}

std::unique_ptr<Declarator> Parser::simpleDeclaratorParse()
{
    if (expect(TokenType::OpenParen)) {
        std::unique_ptr<Declarator> result = declaratorParse();
        if (peekTokenType() != TokenType::CloseParen)
            return nullptr;
        advance();
        return result;
    }
    if (peekTokenType() != TokenType::Identifier) {
        addError("Expected identifier");
        return nullptr;
    }
    std::string iden = peek().m_lexeme;
    advance();
    return std::make_unique<IdentifierDeclarator>(std::move(iden));
}

std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>> Parser::processFunctionDeclarator(
    std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase)
{
    const auto funcDecl = dynCast<FunctionDeclarator>(declarator.get());
    if (funcDecl->declarator->kind != Declarator::Kind::Identifier)
        return {};
    std::vector<std::unique_ptr<TypeBase>> paramTypes;
    std::vector<std::string> params;
    for (ParamInfo& param : funcDecl->params) {
        auto [iden, typeBaseParam, _] =
                declaratorProcess(std::move(param.declarator), std::move(param.type));
        if (typeBaseParam->type == Type::Function)
            return {};
        params.emplace_back(std::move(iden));
        paramTypes.emplace_back(std::move(typeBaseParam));
    }
    const auto idenDecl = dynCast<IdentifierDeclarator>(funcDecl->declarator.get());
    auto funcType = std::make_unique<FuncType>(std::move(typeBase), std::move(paramTypes));
    return std::make_tuple(std::move(idenDecl->identifier), std::move(funcType), std::move(params));
}

std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>> Parser::declaratorProcess(
    std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase)
{
    switch (declarator->kind) {
        case Declarator::Kind::Identifier: {
            const auto identifierDeclarator = dynCast<IdentifierDeclarator>(declarator.get());
            return std::make_tuple(std::move(identifierDeclarator->identifier),
                                   std::move(typeBase),
                                   std::move(std::vector<std::string>()));
        }
        case Declarator::Kind::Pointer: {
            auto pointerType = std::make_unique<PointerType>(std::move(typeBase));
            const auto pointerDeclarator = dynCast<PointerDeclarator>(declarator.get());
            return declaratorProcess(std::move(pointerDeclarator->inner), std::move(pointerType));
        }
        case Declarator::Kind::Function: {
            return processFunctionDeclarator(std::move(declarator), std::move(typeBase));
        }
        case Declarator::Kind::Array: {
            const auto arrayDeclarator = dynCast<ArrayDeclarator>(declarator.get());
            auto arrayType = std::make_unique<ArrayType>(std::move(typeBase), std::move(arrayDeclarator->size));
            return declaratorProcess(std::move(arrayDeclarator->declarator), std::move(arrayType));
        }
    }
    return std::make_tuple(std::string(), std::move(typeBase), std::vector<std::string>());
}

std::unique_ptr<std::vector<ParamInfo>> Parser::paramsListParse()
{
    if (!expect(TokenType::OpenParen))
        return nullptr;
    std::vector<ParamInfo> params;
    if (expect(TokenType::Void)) {
        if (!expect(TokenType::CloseParen)) {
            addError("Expected close parenthesis after param list");
            return nullptr;
        }
        return std::make_unique<std::vector<ParamInfo>>(std::move(params));
    }
    do {
        std::unique_ptr<ParamInfo> param = paramParse();
        if (param == nullptr)
            return nullptr;
        params.push_back(std::move(*param));
    } while (expect(TokenType::Comma));
    if (!expect(TokenType::CloseParen)) {
        addError("Expected close paren after param list");
        return nullptr;
    }
    return std::make_unique<std::vector<ParamInfo>>(std::move(params));
}

std::unique_ptr<ParamInfo> Parser::paramParse()
{
    const Type type = typeParse();
    if (type == Type::Invalid) {
        addError("Could not parse type");
        return nullptr;
    }
    std::unique_ptr<Declarator> declarator = declaratorParse();
    if (declarator == nullptr)
        return nullptr;
    std::unique_ptr<TypeBase> typeExpr = std::make_unique<VarType>(type);
    return std::make_unique<ParamInfo>(std::move(typeExpr), std::move(declarator));
}

std::unique_ptr<Block> Parser::blockParse()
{
    auto block = std::make_unique<Block>();
    if (!expect(TokenType::OpenBrace))
        return nullptr;
    if (expect(TokenType::CloseBrace))
        return block;
    do {
        std::unique_ptr<BlockItem> blockItem = blockItemParse();
        if (blockItem == nullptr)
            return nullptr;
        block->body.push_back(std::move(blockItem));
    } while (!expect(TokenType::CloseBrace));
    return block;
}

std::unique_ptr<BlockItem> Parser::blockItemParse()
{
    if (Operators::isSpecifier(peekTokenType())) {
        std::unique_ptr<Declaration> declaration = declarationParse();
        if (declaration == nullptr)
            return nullptr;
        return std::make_unique<DeclBlockItem>(m_current, std::move(declaration));
    }
    std::unique_ptr<Stmt> statement = stmtParse();
    if (statement == nullptr)
        return nullptr;
    return std::make_unique<StmtBlockItem>(m_current, std::move(statement));
}

std::unique_ptr<Initializer> Parser::initializerParse()
{
    if (expect(TokenType::OpenBrace)) {
        std::vector<std::unique_ptr<Initializer>> initializers;
        std::unique_ptr<Initializer> first = initializerParse();
        if (first == nullptr)
            return nullptr;
        initializers.push_back(std::move(first));
        while (expect(TokenType::Comma)) {
            if (peekTokenType() == TokenType::CloseBrace)
                break;
            auto inner = initializerParse();
            if (inner == nullptr)
                return nullptr;
            initializers.push_back(std::move(inner));
        }
        if (!expect(TokenType::CloseBrace))
            return nullptr;
        return std::make_unique<CompoundInitializer>(std::move(initializers));
    }
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<SingleInitializer>(std::move(expr));
}

std::tuple<std::unique_ptr<ForInit>, bool> Parser::forInitParse()
{
    if (expect(TokenType::Semicolon))
        return {nullptr, false};
    if (Operators::isSpecifier(peekTokenType()))  {
        auto decl = declarationParse();
        if (decl == nullptr)
            return {nullptr, true};
        if (decl->kind == Declaration::Kind::FuncDecl) {
            addError("Unallowed function declaration in for loop declaration");
            return {nullptr, true};
        }
        const auto varDecl = dynCast<VarDecl>(decl.release());
        return {std::make_unique<DeclForInit>(m_current, std::unique_ptr<VarDecl>(varDecl)), false};
    }
    std::unique_ptr<Expr> expr = exprParse(0);
    if (!expect(TokenType::Semicolon)) {
        addError("Expected semicolon after for loop condition");
        return {nullptr, true};
    }
    return {std::make_unique<ExprForInit>(m_current, std::move(expr)), false};
}

std::unique_ptr<Stmt> Parser::stmtParse()
{
    switch (peekTokenType()) {
        case TokenType::Return:         return returnStmtParse();
        case TokenType::Semicolon:      return nullStmtParse();
        case TokenType::If:             return ifStmtParse();
        case TokenType::Goto:           return gotoStmtParse();
        case TokenType::OpenBrace:      return std::make_unique<CompoundStmt>(m_current, blockParse());
        case TokenType::Break:          return breakStmtParse();
        case TokenType::Continue:       return continueStmtParse();
        case TokenType::While:          return whileStmtParse();
        case TokenType::Do:             return doWhileStmtParse();
        case TokenType::For:            return forStmtParse();
        case TokenType::Switch:         return switchStmtParse();
        case TokenType::Case:           return caseStmtParse();
        case TokenType::Default:        return defaultStmtParse();
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
    if (expr == nullptr) {
        addError("Return without Expression", m_current - 1);
        return nullptr;
    }
    auto statement = std::make_unique<ReturnStmt>(m_current, std::move(expr));
    if (!expect(TokenType::Semicolon)) {
        addError("Return without semicolon", m_current - 1);
        return nullptr;
    }
    return statement;
}

std::unique_ptr<Stmt> Parser::exprStmtParse()
{
    const size_t before = m_current;
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr) {
        addError("Invalid expression in expression statement", before);
        return nullptr;
    }
    auto statement = std::make_unique<ExprStmt>(m_current, std::move(expr));
    if (!expect(TokenType::Semicolon)) {
        addError("Expression statement without semicolon");
        return nullptr;
    }
    return statement;
}

std::unique_ptr<Stmt> Parser::ifStmtParse()
{
    if (!expect(TokenType::If))
        return nullptr;
    if (!expect(TokenType::OpenParen)) {
        addError("Expected open paren before if condition");
        return nullptr;
    }
    std::unique_ptr<Expr> condition = exprParse(0);
    if (condition == nullptr)
        return nullptr;
    if (!expect(TokenType::CloseParen)) {
        addError("Expected close paren after if condition");
        return nullptr;
    }
    std::unique_ptr<Stmt> thenStmt = stmtParse();
    if (thenStmt == nullptr)
        return nullptr;
    if (expect(TokenType::Else)) {
        std::unique_ptr<Stmt> elseStmt = stmtParse();
        if (elseStmt == nullptr)
            return nullptr;
        return std::make_unique<IfStmt>(
            m_current, std::move(condition), std::move(thenStmt), std::move(elseStmt));
    }
    return std::make_unique<IfStmt>(m_current, std::move(condition), std::move(thenStmt));
}

std::unique_ptr<Stmt> Parser::gotoStmtParse()
{
    if (!expect(TokenType::Goto))
        return nullptr;
    Lexing::Token lexeme = peek();
    if (!expect(TokenType::Identifier)) {
        addError("Expected identifier after goto statement");
        return nullptr;
    }
    if (!expect(TokenType::Semicolon)) {
        addError("Expected semicolon after goto statement");
        return nullptr;
    }
    return std::make_unique<GotoStmt>(m_current, lexeme.m_lexeme);
}

std::unique_ptr<Stmt> Parser::breakStmtParse()
{
    if (!expect(TokenType::Break))
        return nullptr;
    if (!expect(TokenType::Semicolon)) {
        addError("Expected semicolon after break statement");
        return nullptr;
    }
    return std::make_unique<BreakStmt>(m_current);
}

std::unique_ptr<Stmt> Parser::continueStmtParse()
{
    if (!expect(TokenType::Continue))
        return nullptr;
    if (!expect(TokenType::Semicolon)) {
        addError("Expected semicolon after continue statement");
        return nullptr;
    }
    return std::make_unique<ContinueStmt>(m_current);
}

std::unique_ptr<Stmt> Parser::labelStmtParse()
{
    Lexing::Token lexeme = peek();
    if (!expect(TokenType::Identifier))
        return nullptr;
    if (!expect(TokenType::Colon)) {
        addError("Expected colon after label statement");
        return nullptr;
    }
    auto stmt = stmtParse();
    if (stmt == nullptr)
        return nullptr;
    return std::make_unique<LabelStmt>(m_current, lexeme.m_lexeme, std::move(stmt));
}

std::unique_ptr<Stmt> Parser::caseStmtParse()
{
    if (!expect(TokenType::Case))
        return nullptr;
    const size_t beforeCondition = m_current;
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr) {
        addError("Expected condition after case", beforeCondition);
        return nullptr;
    }
    if (!expect(TokenType::Colon))
        return nullptr;
    std::unique_ptr<Stmt> stmt = stmtParse();
    if (stmt == nullptr) {
        addError("Expected body in case statement");
        return nullptr;
    }
    return std::make_unique<CaseStmt>(m_current, std::move(expr), std::move(stmt));
}

std::unique_ptr<Stmt> Parser::defaultStmtParse()
{
    if (!expect(TokenType::Default))
        return nullptr;
    if (!expect(TokenType::Colon)) {
        addError("Expected colon after default statement");
        return nullptr;
    }
    std::unique_ptr<Stmt> stmt = stmtParse();
    if (stmt == nullptr) {
        addError("Expected body in default statement");
        return nullptr;
    }
    return std::make_unique<DefaultStmt>(m_current, std::move(stmt));
}

std::unique_ptr<Stmt> Parser::whileStmtParse()
{
    if (!expect(TokenType::While))
        return nullptr;
    if (!expect(TokenType::OpenParen)) {
        addError("Expected open paren before while condition");
        return nullptr;
    }
    std::unique_ptr<Expr> condition = exprParse(0);
    if (condition == nullptr) {
        addError("Expected condition in while loop");
        return nullptr;
    }
    if (!expect(TokenType::CloseParen)) {
        addError("Expected close paren after while condition");
        return nullptr;
    }
    std::unique_ptr<Stmt> body = stmtParse();
    if (body == nullptr) {
        addError("Expected body in while loop");
        return nullptr;
    }
    return std::make_unique<WhileStmt>(m_current, std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::doWhileStmtParse()
{
    if (!expect(TokenType::Do))
        return nullptr;
    std::unique_ptr<Stmt> body = stmtParse();
    if (body == nullptr)
        return nullptr;
    if (!expect(TokenType::While)) {
        addError("Expected while after do body");
        return nullptr;
    }
    if (!expect(TokenType::OpenParen))
        return nullptr;
    std::unique_ptr<Expr> condition = exprParse(0);
    if (condition == nullptr) {
        addError("Expected condition in do while loop");
        return nullptr;
    }
    if (!expect(TokenType::CloseParen)) {
        addError("Expected close parenthesis after do while loop condition");
        return nullptr;
    }
    if (!expect(TokenType::Semicolon)) {
        addError("Expected semicolon after do while");
        return nullptr;
    }
    return std::make_unique<DoWhileStmt>(m_current, std::move(body), std::move(condition));
}

std::unique_ptr<Stmt> Parser::forStmtParse()
{
    if (!expect(TokenType::For))
        return nullptr;
    if (!expect(TokenType::OpenParen)) {
        addError("Expected open parenthesis after for keyword");
        return nullptr;
    }
    auto [init, err] = forInitParse();
    if (err)
        return nullptr;
    std::unique_ptr<Expr> condition = exprParse(0);
    if (!expect(TokenType::Semicolon)) {
        addError("Expected semicolon after for condition");
        return nullptr;
    }
    std::unique_ptr<Expr> post = exprParse(0);
    if (!expect(TokenType::CloseParen)) {
        addError("Expected close parenthesis after for loop initialization");
        return nullptr;
    }
    std::unique_ptr<Stmt> body = stmtParse();
    if (body == nullptr) {
        addError("Expected body in for loop");
        return nullptr;
    }
    auto result = std::make_unique<ForStmt>(m_current, std::move(body));
    if (init != nullptr)
        result->init = std::move(init);
    if (condition != nullptr)
        result->condition = std::move(condition);
    if (post != nullptr)
        result->post = std::move(post);
    return result;
}

std::unique_ptr<Stmt> Parser::switchStmtParse()
{
    if (!expect(TokenType::Switch))
        return nullptr;
    if (!expect(TokenType::OpenParen)) {
        addError("Expected open parenthesis before switch condition");
        return nullptr;
    }
    std::unique_ptr<Expr> expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    if (!expect(TokenType::CloseParen)) {
        addError("Expected close parenthesis after switch condition");
        return nullptr;
    }
    std::unique_ptr<Stmt> body = stmtParse();
    if (body == nullptr) {
        addError("Expected body in switch condition");
        return nullptr;
    }
    return std::make_unique<SwitchStmt>(m_current, std::move(expr), std::move(body));
}

std::unique_ptr<Stmt> Parser::nullStmtParse()
{
    if (!expect(TokenType::Semicolon)) {
        addError("Expected semicolon in null statement");
        return nullptr;
    }
    return std::make_unique<NullStmt>(m_current);
}

std::unique_ptr<Expr> Parser::ternaryExprParse(std::unique_ptr<Expr>& condition)
{
    auto trueExpr = exprParse(0);
    if (trueExpr == nullptr) {
        addError("Expected true expression in ternary");
        return nullptr;
    }
    if (!expect(TokenType::Colon)) {
        addError("Expected colon in ternary");
        return nullptr;
    }
    auto falseExpr = exprParse(Operators::precedence(TokenType::Colon));
    if (falseExpr == nullptr) {
        addError("Expected false expression in ternary");
        return nullptr;
    }
    return std::make_unique<TernaryExpr>(
        m_current, std::move(condition), std::move(trueExpr), std::move(falseExpr));
}

std::unique_ptr<Expr> Parser::assignmentExprParse(std::unique_ptr<Expr>& left, const Lexing::Token& nextToken)
{
    AssignmentExpr::Operator op = Operators::assignOperator(nextToken.m_type);
    auto right = exprParse(Operators::precedence(nextToken.m_type));
    if (right == nullptr) {
        addError("Expected right hand side after assignment operator");
        return nullptr;
    }
    return std::make_unique<AssignmentExpr>(
        m_current, op, std::move(left), std::move(right));
}

std::unique_ptr<Expr> Parser::binaryExprParse(std::unique_ptr<Expr>& left, const Lexing::Token& nextToken)
{
    BinaryExpr::Operator op = Operators::binaryOperator(nextToken.m_type);
    auto right = exprParse(Operators::precedence(nextToken.m_type) + 1);
    if (right == nullptr) {
        addError("Expected right hand side after binary operator");
        return nullptr;
    }
    return std::make_unique<BinaryExpr>(m_current, op, std::move(left), std::move(right));
}

std::unique_ptr<Expr> Parser::exprParse(const i32 minPrecedence)
{
    auto left = castExprParse();
    if (left == nullptr)
        return nullptr;
    Lexing::Token nextToken = peek();
    while (continuePrecedenceClimbing(minPrecedence, peekTokenType())) {
        advance();
        if (nextToken.m_type == TokenType::QuestionMark)
            left = ternaryExprParse(left);
        if (Operators::isAssignmentOperator(nextToken.m_type))
            left = assignmentExprParse(left, nextToken);
        if (Operators::isBinaryOperator(nextToken.m_type))
            left = binaryExprParse(left, nextToken);
        if (left == nullptr)
            return nullptr;
        nextToken = peek();
    }
    return left;
}

std::unique_ptr<Expr> Parser::castExprParse()
{
    if (Operators::isType(peekNextTokenType()) && expect(TokenType::OpenParen)) {
        const Type type = typeParse();
        if (type == Type::Invalid)
            return nullptr;
        std::unique_ptr<AbstractDeclarator> abstractDeclarator = abstractDeclaratorParse();
        if (abstractDeclarator == nullptr)
            return nullptr;
        std::unique_ptr<TypeBase> typeBaseInner = std::make_unique<VarType>(type);
        std::unique_ptr<TypeBase> typeBase = abstractDeclaratorProcess(std::move(abstractDeclarator),
                                                                       std::move(typeBaseInner));
        if (!expect(TokenType::CloseParen))
            return nullptr;
        auto innerExpr = castExprParse();
        if (innerExpr == nullptr)
            return nullptr;
        return std::make_unique<CastExpr>(m_current, std::move(typeBase), std::move(innerExpr));
    }
    return unaryExprParse();
}

std::unique_ptr<Expr> Parser::unaryExprParse()
{
    if (!Operators::isUnaryOperator(peekTokenType()))
        return exprPostfixParse();
    if (peekTokenType() == TokenType::Ampersand)
        return addrOFExprParse();
    if (peekTokenType() == TokenType::Asterisk)
        return dereferenceExprParse();
    UnaryExpr::Operator oper = Operators::unaryOperator(peekTokenType());
    advance();
    std::unique_ptr<Expr> expr = castExprParse();
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<UnaryExpr>(m_current, oper, std::move(expr));
}

std::unique_ptr<Expr> Parser::addrOFExprParse()
{
    advance();
    std::unique_ptr<Expr> expr = unaryExprParse();
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<AddrOffExpr>(m_current, std::move(expr));
}

std::unique_ptr<Expr> Parser::dereferenceExprParse()
{
    advance();
    std::unique_ptr<Expr> expr = unaryExprParse();
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<DereferenceExpr>(m_current, std::move(expr));
}

std::unique_ptr<Expr> Parser::exprPostfixParse()
{
    auto expr = factorParse();
    while (true) {
        if (expect(TokenType::Increment))
            expr = std::make_unique<UnaryExpr>(
                m_current, UnaryExpr::Operator::PostFixIncrement, std::move(expr));
        else if (expect(TokenType::Decrement))
            expr = std::make_unique<UnaryExpr>(
                m_current, UnaryExpr::Operator::PostFixDecrement, std::move(expr));
        else if (peekTokenType() == TokenType::OpenSqBracket)
            expr = std::move(subscriptExprParse(std::move(expr)));
        else
            break;
    }
    return expr;
}

std::unique_ptr<Expr> Parser::subscriptExprParse(std::unique_ptr<Expr>&& expr)
{
    while (expect(TokenType::OpenSqBracket)) {
        auto index = exprParse(0);
        if (!expect(TokenType::CloseSqBracket))
            return nullptr;
        expr = std::make_unique<SubscriptExpr>(m_current, std::move(expr), std::move(index));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factorParse()
{
    if (Operators::isLiteral(peekTokenType()))
        return constExprParse();
    switch (const Lexing::Token lexeme = peek(); lexeme.m_type) {
        case TokenType::Identifier: {
            advance();
            if (!expect(TokenType::OpenParen))
                return std::make_unique<VarExpr>(m_current, lexeme.m_lexeme);
            const std::unique_ptr<std::vector<std::unique_ptr<Expr>>> arguments = argumentListParse();
            if (arguments == nullptr)
                return nullptr;
            if (!expect(TokenType::CloseParen))
                return nullptr;
            return std::make_unique<FuncCallExpr>(m_current, lexeme.m_lexeme, std::move(*arguments));
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

std::unique_ptr<Expr> Parser::constExprParse()
{
    switch (const Lexing::Token lexeme = peek(); lexeme.m_type) {
        case TokenType::IntegerLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                m_current, lexeme.getI32Value(), std::make_unique<VarType>(Type::I32));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::LongLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                m_current, lexeme.getI64Value(), std::make_unique<VarType>(Type::I64));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::UnsignedLongLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                m_current, lexeme.getU64Value(), std::make_unique<VarType>(Type::U64));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::UnsignedIntegerLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                m_current, lexeme.getU32Value(), std::make_unique<VarType>(Type::U32));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::DoubleLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                m_current, lexeme.getDoubleValue(), std::make_unique<VarType>(Type::Double));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        default:
            return nullptr;
    }
    std::unreachable();
}

std::unique_ptr<AbstractDeclarator> Parser::abstractDeclaratorParse()
{
    if (peekTokenType() == TokenType::OpenParen || peekTokenType() == TokenType::OpenSqBracket)
        return directAbstractDeclaratorParse();
    auto base = std::make_unique<AbstractBase>();
    if (!expect(TokenType::Asterisk))
        return base;
    std::unique_ptr<AbstractDeclarator> inner = abstractDeclaratorParse();
    if (inner != nullptr)
        return std::make_unique<AbstractPointer>(std::move(inner));
    return std::make_unique<AbstractPointer>(std::move(base));
}

std::unique_ptr<AbstractDeclarator> Parser::directAbstractDeclaratorParse()
{
    if (!expect(TokenType::OpenParen))
        return nullptr;
    std::unique_ptr<AbstractDeclarator> abstractDeclarator = abstractDeclaratorParse();
    if (abstractDeclarator == nullptr)
        return nullptr;
    if (!expect(TokenType::CloseParen))
        return nullptr;
    while (expect(TokenType::OpenSqBracket)) {
        auto expr = constExprParse();
        if (!expect(TokenType::CloseSqBracket))
            return nullptr;
        const auto constExpr = dynCast<ConstExpr>(expr.get());
        const i64 size = constExpr->getValue<i64>();
        abstractDeclarator = std::make_unique<AbstractArrayDeclarator>(std::move(abstractDeclarator), size);
    }
    return abstractDeclarator;
}

std::unique_ptr<TypeBase> Parser::abstractDeclaratorProcess(
    std::unique_ptr<AbstractDeclarator>&& abstractDeclarator, std::unique_ptr<TypeBase>&& type)
{
    switch (abstractDeclarator->kind) {
        case AbstractArrayDeclarator::Kind::Pointer: {
            type = std::make_unique<PointerType>(std::move(type));
            const auto inner = dynCast<AbstractPointer>(abstractDeclarator.get());
            return abstractDeclaratorProcess(std::move(inner->inner), std::move(type));
        }
        case AbstractArrayDeclarator::Kind::Array: {
            const auto arrayDeclarator = dynCast<AbstractArrayDeclarator>(abstractDeclarator.get());
            type = std::make_unique<ArrayType>(std::move(type), arrayDeclarator->size);
            return abstractDeclaratorProcess(std::move(arrayDeclarator->abstractDeclarator), std::move(type));
        }
        case AbstractArrayDeclarator::Kind::Base:
            return type;
    }
    return type;
}

std::unique_ptr<std::vector<std::unique_ptr<Expr>>> Parser::argumentListParse()
{
    std::vector<std::unique_ptr<Expr>> arguments;
    if (peekTokenType() == TokenType::CloseParen)
        return std::make_unique<std::vector<std::unique_ptr<Expr>>>(std::move(arguments));
    auto expr = exprParse(0);
    if (expr == nullptr)
        return nullptr;
    arguments.push_back(std::move(expr));
    while (expect(TokenType::Comma)) {
        expr = exprParse(0);
        if (expr == nullptr)
            return nullptr;
        arguments.push_back(std::move(expr));
    }
    return std::make_unique<std::vector<std::unique_ptr<Expr>>>(std::move(arguments));
}

std::tuple<Type, Lexing::Token::Type> Parser::specifierParse()
{
    std::vector<TokenType> declarations;
    std::vector<TokenType> types;
    TokenType type = peekTokenType();
    while (Operators::isSpecifier(type)) {
        if (Operators::isType(type))
            types.push_back(type);
        else if (Operators::isStorageSpecifier(type))
            declarations.push_back(type);
        else {
            addError("Could not parse specifier");
            return {Type::Invalid, TokenType::NotAToken};
        }
        advance();
        type = peekTokenType();
    }
    const Type varType = typeResolve(types);
    if (varType == Type::Invalid) {
        addError("Could not resolve type");
        return {Type::Invalid, TokenType::NotAToken};
    }
    if (1 < declarations.size()) {
        addError("Too many declarators type");
        return {Type::Invalid, TokenType::NotAToken};
    }
    auto storage = TokenType::NotAToken;
    if (!declarations.empty())
        storage = declarations.front();
    return std::make_tuple(varType, storage);
}

Type Parser::typeParse()
{
    TokenType type = peekTokenType();
    std::vector<TokenType> types;
    while (Operators::isType(type)) {
        types.push_back(type);
        advance();
        type = peekTokenType();
    }
    return typeResolve(types);
}

Type Parser::typeResolve(std::vector<TokenType>& tokens)
{
    if (tokens.size() == 1 && tokens.front() == TokenType::DoubleKeyword)
        return Type::Double;
    if (std::ranges::find(tokens, TokenType::DoubleKeyword) != tokens.end())
        return Type::Invalid;
    if (tokens.empty())
        return Type::Invalid;
    if (containsSameTwice(tokens))
        return Type::Invalid;
    if (std::ranges::find(tokens, TokenType::Unsigned) != tokens.end() &&
        std::ranges::find(tokens, TokenType::Signed) != tokens.end())
        return Type::Invalid;
    if (std::ranges::find(tokens, TokenType::Unsigned) != tokens.end() &&
        std::ranges::find(tokens, TokenType::LongKeyword) != tokens.end())
        return Type::U64;
    if (std::ranges::find(tokens, TokenType::Unsigned) != tokens.end())
        return Type::U32;
    if (std::ranges::find(tokens, TokenType::LongKeyword) != tokens.end())
        return Type::I64;
    return Type::I32;
}

bool containsSameTwice(std::vector<Lexing::Token::Type>& tokens)
{
    std::ranges::sort(tokens);
    for (i64 i = 1; i < tokens.size(); ++i)
        if (tokens[i] == tokens[i - 1])
            return true;
    return false;
}

bool Parser::match(const TokenType &type)
{
    if (type == peekTokenType()) {
        if (advance().m_type == TokenType::EndOfFile)
            return false;
        return true;
    }
    return false;
}

bool Parser::expect(const TokenType type)
{
    if (peekTokenType() == type) {
        if (advance().m_type == TokenType::EndOfFile)
            return false;
        return true;
    }
    return false;
}

void Parser::addError(std::string message)
{
    m_errors.emplace_back(std::move(message), m_current);
}

void Parser::addError(std::string message, const size_t index)
{
    m_errors.emplace_back(std::move(message), index);
}

Lexing::Token::Type Parser::peekTokenType() const
{
    return c_tokenStore.getType(m_current);
}

Lexing::Token::Type Parser::peekNextTokenType() const
{
    if (c_tokenStore.size() <= m_current + 1)
        return TokenType::EndOfFile;
    return c_tokenStore.getType(m_current + 1);
}

Lexing::Token::Type Parser::peekNextNextTokenType() const
{
    if (c_tokenStore.size() <= m_current + 2)
        return TokenType::EndOfFile;
    return c_tokenStore.getType(m_current + 2);
}
} // Parsing