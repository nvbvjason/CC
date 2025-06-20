#include "Parser.hpp"
#include "ASTTypes.hpp"

#include <algorithm>
#include <cassert>

namespace Parsing {

bool Parser::programParse(Program& program)
{
    while (!isAtEnd()) {
        std::unique_ptr<Declaration> declaration = declarationParse();
        if (declaration == nullptr)
            return false;
        program.declarations.push_back(std::move(declaration));
    }
    return true;
}

std::unique_ptr<Declaration> Parser::declarationParse()
{
    auto [type, storageClass] = specifierParse();
    if (type == Type::Invalid)
        return nullptr;
    const Declaration::StorageClass storage = getStorageClass(storageClass);
    const std::unique_ptr<Declarator> declarator = declaratorParse();
    if (declarator == nullptr)
        return nullptr;
    if (declarator->kind == Declarator::Kind::Function) {
        auto functionDeclarator = static_cast<FunctionDeclarator*>(declarator.get());
        return funDeclParse(type, storage, *functionDeclarator);
    }
    return varDeclParse(type, storage, *declarator);
}

std::unique_ptr<VarDecl> Parser::varDeclParse(const Type type,
                                              const Storage storage,
                                              Declarator& declarator)
{
    if (declarator.kind == Declarator::Kind::Identifier) {
        auto idenDeclarator = static_cast<IdentifierDeclarator*>(&declarator);
        auto typeExpr = std::make_unique<VarType>(type);
        return std::make_unique<VarDecl>(storage, idenDeclarator->identifier, std::move(typeExpr));
    }

}

std::unique_ptr<FunDecl> Parser::funDeclParse(const Type type,
                                              const Storage storage,
                                              FunctionDeclarator& functionDeclarator)
{
    if (!expect(TokenType::OpenParen))
        return nullptr;
    const std::unique_ptr<std::vector<ParamInfo>> paramList = paramsListParse();
    if (paramList == nullptr)
        return nullptr;
    if (!expect(TokenType::CloseParen))
        return nullptr;
    std::unique_ptr<Block> block = nullptr;
    if (peekTokenType() == TokenType::OpenBrace) {
        m_atFileScope = false;
        block = blockParse();
        m_atFileScope = true;
        if (block == nullptr)
            return nullptr;
    }
    else if (!expect(TokenType::Semicolon))
        return nullptr;
    auto result = std::make_unique<FunDecl>(storage, iden);
    if (block != nullptr)
        result->body = std::move(block);
    result->params = std::move(paramList->params);
    result->type = std::make_unique<FuncType>(
        std::make_unique<VarType>(type),
        std::move(paramList->types));
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
    if (peekTokenType() != TokenType::OpenParen)
        return result;
    const std::unique_ptr<std::vector<ParamInfo>> paramList = paramsListParse();
    if (paramList == nullptr)
        return nullptr;
    return std::make_unique<FunctionDeclarator>(std::move(result), std::move(*paramList));
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
    if (peekTokenType() != TokenType::Identifier)
        return nullptr;
    std::string iden = peek().m_lexeme;
    advance();
    return std::make_unique<IdentifierDeclarator>(std::move(iden));
}

std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>> Parser::delaratorProcess(
    std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase)
{
    switch (declarator->kind) {
        case Declarator::Kind::Identifier:
            auto iden = static_cast<IdentifierDeclarator*>(declarator.get());
            return {iden->identifier, {}, {}};

    }
}

std::unique_ptr<std::vector<ParamInfo>> Parser::paramsListParse()
{
    if (!expect(TokenType::OpenParen))
        return nullptr;
    std::vector<ParamInfo> params;
    if (expect(TokenType::Void)) {
        if (!expect(TokenType::CloseParen))
            return nullptr;
        return std::make_unique<std::vector<ParamInfo>>(params);
    }
    while (!expect(TokenType::Comma)) {
        std::unique_ptr<ParamInfo> param;
        if (param == nullptr)
            return nullptr;
        params.push_back(*param);
    }
    if (!expect(TokenType::CloseParen))
        return nullptr;
    return std::make_unique<std::vector<ParamInfo>>(params);
}

std::unique_ptr<ParamInfo> Parser::paramParse()
{
    const Type type = typeParse();
    if (type == Type::Invalid)
        return nullptr;
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
        return std::make_unique<DeclBlockItem>(std::move(declaration));
    }
    std::unique_ptr<Stmt> statement = stmtParse();
    if (statement == nullptr)
        return nullptr;
    return std::make_unique<StmtBlockItem>(std::move(statement));
}

