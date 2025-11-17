#include "TypeResolution.hpp"
#include "ASTIr.hpp"
#include "TypeConversion.hpp"
#include "Utils.hpp"
#include "ASTDeepCopy.hpp"
#include "AstToIrOperators.hpp"

namespace {
using BinaryOp = Parsing::BinaryExpr::Operator;
}

namespace Semantics {
std::vector<Error> TypeResolution::validate(Parsing::Program& program)
{
    m_errors = std::vector<Error>();
    ASTTraverser::visit(program);
    return std::move(m_errors);
}

void TypeResolution::visit(Parsing::FuncDeclaration& funDecl)
{
    const auto it = m_functions.find(funDecl.name);
    if (it != m_functions.end() && !validFuncDecl(it->second, funDecl)) {
        addError("Function declaration does not exist", funDecl.location);
        return;
    }
    const auto funcType = dynCast<Parsing::FuncType>(funDecl.type.get());
    std::vector<std::unique_ptr<Parsing::TypeBase>> paramsTypes;
    if (funcType->returnType->type == Type::Array) {
        addError("Function cannot return array", funDecl.location);
        return;
    }

    if (funDecl.body != nullptr)
        m_definedFunctions.insert(funDecl.name);
    const auto type = dynCast<Parsing::FuncType>(funDecl.type.get());
    FuncEntry funcEntry(type->params, type->returnType->type, funDecl.storage,
        funDecl.body != nullptr);
    m_functions.emplace_hint(it, funDecl.name, std::move(funcEntry));

    if (funDecl.body) {
        m_global = false;

        funDecl.body->accept(*this);

        m_global = true;
    }
}

bool TypeResolution::validFuncDecl(const FuncEntry& funcEntry, const Parsing::FuncDeclaration& funDecl)
{
    if ((funcEntry.storage == Storage::Extern || funcEntry.storage == Storage::None) &&
        funDecl.storage == Storage::Static) {
        addError("Incompatible storage types", funDecl.location);
        return false;
    }
    if (funDecl.params.size() != funcEntry.paramTypes.size()) {
        addError("Unequal parameter numbers in function declarations", funDecl.location);
        return false;
    }
    const auto funcType = dynCast<Parsing::FuncType>(funDecl.type.get());
    for (size_t i = 0; i < funDecl.params.size(); ++i) {
        const auto paramTypeEntry = funcEntry.paramTypes[i].get();
        const auto paramTypeFunc = funcType->params[i].get();
        if (!Parsing::areEquivalentArrayConversion(*paramTypeEntry, *paramTypeFunc)) {
            addError("Incompatible parameter types in function declarations", funDecl.location);
            return false;
        }
    }
    if (funcType->returnType->type != funcEntry.returnType) {
        addError("Incompatible return types in function declarations", funDecl.location);
        return false;
    }
    return true;
}

void TypeResolution::visit(Parsing::VarDecl& varDecl)
{
    if (isIllegalVarDecl(varDecl))
        return;
    if (m_global && varDecl.storage == Storage::Static)
        m_globalStaticVars.insert(varDecl.name);
    if (!m_global && !m_globalStaticVars.contains(varDecl.name))
        m_definedFunctions.insert(varDecl.name);
    m_isConst = true;

    if (varDecl.init) {
        if (varDecl.type->type == Type::Array)
            m_inArrayInit = true;
        varDecl.init->accept(*this);
        m_inArrayInit = false;
    }

    if (hasError())
        return;
    if (illegalNonConstInitialization(varDecl, m_isConst, m_global)) {
        addError("Is illegal non const variable initilization", varDecl.location);
        return;
    }
    if (varDecl.init == nullptr)
        return;
    if (varDecl.init->kind == Parsing::Initializer::Kind::Single)
        handleSingleInit(varDecl);
    else {
        handelCompoundInit(varDecl);
        return;
    }
    assignTypeToArithmeticUnaryExpr(varDecl);
}

void TypeResolution::verifyArrayInSingleInit(
    const Parsing::VarDecl& varDecl, const Parsing::SingleInitializer& singleInitializer)
{
    const auto arrayType = dynCast<Parsing::ArrayType>(varDecl.type.get());
    if (!isCharacterType(arrayType->elementType->type)) {
        addError("Cannot initialize array with single init", varDecl.location);
        return;
    }
    if (singleInitializer.expr->kind == Parsing::Expr::Kind::String) {
        const auto stringExpr = dynCast<Parsing::StringExpr>(singleInitializer.expr.get());
        if (arrayType->size < stringExpr->value.size())
            addError("Character array shorter than string init", varDecl.location);
    }
}

void TypeResolution::handleSingleInit(Parsing::VarDecl& varDecl)
{
    const auto singleInit = dynCast<Parsing::SingleInitializer>(varDecl.init.get());

    if (varDecl.type->type == Type::Array) {
        verifyArrayInSingleInit(varDecl, *singleInit);
        return;
    }

    if (hasError())
        return;

    if (!Parsing::areEquivalentTypes(*varDecl.type, *singleInit->expr->type)) {
        if (varDecl.type->type == Type::Pointer) {
            if (!canConvertToNullPtr(*singleInit->expr) && !isVoidPointer(*singleInit->expr->type)) {
                addError("Cannot convert pointer init to pointer", varDecl.location);
                return;
            }
            auto typeExpr = std::make_unique<Parsing::VarType>(Type::U64);
            varDecl.init = std::make_unique<Parsing::SingleInitializer>(
                std::make_unique<Parsing::ConstExpr>(0ul, std::move(typeExpr)));
        }
    }
}

void TypeResolution::handelCompoundInit(const Parsing::VarDecl& varDecl)
{
    const auto compoundInit = dynCast<Parsing::CompoundInitializer>(varDecl.init.get());
    if (varDecl.type->type != Type::Array) {
        addError("Cannot have compound initializer on non array", varDecl.location);
        return;
    }
    const auto arrayType = dynCast<Parsing::ArrayType>(varDecl.type.get());
    if (arrayType->size < compoundInit->initializers.size())
        addError("Compound initializer cannot be longer than array size", varDecl.location);

    const Type arrayTypeInner = Ir::getArrayType(varDecl.type.get());

    for (const auto& initializer : compoundInit->initializers) {
        if (initializer->kind == Parsing::Initializer::Kind::Single) {
            const auto singleInit = dynCast<Parsing::SingleInitializer>(initializer.get());
            if (arrayType->elementType->type == Type::Pointer && singleInit->expr->type->type == Type::Double) {
                addError("Cannot init ptr with double", varDecl.location);
                break;
            }
            if (singleInit->expr->kind == Parsing::Expr::Kind::String)
                continue;
            if (singleInit->expr->type->type != arrayTypeInner) {
                singleInit->expr = std::make_unique<Parsing::CastExpr>(
                    singleInit->expr->location,
                    Parsing::deepCopy(*arrayType->elementType),
                    std::move(singleInit->expr));
            }
        }
    }
}

void TypeResolution::assignTypeToArithmeticUnaryExpr(Parsing::VarDecl& varDecl)
{
    if (varDecl.init->kind != Parsing::Initializer::Kind::Single)
        return;

    const auto singleInit = dynCast<Parsing::SingleInitializer>(varDecl.init.get());
    if (varDecl.type->type == singleInit->expr->type->type)
        return;
    if (!isArithmetic(varDecl.type->type))
        return;
    auto newExpr = convertOrCastToType(*singleInit->expr, varDecl.type->type);
    varDecl.init = std::make_unique<Parsing::SingleInitializer>(std::move(newExpr));
}

void TypeResolution::visit(Parsing::SingleInitializer& singleInitializer)
{
    singleInitializer.expr = convertArrayType(*singleInitializer.expr);
}

void TypeResolution::visit(Parsing::CompoundInitializer& compoundInitializer)
{
    for (const auto& initializer : compoundInitializer.initializers)
        initializer->accept(*this);
}

void TypeResolution::visit(Parsing::DeclForInit& declForInit)
{
    if (hasStorageClassSpecifier(declForInit))
        addError("Declaration in for cannot have storage specifier", declForInit.location);
    ASTTraverser::visit(declForInit);
}

void TypeResolution::visit(Parsing::ExprForInit& exprForInit)
{
    exprForInit.expression = convertArrayType(*exprForInit.expression);
}

void TypeResolution::visit(Parsing::ReturnStmt& stmt)
{
    if (stmt.expr)
        stmt.expr = convertArrayType(*stmt.expr);
}

void TypeResolution::visit(Parsing::ExprStmt& stmt)
{
    stmt.expr = convertArrayType(*stmt.expr);
}

void TypeResolution::visit(Parsing::IfStmt& ifStmt)
{
    ifStmt.condition = convertArrayType(*ifStmt.condition);
    if (ifStmt.condition) {
        if (ifStmt.condition->type && !isScalarType(*ifStmt.condition->type))
            addError("If condition must have scalar type", ifStmt.location);
    }
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt)
        ifStmt.elseStmt->accept(*this);
}

