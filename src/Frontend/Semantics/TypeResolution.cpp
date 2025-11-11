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
    auto funcType = dynCast<Parsing::FuncType>(funDecl.type.get());
    std::vector<std::unique_ptr<Parsing::TypeBase>> paramsTypes;
    for (auto & param : funcType->params) {
        if (param->type == Type::Array) {
            const auto arrayType = dynCast<Parsing::ArrayType>(param.get());
            auto pointerType = std::make_unique<Parsing::PointerType>(
                Parsing::deepCopy(*arrayType->elementType));
            param = std::move(pointerType);
        }
    }
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

void TypeResolution::handelCompoundInit(Parsing::VarDecl& varDecl)
{
    auto compoundInit = dynCast<Parsing::CompoundInitializer>(varDecl.init.get());
    if (varDecl.type->type != Type::Array) {
        addError("Cannot have compound initializer on non array", varDecl.location);
        return;
    }
    auto arrayType = dynCast<Parsing::ArrayType>(varDecl.type.get());
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
            if (singleInit->expr->type->type != arrayTypeInner) {
                singleInit->expr = std::make_unique<Parsing::CastExpr>(singleInit->expr->location,
                                                                       Parsing::deepCopy(*arrayType->elementType), std::move(singleInit->expr));
            }
        }
    }
    return;
}

void TypeResolution::handleSingleInit(Parsing::VarDecl& varDecl)
{
    auto singleInit = dynCast<Parsing::SingleInitializer>(varDecl.init.get());

    if (varDecl.type->type == Type::Array) {
        addError("Cannot initialize array with single init", varDecl.location);
        return;
    }

    if (hasError())
        return;

    if (!Parsing::areEquivalent(*varDecl.type, *singleInit->expr->type)) {
        if (varDecl.type->type == Type::Pointer) {
            if (!canConvertToNullPtr(*singleInit->expr)) {
                addError("Cannot convert pointer init to pointer", varDecl.location);
                return;
            }
            auto typeExpr = std::make_unique<Parsing::VarType>(Type::U64);
            varDecl.init = std::make_unique<Parsing::SingleInitializer>(
                std::make_unique<Parsing::ConstExpr>(0ul, std::move(typeExpr))
            );
        }
    }
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

    if (varDecl.init)
        varDecl.init->accept(*this);

    if (hasError())
        return;
    if (illegalNonConstInitialization(varDecl, m_isConst, m_global)) {
        addError("Is illegal non const variable initilization", varDecl.location);
        return;
    }
    if (varDecl.init == nullptr)
        return;
    if (varDecl.init->kind == Parsing::Initializer::Kind::Single) {
        handleSingleInit(varDecl);
    }
    else {
        handelCompoundInit(varDecl);
        return;
    }
    assignTypeToArithmeticUnaryExpr(varDecl);
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
    if (singleInit->expr->kind == Parsing::Expr::Kind::Constant) {
        const auto constExpr = dynCast<const Parsing::ConstExpr>(singleInit->expr.get());
        auto newExpr = convertOrCastToType(*constExpr, varDecl.type->type);
        varDecl.init = std::make_unique<Parsing::SingleInitializer>(std::move(newExpr));
        return;
    }
    varDecl.init = std::make_unique<Parsing::SingleInitializer>(
        std::make_unique<Parsing::CastExpr>(
        std::make_unique<Parsing::VarType>(varDecl.type->type),
        std::move(singleInit->expr)));
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
    stmt.expr = convertArrayType(*stmt.expr);
}

void TypeResolution::visit(Parsing::ExprStmt& stmt)
{
    stmt.expr = convertArrayType(*stmt.expr);
}

void TypeResolution::visit(Parsing::IfStmt& ifStmt)
{
    ifStmt.condition = convertArrayType(*ifStmt.condition);
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt)
        ifStmt.elseStmt->accept(*this);
}