std::tuple<std::unique_ptr<ForInit>, bool> Parser::forInitParse()
{
    if (expect(TokenType::Semicolon))
        return {nullptr, false};
    if (Operators::isSpecifier(peekTokenType()))  {
        auto decl = declarationParse();
        if (decl == nullptr)
            return {nullptr, true};
        if (decl->kind == Declaration::Kind::FuncDecl)
            return {nullptr, true};
        const auto varDecl = static_cast<VarDecl*>(decl.release());
        return {std::make_unique<DeclForInit>(std::unique_ptr<VarDecl>(varDecl)), false};
    }
    std::unique_ptr<Expr> expr = exprParse(0);
    if (!expect(TokenType::Semicolon))
        return {nullptr, true};
    return {std::make_unique<ExprForInit>(std::move(expr)), false};
}

std::unique_ptr<Stmt> Parser::stmtParse()
{
    switch (peek().m_type) {
        case TokenType::Return:         return returnStmtParse();
        case TokenType::Semicolon:      return nullStmtParse();
        case TokenType::If:             return ifStmtParse();
        case TokenType::Goto:           return gotoStmtParse();
        case TokenType::OpenBrace:      return std::make_unique<CompoundStmt>(blockParse());
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
    std::unique_ptr<Expr> condition = exprParse(0);
    if (condition == nullptr)
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
        return std::make_unique<IfStmt>(std::move(condition), std::move(thenStmt), std::move(elseStmt));
    }
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenStmt));
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
    return std::make_unique<BreakStmt>();
}

std::unique_ptr<Stmt> Parser::continueStmtParse()
{
    if (!expect(TokenType::Continue))
        return nullptr;
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<ContinueStmt>();
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
    return std::make_unique<CaseStmt>(std::move(expr), std::move(stmt));
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
    return std::make_unique<DefaultStmt>(std::move(stmt));
}