void TypeResolution::visit(Parsing::CaseStmt& caseStmt)
{
    caseStmt.condition = convertArrayType(*caseStmt.condition);
    if (caseStmt.condition) {
        if (caseStmt.condition->type && !isScalarType(*caseStmt.condition->type))
            addError("Case condition must have scalar type", caseStmt.location);
    }
    caseStmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::WhileStmt& whileStmt)
{
    whileStmt.condition = convertArrayType(*whileStmt.condition);
    if (whileStmt.condition) {
        if (whileStmt.condition->type && !isScalarType(*whileStmt.condition->type))
            addError("While condition must have scalar type", whileStmt.location);
    }
    whileStmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::DoWhileStmt& doWhileStmt)
{
    doWhileStmt.condition = convertArrayType(*doWhileStmt.condition);
    if (doWhileStmt.condition) {
        if (doWhileStmt.condition->type && !isScalarType(*doWhileStmt.condition->type))
            addError("Do While condition must have scalar type", doWhileStmt.location);
    }
    doWhileStmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::ForStmt& forStmt)
{
    if (forStmt.init)
        forStmt.init->accept(*this);
    if (forStmt.condition) {
        forStmt.condition = convertArrayType(*forStmt.condition);
        if (forStmt.condition->type && !isScalarType(*forStmt.condition->type))
            addError("For loop condition must have scalar type", forStmt.location);
    }
    if (forStmt.post)
        forStmt.post = convertArrayType(*forStmt.post);
    forStmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::SwitchStmt& switchStmt)
{
    switchStmt.condition = convertArrayType(*switchStmt.condition);
    if (switchStmt.condition) {
        if (switchStmt.condition->type && !isScalarType(*switchStmt.condition->type))
            addError("Switch condition must have scalar type", switchStmt.location);
    }
    switchStmt.body->accept(*this);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertArrayType(Parsing::Expr& expr)
{
    auto genExpr = convert(expr);
    if (genExpr->type && genExpr->type->type == Type::Array && genExpr->kind != Parsing::Expr::Kind::AddrOf) {
        if (genExpr->kind != Parsing::Expr::Kind::AddrOf) {
            const auto arrayType = dynCast<Parsing::ArrayType>(genExpr->type.get());
            auto addressOf = std::make_unique<Parsing::AddrOffExpr>(genExpr->location, Parsing::deepCopy(*genExpr));
            addressOf->type = std::make_unique<Parsing::PointerType>(Parsing::deepCopy(*arrayType->elementType));
            return addressOf;
        }
    }
    return genExpr;
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::Expr& expr)
{
    using ExprKind = Parsing::Expr::Kind;
    switch (expr.kind) {
        case ExprKind::Constant: {
            const auto constExpr = dynCast<Parsing::ConstExpr>(&expr);
            return convertConstExpr(*constExpr);
        }
        case ExprKind::String: {
            const auto constExpr = dynCast<Parsing::StringExpr>(&expr);
            return convertStringExpr(*constExpr);
        }
        case ExprKind::Var: {
            const auto varExpr = dynCast<Parsing::VarExpr>(&expr);
            return convertVarExpr(*varExpr);
        }
        case ExprKind::Cast: {
            const auto cast = dynCast<Parsing::CastExpr>(&expr);
            return convertCastExpr(*cast);
        }
        case ExprKind::Unary: {
            const auto unary = dynCast<Parsing::UnaryExpr>(&expr);
            return convertUnaryExpr(*unary);
        }
        case ExprKind::Binary: {
            const auto binary = dynCast<Parsing::BinaryExpr>(&expr);
            return convertBinaryExpr(*binary);
        }
        case ExprKind::Assignment: {
            const auto assignment = dynCast<Parsing::AssignmentExpr>(&expr);
            return convertAssignExpr(*assignment);
        }
        case ExprKind::Ternary: {
            const auto ternary = dynCast<Parsing::TernaryExpr>(&expr);
            return convertTernaryExpr(*ternary);
        }
        case ExprKind::FunctionCall: {
            const auto functionCall = dynCast<Parsing::FuncCallExpr>(&expr);
            return convertFuncCallExpr(*functionCall);
        }
        case ExprKind::Dereference: {
            const auto deref = dynCast<Parsing::DereferenceExpr>(&expr);
            return convertDerefExpr(*deref);
        }
        case ExprKind::AddrOf: {
            const auto addrOf = dynCast<Parsing::AddrOffExpr>(&expr);
            return convertAddrOfExpr(*addrOf);
        }
        case ExprKind::Subscript: {
            const auto subscript = dynCast<Parsing::SubscriptExpr>(&expr);
            return convertSubscriptExpr(*subscript);
        }
        case ExprKind::SizeOfExpr: {
            const auto sizeOfExpr = dynCast<Parsing::SizeOfExprExpr>(&expr);
            return convertSizeOfExprExpr(*sizeOfExpr);
        }
        case ExprKind::SizeOfType: {
            const auto sizeOfType = dynCast<Parsing::SizeOfTypeExpr>(&expr);
            return convertSizeOfExprType(*sizeOfType);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertConstExpr(const Parsing::ConstExpr& expr)
{
    return deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertStringExpr(Parsing::StringExpr& expr)
{
    if (m_inArrayInit)
        return deepCopy(expr);
    expr.type = std::make_unique<Parsing::ArrayType>(
        std::make_unique<Parsing::VarType>(Type::Char), expr.value.size() + 1);
    return deepCopy(expr);
}

void TypeResolution::validateAndConvertFuncCallArgs(
    Parsing::FuncCallExpr& funCallExpr,
    const std::unordered_map<std::string, FuncEntry>::iterator& it)
{
    for (size_t i = 0; i < funCallExpr.args.size(); ++i) {
        const Parsing::Expr* const callExpr = funCallExpr.args[i].get();
        const Parsing::TypeBase* const argTypeBase = it->second.paramTypes[i].get();
        const Type typeInner = funCallExpr.args[i]->type->type;
        const Type castToType = it->second.paramTypes[i]->type;
        if (typeInner == Type::Pointer && castToType == Type::Pointer) {
            if (!Parsing::areEquivalentTypes(*callExpr->type, *argTypeBase)
                && !isVoidPointer(*argTypeBase))
                addError("Function arg of of different type with param", callExpr->location);
        }
        if (castToType != Type::Pointer && typeInner == Type::Pointer)
            addError("Cannot cast pointer arg to non pointer", callExpr->location);
        if (typeInner != castToType) {
            funCallExpr.args[i] = std::make_unique<Parsing::CastExpr>(
                Parsing::deepCopy(*argTypeBase), std::move(funCallExpr.args[i]));
        }
    }
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertFuncCallExpr(Parsing::FuncCallExpr& funCallExpr)
{
    const auto it = m_functions.find(funCallExpr.name);
    if (it == m_functions.end()) {
        addError("Called function is not declared", funCallExpr.location);
        return Parsing::deepCopy(funCallExpr);
    }
    if (it->second.paramTypes.size() != funCallExpr.args.size()) {
        addError("Called function is not declared", funCallExpr.location);
        return Parsing::deepCopy(funCallExpr);
    }

    std::vector<std::unique_ptr<Parsing::Expr>> args;
    for (const auto& arg : funCallExpr.args)
        args.push_back(convertArrayType(*arg));
    funCallExpr.args = std::move(args);

    if (hasError())
        return Parsing::deepCopy(funCallExpr);
    validateAndConvertFuncCallArgs(funCallExpr, it);
    return Parsing::deepCopy(funCallExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertVarExpr(Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    ASTTraverser::visit(varExpr);
    return Parsing::deepCopy(varExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertUnaryExpr(Parsing::UnaryExpr& unaryExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;

    unaryExpr.innerExpr = convertArrayType(*unaryExpr.innerExpr);

    if (hasError())
        return Parsing::deepCopy(unaryExpr);

    if (unaryExpr.innerExpr->type->type == Type::Void) {
        addError("Cannot apply unary operation to void", unaryExpr.innerExpr->location);
        return Parsing::deepCopy(unaryExpr);
    }
    if (isVoidPointer(*unaryExpr.innerExpr->type)) {
        addError("Cannot apply unary operation to void pointer", unaryExpr.innerExpr->location);
        return Parsing::deepCopy(unaryExpr);
    }
    if (unaryExpr.innerExpr->type->type == Type::Array || unaryExpr.innerExpr->kind == Parsing::Expr::Kind::AddrOf) {
        if (unaryExpr.op == Operator::PrefixDecrement)
            addError("Cannot apply prefix decrement", unaryExpr.location);
        if (unaryExpr.op == Operator::PrefixIncrement)
            addError("Cannot apply prefix increment", unaryExpr.location);
        if (unaryExpr.op == Operator::PostFixDecrement)
            addError("Cannot apply postfix decrement", unaryExpr.location);
        if (unaryExpr.op == Operator::PostFixIncrement)
            addError("Cannot apply postfix increment", unaryExpr.location);
    }

    if (unaryExpr.innerExpr->type->type == Type::Double) {
        if (unaryExpr.op == Operator::Complement) {
            addError("Cannot complement double", unaryExpr.location);
            return Parsing::deepCopy(unaryExpr);
        }
    }
    if (unaryExpr.innerExpr->type->type == Type::Pointer) {
        if (isIllegalUnaryPointerOperator(unaryExpr.op)) {
            addError("Cannot apply operator to pointer", unaryExpr.location);
            return Parsing::deepCopy(unaryExpr);
        }
    }
    if (unaryExpr.op == Operator::Not)
        unaryExpr.type = std::make_unique<Parsing::VarType>(s_boolType);
    else
        unaryExpr.type = Parsing::deepCopy(*unaryExpr.innerExpr->type);

    if (isCharacterType(unaryExpr.innerExpr->type->type) &&
        (unaryExpr.op == Operator::Negate || unaryExpr.op == Operator::Complement)) {
        unaryExpr.innerExpr = convertOrCastToType(*unaryExpr.innerExpr, Type::I32);
        unaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
    }

    return Parsing::deepCopy(unaryExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertBinaryExpr(Parsing::BinaryExpr& binaryExpr)
{
    binaryExpr.lhs = convertArrayType(*binaryExpr.lhs);
    binaryExpr.rhs = convertArrayType(*binaryExpr.rhs);

    if (hasError())
        return Parsing::deepCopy(binaryExpr);
    const Type leftType = binaryExpr.lhs->type->type;
    const Type rightType = binaryExpr.rhs->type->type;
    if (leftType == Type::Void || rightType == Type::Void) {
        addError("Cannot have void type in binary expression.", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }
    if (binaryExpr.op == BinaryOp::And || binaryExpr.op == BinaryOp::Or) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return Parsing::deepCopy(binaryExpr);
    }
    const Type commonType = getCommonType(leftType, rightType);

    if (commonType == Type::Double && (isBinaryBitwise(binaryExpr.op) || binaryExpr.op == BinaryOp::Modulo)) {
        addError("Cannot apply operator to double", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }
    if (leftType == Type::Pointer || rightType == Type::Pointer)
        return handleBinaryPtr(binaryExpr, leftType, rightType, commonType);
    assignTypeToArithmeticBinaryExpr(binaryExpr);
    return Parsing::deepCopy(binaryExpr);
}

bool areValidNonArithmeticTypesInBinaryExpr(const Parsing::BinaryExpr& binaryExpr,
        const Type leftType, const Type rightType, const Type commonType)
{
    if (Parsing::areEquivalentTypes(*binaryExpr.lhs->type, *binaryExpr.rhs->type))
        return true;
    if (canConvertToNullPtr(*binaryExpr.lhs) || canConvertToNullPtr(*binaryExpr.rhs))
        return true;
    if (commonType == Type::Pointer && (isIntegerType(leftType) || isIntegerType(rightType)) &&
            (binaryExpr.op == BinaryOp::Add || binaryExpr.op == BinaryOp::Subtract))
        return true;
    return false;
}

std::unique_ptr<Parsing::Expr> TypeResolution::handleAddSubtractPtrToIntegerTypes(
    Parsing::BinaryExpr& binaryExpr, const Type leftType, const Type rightType)
{
    if (isVoidPointer(*binaryExpr.lhs->type) || isVoidPointer(*binaryExpr.rhs->type)) {
        addError("Cannot add or subtract void pointer", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }
    if (isIntegerType(rightType)) {
        if (rightType != Type::I64)
            binaryExpr.rhs = convertOrCastToType(*binaryExpr.rhs, Type::I64);
        binaryExpr.type = Parsing::deepCopy(*binaryExpr.lhs->type);
    }
    else if (isIntegerType(leftType)) {
        if (binaryExpr.op == BinaryOp::Subtract) {
            addError("Cannot subtract a pointer form an integer type", binaryExpr.location);
            return Parsing::deepCopy(binaryExpr);
        }
        if (leftType != Type::I64)
            binaryExpr.lhs = convertOrCastToType(*binaryExpr.lhs, Type::I64);
        binaryExpr.type = Parsing::deepCopy(*binaryExpr.rhs->type);
        auto returnExpr = std::make_unique<Parsing::BinaryExpr>(binaryExpr.location,
            binaryExpr.op, std::move(binaryExpr.rhs), std::move(binaryExpr.lhs));
        if (binaryExpr.type)
            returnExpr->type =  std::move(binaryExpr.type);
        return returnExpr;
    }
    return Parsing::deepCopy(binaryExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::handlePtrToPtrBinaryOpers(Parsing::BinaryExpr& binaryExpr)
{
    if (Parsing::areEquivalentTypes(*binaryExpr.lhs->type, *binaryExpr.rhs->type)) {
        if (isBinaryComparison(binaryExpr.op)) {
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
            return Parsing::deepCopy(binaryExpr);
        }
        if (isVoidPointer(*binaryExpr.lhs->type) || isVoidPointer(*binaryExpr.rhs->type)) {
            addError("Cannot apply subtraction to void pointers", binaryExpr.location);
            return Parsing::deepCopy(binaryExpr);
        }
        if (binaryExpr.op == BinaryOp::Subtract) {
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I64);
            return Parsing::deepCopy(binaryExpr);
        }
    }
    addError("Cannot apply operator to pointers", binaryExpr.location);
    return Parsing::deepCopy(binaryExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::handleBinaryPtr(Parsing::BinaryExpr& binaryExpr,
                                                               const Type leftType,
                                                               const Type rightType,
                                                               const Type commonType)
{
    if (isUnallowedPtrBinaryOperation(binaryExpr.op)) {
        addError("Cannot apply operator to pointer", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }

    if (leftType == Type::Pointer && rightType == Type::Pointer)
        return handlePtrToPtrBinaryOpers(binaryExpr);

    if (leftType == Type::Double || rightType == Type::Double) {
        addError("Cannot have binary on double and pointer", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }

    if (isUnallowedComparisonBetweenPtrAndInteger(binaryExpr.op)) {
        addError("Unallowed comparison operator between pointer and integer types", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }

    if (binaryExpr.op == BinaryOp::Add || binaryExpr.op == BinaryOp::Subtract)
        return handleAddSubtractPtrToIntegerTypes(binaryExpr, leftType, rightType);

    if (!areValidNonArithmeticTypesInBinaryExpr(binaryExpr, leftType, rightType, commonType)) {
        addError("Cannot apply operator to non-arithmetic types", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }

    if (leftType != Type::Pointer) {
        binaryExpr.lhs = std::make_unique<Parsing::CastExpr>(binaryExpr.lhs->location,
            std::move(Parsing::deepCopy(*binaryExpr.rhs->type)), std::move(binaryExpr.lhs));
    }
    if (rightType != Type::Pointer) {
        binaryExpr.rhs = std::make_unique<Parsing::CastExpr>(binaryExpr.rhs->location,
            std::move(Parsing::deepCopy(*binaryExpr.lhs->type)), std::move(binaryExpr.rhs));
    }

    if (binaryExpr.op == BinaryOp::Equal || binaryExpr.op == BinaryOp::NotEqual)
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
    return Parsing::deepCopy(binaryExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertAssignExpr(Parsing::AssignmentExpr& assignmentExpr)
{
    using Oper = Parsing::AssignmentExpr::Operator;

    assignmentExpr.lhs = convertArrayType(*assignmentExpr.lhs);
    assignmentExpr.rhs = convertArrayType(*assignmentExpr.rhs);

    if (hasError())
        return Parsing::deepCopy(assignmentExpr);
    const Type leftType = assignmentExpr.lhs->type->type;
    const Type rightType = assignmentExpr.rhs->type->type;
    if ((assignmentExpr.op == Oper::PlusAssign || assignmentExpr.op == Oper::MinusAssign) &&
        rightType == Type::Pointer) {
        addError("Cannot have pointer as right hand side of compound assign", assignmentExpr.rhs->location);
        return Parsing::deepCopy(assignmentExpr);
    }
    if (assignmentExpr.op != Oper::Assign && isVoidPointer(*assignmentExpr.lhs->type)) {
        addError("Cannot compound assign to void ptr", assignmentExpr.lhs->location);
        return Parsing::deepCopy(assignmentExpr);
    }
    if (leftType == Type::Array) {
        addError("Cannot assign to array", assignmentExpr.location);
        return Parsing::deepCopy(assignmentExpr);
    }
    if (!isLegalAssignExpr(assignmentExpr))
        return Parsing::deepCopy(assignmentExpr);
    if (leftType != rightType && assignmentExpr.op == Parsing::AssignmentExpr::Operator::Assign &&
        leftType != Type::Pointer) {
        if (!isCharacterType(leftType)) {
            assignmentExpr.rhs = convertOrCastToType(*assignmentExpr.rhs, leftType);
        }
        else {
            assignmentExpr.rhs = std::make_unique<Parsing::CastExpr>(
                std::make_unique<Parsing::VarType>(leftType), std::move(assignmentExpr.rhs));
        }
    }
    if (leftType == Type::Pointer && isIntegerType(rightType))
        assignmentExpr.rhs = convertOrCastToType(*assignmentExpr.rhs, Type::I64);
    assignmentExpr.type = Parsing::deepCopy(*assignmentExpr.lhs->type);
    return Parsing::deepCopy(assignmentExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertCastExpr(Parsing::CastExpr& castExpr)
{
    castExpr.innerExpr = convertArrayType(*castExpr.innerExpr);

    if (hasError())
        return Parsing::deepCopy(castExpr);
    const Type outerType = castExpr.type->type;
    const Type innerType = castExpr.innerExpr->type->type;
    if (innerType == Type::Void)
        addError("Cannot from void pointer", castExpr.location);
    if (isPointerToVoidArray(*castExpr.type))
        addError("Cannot cast to pointer to void array", castExpr.location);
    if (isArrayOfVoidPointer(*castExpr.type))
        addError("Cannot cast to void pointer array", castExpr.location);
    if (outerType == Type::Double && innerType == Type::Pointer)
        addError("Cannot convert pointer to double", castExpr.location);
    if (outerType == Type::Pointer && innerType == Type::Double)
        addError("Cannot convert double to pointer", castExpr.location);
    if (isArithmetic(outerType) && isArithmetic(innerType))
        return convertOrCastToType(*castExpr.innerExpr, outerType);
    if (outerType == Type::Pointer && innerType == Type::Pointer) {
        auto innerTypeCopy = Parsing::deepCopy(*castExpr.type);
        castExpr.innerExpr->type = std::move(innerTypeCopy);
        return Parsing::deepCopy(*castExpr.innerExpr);
    }
    return Parsing::deepCopy(castExpr);
}

bool TypeResolution::isLegalAssignExpr(Parsing::AssignmentExpr& assignmentExpr)
{
    using Oper = Parsing::AssignmentExpr::Operator;

    const Type leftType = assignmentExpr.lhs->type->type;
    const Type rightType = assignmentExpr.rhs->type->type;
    if (rightType == Type::Void) {
        addError("Cannot assign with type void.", assignmentExpr.rhs->location);
        return false;
    }
    const Type commonType = getCommonType(leftType, rightType);
    if ((leftType == Type::Pointer || rightType == Type::Pointer) &&
        isIllegalPointerCompoundAssignOperation(assignmentExpr.op)) {
        addError("Is illegal pointer assign operator", assignmentExpr.location);
        return false;
    }
    if (commonType == Type::Double && isIllegalDoubleCompoundAssignOperation(assignmentExpr.op)) {
        addError("Is double compound assign operator", assignmentExpr.location);
        return false;
    }
    if (leftType == Type::Pointer && rightType == Type::Pointer) {
        if (!Parsing::areEquivalentTypes(*assignmentExpr.lhs->type, *assignmentExpr.rhs->type)
            && !isVoidPointer(*assignmentExpr.rhs->type)) {
            addError("Cannot assign pointer of different types", assignmentExpr.location);
            return false;
        }
        return true;
    }
    if (leftType == Type::Pointer) {
        if (canConvertToNullPtr(*assignmentExpr.rhs)) {
            assignmentExpr.rhs = std::make_unique<Parsing::CastExpr>(
                Parsing::deepCopy(*assignmentExpr.lhs->type), std::move(assignmentExpr.rhs));
        }
        else if (assignmentExpr.lhs->kind == Parsing::Expr::Kind::Subscript)
            return true;
        else if ((assignmentExpr.op == Oper::PlusAssign || assignmentExpr.op == Oper::MinusAssign) &&
                 isIntegerType(rightType)) {
            return true;
        }
        else {
            addError("Cannot convert one type to pointer", assignmentExpr.location);
            return false;
        }
    }
    return true;
}

std::unique_ptr<Parsing::Expr> TypeResolution::validateAndConvertPtrsInTernaryExpr(
    Parsing::TernaryExpr& ternaryExpr, const Type trueType, const Type falseType)
{
    if (trueType == Type::Pointer && falseType == Type::Pointer) {
        if (isVoidPointer(*ternaryExpr.trueExpr->type) || isVoidPointer(*ternaryExpr.falseExpr->type)) {
            ternaryExpr.type = std::make_unique<Parsing::PointerType>(
                std::make_unique<Parsing::VarType>(Type::Void));
            return Parsing::deepCopy(ternaryExpr);
        }
        if (!Parsing::areEquivalentTypes(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
            addError("Ternary true and false expression must have same type", ternaryExpr.location);
        ternaryExpr.type = std::move(Parsing::deepCopy(*ternaryExpr.trueExpr->type));
        return Parsing::deepCopy(ternaryExpr);
    }
    if (!areValidNonArithmeticTypesInTernaryExpr(ternaryExpr)) {
        addError("Are not valid non arithmetic types in ternary", ternaryExpr.location);
        return Parsing::deepCopy(ternaryExpr);
    }
    if (trueType != Type::Pointer) {
        ternaryExpr.trueExpr = std::make_unique<Parsing::CastExpr>(
            std::move(Parsing::deepCopy(*ternaryExpr.falseExpr->type)), std::move(ternaryExpr.trueExpr));
    }
    if (falseType != Type::Pointer) {
        ternaryExpr.falseExpr = std::make_unique<Parsing::CastExpr>(
            std::move(Parsing::deepCopy(*ternaryExpr.trueExpr->type)), std::move(ternaryExpr.falseExpr));
    }
    ternaryExpr.type = std::move(Parsing::deepCopy(*ternaryExpr.trueExpr->type));
    return Parsing::deepCopy(ternaryExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertTernaryExpr(Parsing::TernaryExpr& ternaryExpr)
{
    ternaryExpr.condition = convertArrayType(*ternaryExpr.condition);
    ternaryExpr.trueExpr = convertArrayType(*ternaryExpr.trueExpr);
    ternaryExpr.falseExpr = convertArrayType(*ternaryExpr.falseExpr);

    if (!isScalarType(*ternaryExpr.condition->type))
        addError("Ternary condition must have scalar type", ternaryExpr.condition->location);

    const Type trueType = ternaryExpr.trueExpr->type->type;
    const Type falseType = ternaryExpr.falseExpr->type->type;
    if (trueType == Type::Void && falseType == Type::Void) {
        ternaryExpr.type = Parsing::deepCopy(*ternaryExpr.trueExpr->type);
        return Parsing::deepCopy(ternaryExpr);
    }
    if (trueType == Type::Void || falseType == Type::Void) {
        addError("Ternary true and false expression must not have type void", ternaryExpr.location);
        return Parsing::deepCopy(ternaryExpr);
    }
    const Type commonType = getCommonType(trueType, falseType);
    if (trueType == Type::Pointer || falseType == Type::Pointer)
        return validateAndConvertPtrsInTernaryExpr(ternaryExpr, trueType, falseType);
    if (commonType != trueType)
        ternaryExpr.trueExpr = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(ternaryExpr.trueExpr));
    if (commonType != falseType)
        ternaryExpr.falseExpr = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(ternaryExpr.falseExpr));
    ternaryExpr.type = std::make_unique<Parsing::VarType>(commonType);
    return Parsing::deepCopy(ternaryExpr);
}

bool areValidNonArithmeticTypesInTernaryExpr(const Parsing::TernaryExpr& ternaryExpr)
{
    if (Parsing::areEquivalentTypes(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
        return true;
    if (canConvertToNullPtr(*ternaryExpr.trueExpr) || canConvertToNullPtr(*ternaryExpr.falseExpr))
        return true;
    return false;
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertAddrOfExpr(Parsing::AddrOffExpr& addrOffExpr)
{
    addrOffExpr.reference = convert(*addrOffExpr.reference);

    if (hasError())
        return Parsing::deepCopy(addrOffExpr);
    if (addrOffExpr.reference->kind == Parsing::Expr::Kind::AddrOf) {
        addError("Cannot have address-of of address-of operation", addrOffExpr.location);
        return Parsing::deepCopy(addrOffExpr);
    }
    addrOffExpr.type = std::make_unique<Parsing::PointerType>(
        Parsing::deepCopy(*addrOffExpr.reference->type));
    return Parsing::deepCopy(addrOffExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertDerefExpr(Parsing::DereferenceExpr& dereferenceExpr)
{
    dereferenceExpr.reference = convertArrayType(*dereferenceExpr.reference);

    if (hasError())
        return Parsing::deepCopy(dereferenceExpr);
    if (dereferenceExpr.reference->type->type != Type::Pointer) {
        addError("Cannot dereference non pointer", dereferenceExpr.location);
        return Parsing::deepCopy(dereferenceExpr);
    }
    const auto referencedPtrType = dynCast<const Parsing::PointerType>(dereferenceExpr.reference->type.get());
    dereferenceExpr.type = Parsing::deepCopy(*referencedPtrType->referenced);
    return Parsing::deepCopy(dereferenceExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertSubscriptExpr(Parsing::SubscriptExpr& subscriptExpr)
{
    subscriptExpr.referencing = convertArrayType(*subscriptExpr.referencing);
    subscriptExpr.index = convertArrayType(*subscriptExpr.index);

    if (hasError())
        return Parsing::deepCopy(subscriptExpr);

    if (isVoidPointer(*subscriptExpr.referencing->type)) {
        addError("Cannot subscript void pointer", subscriptExpr.referencing->location);
        return Parsing::deepCopy(subscriptExpr);
    }

    std::unique_ptr<Parsing::Expr> result = nullptr;

    const Type referencingType = subscriptExpr.referencing->type->type;
    const Type indexType = subscriptExpr.index->type->type;

    if (referencingType == Type::Pointer && isIntegerType(indexType))
        result = std::move(Parsing::deepCopy(subscriptExpr));
    else if (indexType == Type::Pointer && isIntegerType(referencingType)) {
        result = std::make_unique<Parsing::SubscriptExpr>(subscriptExpr.location,
            std::move(subscriptExpr.index), std::move(subscriptExpr.referencing));
    }
    else {
        addError("Cannot convert subscript to non pointer", subscriptExpr.location);
        return Parsing::deepCopy(subscriptExpr);
    }
    const auto subscriptExprPtr = dynCast<Parsing::SubscriptExpr>(result.get());
    if (subscriptExprPtr->index->type->type != Type::I64)
        subscriptExprPtr->index = convertOrCastToType(*subscriptExprPtr->index, Type::I64);
    const auto ptrType = dynCast<Parsing::PointerType>(subscriptExprPtr->referencing->type.get());
    result->type = Parsing::deepCopy(*ptrType->referenced);
    return result;
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertSizeOfExprExpr(Parsing::SizeOfExprExpr& sizeOfExprExpr)
{
    sizeOfExprExpr.innerExpr = convertArrayType(*sizeOfExprExpr.innerExpr);
    if (sizeOfExprExpr.innerExpr && sizeOfExprExpr.innerExpr->type) {
        if (isVoidArray(*sizeOfExprExpr.innerExpr->type)) {
            addError("Cannot call sizeof on void array expression", sizeOfExprExpr.location);
            return Parsing::deepCopy(sizeOfExprExpr);
        }
        if (sizeOfExprExpr.innerExpr->type->type == Type::Void) {
            addError("Cannot call sizeof on void expression", sizeOfExprExpr.location);
            return Parsing::deepCopy(sizeOfExprExpr);
        }
    }
    return Parsing::deepCopy(sizeOfExprExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertSizeOfExprType(const Parsing::SizeOfTypeExpr& sizeOfTypeExpr)
{
    if (isVoidPointer(*sizeOfTypeExpr.sizeType)) {
        addError("Cannot call sizeof on void pointer type", sizeOfTypeExpr.location);
        return Parsing::deepCopy(sizeOfTypeExpr);
    }
    if (isVoidArray(*sizeOfTypeExpr.sizeType)) {
        addError("Cannot call sizeof on void array type", sizeOfTypeExpr.location);
        return Parsing::deepCopy(sizeOfTypeExpr);
    }
    if (sizeOfTypeExpr.sizeType->type == Type::Void) {
        addError("Cannot call sizeof on void type", sizeOfTypeExpr.location);
        return Parsing::deepCopy(sizeOfTypeExpr);
    }
    return Parsing::deepCopy(sizeOfTypeExpr);
}

bool TypeResolution::isIllegalVarDecl(const Parsing::VarDecl& varDecl)
{
    if (varDecl.storage == Storage::Extern && varDecl.init != nullptr) {
        addError("Initiated extern variable", varDecl.location);
        return true;
    }
    if (varDecl.storage == Storage::Static && m_definedFunctions.contains(varDecl.name)) {
        addError("Static variable with same name as defined function", varDecl.location);
        return true;
    }
    return false;
}
} // namespace Semantics