void TypeResolution::visit(Parsing::CaseStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::WhileStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::DoWhileStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::ForStmt& stmt)
{
    if (stmt.init)
        stmt.init->accept(*this);
    if (stmt.condition)
        stmt.condition = convertArrayType(*stmt.condition);
    if (stmt.post)
        stmt.post = convertArrayType(*stmt.post);
    stmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::SwitchStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertArrayType(Parsing::Expr& expr)
{
    auto genExpr = convert(expr);
    if (genExpr->type && genExpr->type->type == Type::Array && genExpr->kind != Parsing::Expr::Kind::AddrOf) {
        if (genExpr->kind != Parsing::Expr::Kind::AddrOf) {
            auto arrayType = dynCast<Parsing::ArrayType>(genExpr->type.get());
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
            return convert(*constExpr);
        }
        case ExprKind::Var: {
            const auto varExpr = dynCast<Parsing::VarExpr>(&expr);
            return convert(*varExpr);
        }
        case ExprKind::Cast: {
            const auto cast = dynCast<Parsing::CastExpr>(&expr);
            return convert(*cast);
        }
        case ExprKind::Unary: {
            const auto unary = dynCast<Parsing::UnaryExpr>(&expr);
            return convert(*unary);
        }
        case ExprKind::Binary: {
            const auto binary = dynCast<Parsing::BinaryExpr>(&expr);
            return convert(*binary);
        }
        case ExprKind::Assignment: {
            const auto assignment = dynCast<Parsing::AssignmentExpr>(&expr);
            return convert(*assignment);
        }
        case ExprKind::Ternary: {
            const auto ternary = dynCast<Parsing::TernaryExpr>(&expr);
            return convert(*ternary);
        }
        case ExprKind::FunctionCall: {
            const auto functionCall = dynCast<Parsing::FuncCallExpr>(&expr);
            return convert(*functionCall);
        }
        case ExprKind::Dereference: {
            const auto deref = dynCast<Parsing::DereferenceExpr>(&expr);
            return convert(*deref);
        }
        case ExprKind::AddrOf: {
            const auto addrOf = dynCast<Parsing::AddrOffExpr>(&expr);
            return convert(*addrOf);
        }
        case ExprKind::Subscript: {
            const auto subscript = dynCast<Parsing::SubscriptExpr>(&expr);
            return convert(*subscript);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(const Parsing::ConstExpr& expr)
{
    return deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::FuncCallExpr& funCallExpr)
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
    for (size_t i = 0; i < funCallExpr.args.size(); ++i) {
        const Type typeInner = funCallExpr.args[i]->type->type;
        const Type castTo = it->second.paramTypes[i]->type;
        if (typeInner == Type::Pointer && castTo == Type::Pointer) {
            if (!Parsing::areEquivalent(*funCallExpr.args[i]->type, *it->second.paramTypes[i])) {
                addError("Function arg of of different type with param", funCallExpr.args[i]->location);
                return Parsing::deepCopy(funCallExpr);
            }
        }
        if (castTo != Type::Pointer && typeInner == Type::Pointer) {
            addError("Cannot cast pointer arg to non pointer", funCallExpr.args[i]->location);
            return Parsing::deepCopy(funCallExpr);
        }
        if (typeInner != castTo)
            funCallExpr.args[i] = std::make_unique<Parsing::CastExpr>(
                Parsing::deepCopy(*it->second.paramTypes[i]), std::move(funCallExpr.args[i]));
    }
    return Parsing::deepCopy(funCallExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    ASTTraverser::visit(varExpr);
    return Parsing::deepCopy(varExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::UnaryExpr& unaryExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;

    unaryExpr.operand = convertArrayType(*unaryExpr.operand);

    if (hasError())
        return Parsing::deepCopy(unaryExpr);

    if (unaryExpr.operand->type->type == Type::Array || unaryExpr.operand->kind == Parsing::Expr::Kind::AddrOf) {
        if (unaryExpr.op == Operator::PrefixDecrement)
            addError("Cannot apply prefix decrement", unaryExpr.location);
        if (unaryExpr.op == Operator::PrefixIncrement)
            addError("Cannot apply prefix increment", unaryExpr.location);
        if (unaryExpr.op == Operator::PostFixDecrement)
            addError("Cannot apply postfix decrement", unaryExpr.location);
        if (unaryExpr.op == Operator::PostFixIncrement)
            addError("Cannot apply postfix increment", unaryExpr.location);
    }

    if (unaryExpr.operand->type->type == Type::Double) {
        if (unaryExpr.op == Operator::Complement) {
            addError("Cannot complement double", unaryExpr.location);
            return Parsing::deepCopy(unaryExpr);
        }
    }
    if (unaryExpr.operand->type->type == Type::Pointer) {
        if (isIllegalUnaryPointerOperator(unaryExpr.op)) {
            addError("Cannot apply operator to pointer", unaryExpr.location);
            return Parsing::deepCopy(unaryExpr);
        }
    }
    if (unaryExpr.op == Operator::Not)
        unaryExpr.type = std::make_unique<Parsing::VarType>(s_boolType);
    else
        unaryExpr.type = Parsing::deepCopy(*unaryExpr.operand->type);

    return Parsing::deepCopy(unaryExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::BinaryExpr& binaryExpr)
{
    binaryExpr.lhs = convertArrayType(*binaryExpr.lhs);
    binaryExpr.rhs = convertArrayType(*binaryExpr.rhs);

    if (hasError())
        return Parsing::deepCopy(binaryExpr);
    if (binaryExpr.op == BinaryOp::And || binaryExpr.op == BinaryOp::Or) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return Parsing::deepCopy(binaryExpr);
    }
    const Type leftType = binaryExpr.lhs->type->type;
    const Type rightType = binaryExpr.rhs->type->type;
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
    if (Parsing::areEquivalent(*binaryExpr.lhs->type, *binaryExpr.rhs->type))
        return true;
    if (canConvertToNullPtr(*binaryExpr.lhs) || canConvertToNullPtr(*binaryExpr.rhs))
        return true;
    if (commonType == Type::Pointer && (isIntegerType(leftType) || isIntegerType(rightType)) &&
            (binaryExpr.op == BinaryOp::Add || binaryExpr.op == BinaryOp::Subtract))
        return true;
    return false;
}

std::unique_ptr<Parsing::Expr> TypeResolution::handleBinaryPtr(Parsing::BinaryExpr& binaryExpr,
                                                               const Type leftType,
                                                               const Type rightType,
                                                               const Type commonType)
{
    if (binaryExpr.op == BinaryOp::Modulo || binaryExpr.op == BinaryOp::Multiply ||
        binaryExpr.op == BinaryOp::Divide ||
        binaryExpr.op == BinaryOp::BitwiseOr || binaryExpr.op == BinaryOp::BitwiseXor) {
        addError("Cannot apply operator to pointer", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }

    if (leftType == Type::Pointer && rightType == Type::Pointer) {
        if (Parsing::areEquivalent(*binaryExpr.lhs->type, *binaryExpr.rhs->type)) {
            if (isBinaryComparison(binaryExpr.op)) {
                binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
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

    if (leftType == Type::Double || rightType == Type::Double) {
        addError("Cannot have binary on double and pointer", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }

    if ((binaryExpr.op == BinaryOp::GreaterThan || binaryExpr.op == BinaryOp::GreaterOrEqual ||
         binaryExpr.op == BinaryOp::LessThan    || binaryExpr.op == BinaryOp::LessOrEqual) &&
         (isIntegerType(leftType) || isIntegerType(rightType))) {
        addError("Cannot apply operator to pointer", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }

    if (binaryExpr.op == BinaryOp::Add || binaryExpr.op == BinaryOp::Subtract) {
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
        }
        return Parsing::deepCopy(binaryExpr);
    }

    if (!areValidNonArithmeticTypesInBinaryExpr(binaryExpr, leftType, rightType, commonType)) {
        addError("Cannot apply operator to non-arithmetic types", binaryExpr.location);
        return Parsing::deepCopy(binaryExpr);
    }

    if (leftType != Type::Pointer || rightType != Type::Pointer) {
        if (leftType != Type::Pointer) {
            binaryExpr.lhs = std::make_unique<Parsing::CastExpr>(binaryExpr.lhs->location,
                                                                 std::move(Parsing::deepCopy(*binaryExpr.rhs->type)), std::move(binaryExpr.lhs));
        }
        if (rightType != Type::Pointer) {
            binaryExpr.rhs = std::make_unique<Parsing::CastExpr>(binaryExpr.rhs->location,
                                                                 std::move(Parsing::deepCopy(*binaryExpr.lhs->type)), std::move(binaryExpr.rhs));
        }
    }

    if (binaryExpr.op == BinaryOp::Equal || binaryExpr.op == BinaryOp::NotEqual) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return Parsing::deepCopy(binaryExpr);
    }
    return Parsing::deepCopy(binaryExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::AssignmentExpr& assignmentExpr)
{
    using Oper = Parsing::AssignmentExpr::Operator;

    assignmentExpr.lhs = convertArrayType(*assignmentExpr.lhs);
    assignmentExpr.rhs = convertArrayType(*assignmentExpr.rhs);

    if (hasError())
        return Parsing::deepCopy(assignmentExpr);
    const Type leftType = assignmentExpr.lhs->type->type;
    const Type rightType = assignmentExpr.rhs->type->type;
    if (rightType == Type::Pointer && (assignmentExpr.op == Oper::PlusAssign || assignmentExpr.op == Oper::MinusAssign)) {
        addError("Cannot have pointer as right hand side of compound assign", assignmentExpr.rhs->location);
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
        assignmentExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(leftType), std::move(assignmentExpr.rhs));
    }
    assignmentExpr.type = Parsing::deepCopy(*assignmentExpr.lhs->type);
    return Parsing::deepCopy(assignmentExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::CastExpr& castExpr)
{
    castExpr.expr = convertArrayType(*castExpr.expr);

    if (hasError())
        return Parsing::deepCopy(castExpr);
    const Type outerType = castExpr.type->type;
    const Type innerType = castExpr.expr->type->type;
    if (outerType == Type::Double && innerType == Type::Pointer)
        addError("Cannot convert pointer to double", castExpr.location);
    if (outerType == Type::Pointer && innerType == Type::Double)
        addError("Cannot convert double to pointer", castExpr.location);
    return Parsing::deepCopy(castExpr);
}

bool TypeResolution::isLegalAssignExpr(Parsing::AssignmentExpr& assignmentExpr)
{
    using Oper = Parsing::AssignmentExpr::Operator;

    const Type leftType = assignmentExpr.lhs->type->type;
    const Type rightType = assignmentExpr.rhs->type->type;
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
        if (!Parsing::areEquivalent(*assignmentExpr.lhs->type, *assignmentExpr.rhs->type)) {
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

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::TernaryExpr& ternaryExpr)
{
    ternaryExpr.condition = convertArrayType(*ternaryExpr.condition);
    ternaryExpr.trueExpr = convertArrayType(*ternaryExpr.trueExpr);
    ternaryExpr.falseExpr = convertArrayType(*ternaryExpr.falseExpr);

    const Type trueType = ternaryExpr.trueExpr->type->type;
    const Type falseType = ternaryExpr.falseExpr->type->type;
    const Type commonType = getCommonType(trueType, falseType);
    if (trueType == Type::Pointer || falseType == Type::Pointer) {
        if (trueType == Type::Pointer && falseType == Type::Pointer) {
            if (!Parsing::areEquivalent(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
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
    if (Parsing::areEquivalent(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
        return true;
    if (canConvertToNullPtr(*ternaryExpr.trueExpr) || canConvertToNullPtr(*ternaryExpr.falseExpr))
        return true;
    return false;
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::AddrOffExpr& addrOffExpr)
{
    addrOffExpr.reference = convert(*addrOffExpr.reference);

    if (hasError())
        return Parsing::deepCopy(addrOffExpr);
    if (addrOffExpr.reference->kind == Parsing::Expr::Kind::AddrOf) {
        addError("Cannot have address of of address of operation", addrOffExpr.location);
        return Parsing::deepCopy(addrOffExpr);
    }
    addrOffExpr.type = std::make_unique<Parsing::PointerType>(
        Parsing::deepCopy(*addrOffExpr.reference->type));
    return Parsing::deepCopy(addrOffExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::DereferenceExpr& dereferenceExpr)
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



std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::SubscriptExpr& subscriptExpr)
{
    subscriptExpr.referencing = convertArrayType(*subscriptExpr.referencing);
    subscriptExpr.index = convertArrayType(*subscriptExpr.index);

    if (hasError())
        return Parsing::deepCopy(subscriptExpr);

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