std::unique_ptr<Stmt> Parser::whileStmtParse()
{
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
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::doWhileStmtParse()
{
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
    return std::make_unique<DoWhileStmt>(std::move(body), std::move(condition));
}

std::unique_ptr<Stmt> Parser::forStmtParse()
{
    if (!expect(TokenType::For))
        return nullptr;
    if (!expect(TokenType::OpenParen))
        return nullptr;
    auto [init, err] = forInitParse();
    if (err)
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
    auto result = std::make_unique<ForStmt>(std::move(body));
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
    return std::make_unique<SwitchStmt>(std::move(expr), std::move(body));
}

std::unique_ptr<Stmt> Parser::nullStmtParse()
{
    if (!expect(TokenType::Semicolon))
        return nullptr;
    return std::make_unique<NullStmt>();
}

std::unique_ptr<Expr> Parser::exprParse(const i32 minPrecedence)
{
    auto left = castExpr();
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
            left = std::make_unique<TernaryExpr>(
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

std::unique_ptr<Expr> Parser::castExpr()
{
    if (peek().m_type == TokenType::OpenParen && Operators::isType(peekNextTokenType())) {
        advance();
        if (!Operators::isType(peek().m_type))
            return nullptr;
        const Type type = typeParse();
        if (type == Type::Invalid)
            return nullptr;
        if (!expect(TokenType::CloseParen))
            return nullptr;
        auto innerExpr = castExpr();
        if (innerExpr == nullptr)
            return nullptr;
        return std::make_unique<CastExpr>(
            std::make_unique<VarType>(type), std::move(innerExpr));
    }
    return unaryExprParse();
}

std::unique_ptr<Expr> Parser::unaryExprParse()
{
    if (!Operators::isUnaryOperator(peek().m_type))
        return exprPostfix();
    if (peek().m_type == TokenType::Ampersand)
        return addrOFExprParse();
    if (peek().m_type == TokenType::Asterisk)
        return dereferenceExprParse();
    UnaryExpr::Operator oper = Operators::unaryOperator(peek().m_type);
    advance();
    std::unique_ptr<Expr> expr = unaryExprParse();
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<UnaryExpr>(oper, std::move(expr));
}

std::unique_ptr<Expr> Parser::addrOFExprParse()
{
    advance();
    std::unique_ptr<Expr> expr = unaryExprParse();
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<AddrOffExpr>(std::move(expr));
}

std::unique_ptr<Expr> Parser::dereferenceExprParse()
{
    advance();
    std::unique_ptr<Expr> expr = unaryExprParse();
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<DereferenceExpr>(std::move(expr));
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
        case TokenType::IntegerLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                lexeme.getI32Value(), std::make_unique<VarType>(Type::I32));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::LongLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                lexeme.getI64Value(), std::make_unique<VarType>(Type::I64));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::UnsignedLongLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                lexeme.getU64Value(), std::make_unique<VarType>(Type::U64));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::UnsignedIntegerLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                lexeme.getU32Value(), std::make_unique<VarType>(Type::U32));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::DoubleLiteral: {
            auto constantExpr = std::make_unique<ConstExpr>(
                lexeme.getDoubleValue(), std::make_unique<VarType>(Type::Double));
            if (advance().m_type == TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case TokenType::Identifier: {
            advance();
            if (!expect(TokenType::OpenParen))
                return std::make_unique<VarExpr>(lexeme.m_lexeme);
            const std::unique_ptr<std::vector<std::unique_ptr<Expr>>> arguments = argumentListParse();
            if (arguments == nullptr)
                return nullptr;
            if (!expect(TokenType::CloseParen))
                return nullptr;
            return std::make_unique<FunCallExpr>(lexeme.m_lexeme, std::move(*arguments));
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

std::unique_ptr<std::vector<std::unique_ptr<Expr>>> Parser::argumentListParse()
{
    std::vector<std::unique_ptr<Expr>> arguments;
    if (peek().m_type == TokenType::CloseParen)
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
        else
            return {Type::Invalid, TokenType::NotAToken};
        advance();
        type = peekTokenType();
    }
    const Type varType = typeResolve(types);
    if (varType == Type::Invalid)
        return {Type::Invalid, TokenType::NotAToken};
    if (1 < declarations.size())
        return {Type::Invalid, TokenType::NotAToken};
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

Declaration::StorageClass getStorageClass(const Lexing::Token::Type tokenType)
{
    using TokenType = Lexing::Token::Type;
    using StorageClass = Declaration::StorageClass;
    switch (tokenType) {
        case TokenType::Static:     return StorageClass::Static;
        case TokenType::Extern:     return StorageClass::Extern;
        case TokenType::NotAToken:  return StorageClass::None;
        default:
            assert("getVarStorageClass invalid TokenType");
            std::abort();
    }
    assert("getVarStorageClass invalid TokenType");
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

Lexing::Token::Type Parser::peekTokenType() const
{
    return c_tokens[m_current].m_type;
}

Lexing::Token::Type Parser::peekNextTokenType() const
{
    if (c_tokens.size() <= m_current + 1)
        return TokenType::EndOfFile;
    return c_tokens[m_current + 1].m_type;
}

Lexing::Token::Type Parser::peekNextNextTokenType() const
{
    if (c_tokens.size() <= m_current + 2)
        return TokenType::EndOfFile;
    return c_tokens[m_current + 2].m_type;
}
} // Parsing