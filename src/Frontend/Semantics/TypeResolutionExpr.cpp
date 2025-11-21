#include "TypeResolutionExpr.hpp"
#include "DynCast.hpp"
#include "ASTUtils.hpp"
#include "TypeConversion.hpp"

namespace {
using BinaryOp = Parsing::BinaryExpr::Operator;
}

namespace Semantics {

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertArrayType(Parsing::Expr& expr)
{
    auto genExpr = convert(expr);
    if (genExpr->type && genExpr->type->type == Type::Array && genExpr->kind != Parsing::Expr::Kind::AddrOf) {
        if (genExpr->kind != Parsing::Expr::Kind::AddrOf) {
            const auto arrayType = dynCast<Parsing::ArrayType>(genExpr->type.get());
            auto addressOf = std::make_unique<Parsing::AddrOffExpr>(genExpr->location, std::move(genExpr));
            addressOf->type = std::make_unique<Parsing::PointerType>(Parsing::deepCopy(*arrayType->elementType));
            return addressOf;
        }
    }
    if (genExpr->type && isStructuredType(genExpr->type->type)) {
        if (varTable.isInCompleteStructuredType(*genExpr->type))
            addError("Invalid use of undefined structured type", genExpr->location);
    }
    return genExpr;
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convert(Parsing::Expr& expr)
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
        case ExprKind::Dot: {
            const auto dot = dynCast<Parsing::DotExpr>(&expr);
            return convertDotExpr(*dot);
        }
        case ExprKind::Arrow: {
            const auto arrow = dynCast<Parsing::ArrowExpr>(&expr);
            return convertArrowExpr(*arrow);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertConstExpr(Parsing::ConstExpr& constExpr)
{
    return std::make_unique<Parsing::ConstExpr>(std::move(constExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertStringExpr(Parsing::StringExpr& stringExpr) const
{
    if (m_inArrayInit)
        return std::make_unique<Parsing::StringExpr>(std::move(stringExpr));
    stringExpr.type = std::make_unique<Parsing::ArrayType>(
        std::make_unique<Parsing::VarType>(Type::Char), stringExpr.value.size() + 1);
    return std::make_unique<Parsing::StringExpr>(std::move(stringExpr));
}

void TypeResolutionExpr::validateAndConvertFuncCallArgs(
    Parsing::FuncCallExpr& funCallExpr,
    const std::vector<std::unique_ptr<Parsing::TypeBase>>& params) const
{
    for (size_t i = 0; i < funCallExpr.args.size(); ++i) {
        const i64 location = funCallExpr.args[i]->location;
        const Parsing::TypeBase* const callTypeBase = funCallExpr.args[i]->type.get();
        const Parsing::TypeBase* const paramTypeBase = params[i].get();
        const Type typeInner = callTypeBase->type;
        const Type castToType = params[i]->type;
        if (isStructuredType(typeInner) || isStructuredType(castToType)) {
            if (!Parsing::areEquivalentTypes(*callTypeBase, *paramTypeBase)) {
                addError("Cannot convert structured types in funcall", location);
            }
        }
        if (typeInner == Type::Pointer && castToType == Type::Pointer) {
            if (!Parsing::areEquivalentTypes(*callTypeBase, *paramTypeBase)
                && !isVoidPointer(*callTypeBase) && !isVoidPointer(*paramTypeBase))
                addError("Function arg of of different type with param", location);
        }
        if (castToType != Type::Pointer && typeInner == Type::Pointer)
            addError("Cannot cast pointer arg to non pointer", location);
        if (typeInner != castToType) {
            funCallExpr.args[i] = std::make_unique<Parsing::CastExpr>(
                Parsing::deepCopy(*paramTypeBase), std::move(funCallExpr.args[i]));
        }
    }
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertFuncCallExpr(Parsing::FuncCallExpr& funCallExpr)
{
    const auto it = m_functions.find(funCallExpr.name);
    if (it == m_functions.end()) {
        addError("Called function is not declared", funCallExpr.location);
        return std::make_unique<Parsing::FuncCallExpr>(std::move(funCallExpr));
    }
    if (it->second.paramTypes.size() != funCallExpr.args.size()) {
        addError("Called function is not declared", funCallExpr.location);
        return std::make_unique<Parsing::FuncCallExpr>(std::move(funCallExpr));
    }

    std::vector<std::unique_ptr<Parsing::Expr>> args;
    for (const auto& arg : funCallExpr.args)
        args.push_back(convertArrayType(*arg));
    funCallExpr.args = std::move(args);

    if (hasError())
        return std::make_unique<Parsing::FuncCallExpr>(std::move(funCallExpr));
    validateAndConvertFuncCallArgs(funCallExpr, it->second.paramTypes);
    return std::make_unique<Parsing::FuncCallExpr>(std::move(funCallExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertVarExpr(Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    return std::make_unique<Parsing::VarExpr>(std::move(varExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertUnaryExpr(Parsing::UnaryExpr& unaryExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;

    unaryExpr.innerExpr = convertArrayType(*unaryExpr.innerExpr);

    if (hasError())
        return std::make_unique<Parsing::UnaryExpr>(std::move(unaryExpr));

    if (isStructuredType(unaryExpr.innerExpr->type->type)) {
        addError("Cannot apply unary operation to structure type", unaryExpr.innerExpr->location);
        return std::make_unique<Parsing::UnaryExpr>(std::move(unaryExpr));
    }
    if (unaryExpr.innerExpr->type->type == Type::Void) {
        addError("Cannot apply unary operation to void", unaryExpr.innerExpr->location);
        return std::make_unique<Parsing::UnaryExpr>(std::move(unaryExpr));
    }
    if (isVoidPointer(*unaryExpr.innerExpr->type)) {
        addError("Cannot apply unary operation to void pointer", unaryExpr.innerExpr->location);
        return std::make_unique<Parsing::UnaryExpr>(std::move(unaryExpr));
    }
    if (unaryExpr.innerExpr->type->type == Type::Array || unaryExpr.innerExpr->kind == Parsing::Expr::Kind::AddrOf) {
        if (isPrefixOp(unaryExpr.op))
            addError("Cannot apply prefix decrement", unaryExpr.location);
        if (Parsing::isPostfixOp(unaryExpr.op))
            addError("Cannot apply postfix decrement", unaryExpr.location);
    }

    if (unaryExpr.innerExpr->type->type == Type::Double) {
        if (unaryExpr.op == Operator::Complement) {
            addError("Cannot complement double", unaryExpr.location);
            return std::make_unique<Parsing::UnaryExpr>(std::move(unaryExpr));
        }
    }
    if (unaryExpr.innerExpr->type->type == Type::Pointer) {
        if (isIllegalUnaryPointerOperator(unaryExpr.op)) {
            addError("Cannot apply operator to pointer", unaryExpr.location);
            return std::make_unique<Parsing::UnaryExpr>(std::move(unaryExpr));
        }
    }
    if (unaryExpr.op == Operator::Not)
        unaryExpr.type = std::make_unique<Parsing::VarType>(s_boolType);
    else
        unaryExpr.type = Parsing::deepCopy(*unaryExpr.innerExpr->type);

    if (isCharacterType(unaryExpr.innerExpr->type->type) &&
        (unaryExpr.op == Operator::Negate || unaryExpr.op == Operator::Complement)) {
        unaryExpr.innerExpr = convertOrCastToType(unaryExpr.innerExpr, Type::I32);
        unaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
    }

    return std::make_unique<Parsing::UnaryExpr>(std::move(unaryExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertBinaryExpr(Parsing::BinaryExpr& binaryExpr)
{
    binaryExpr.lhs = convertArrayType(*binaryExpr.lhs);
    binaryExpr.rhs = convertArrayType(*binaryExpr.rhs);

    if (hasError())
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    const Type leftType = binaryExpr.lhs->type->type;
    const Type rightType = binaryExpr.rhs->type->type;
    if (isStructuredType(leftType) || isStructuredType(rightType)) {
        addError("Cannot apply binary operator to structured type", binaryExpr.location);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    }
    if (leftType == Type::Void || rightType == Type::Void) {
        addError("Cannot have void type in binary expression.", binaryExpr.location);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    }
    if (binaryExpr.op == BinaryOp::And || binaryExpr.op == BinaryOp::Or) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    }
    const Type commonType = getCommonType(leftType, rightType);
    if (commonType == Type::Double && isIllegalFloatingBinaryOperator(binaryExpr.op)) {
        addError("Cannot apply operator to double", binaryExpr.location);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    }
    if (leftType == Type::Pointer || rightType == Type::Pointer)
        return handleBinaryPtr(binaryExpr, leftType, rightType);
    assignTypeToArithmeticBinaryExpr(binaryExpr);
    return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
}

bool areValidNonArithmeticTypesInBinaryExpr(const Parsing::BinaryExpr& binaryExpr,
        const Type leftType, const Type rightType)
{
    if (Parsing::areEquivalentTypes(*binaryExpr.lhs->type, *binaryExpr.rhs->type))
        return true;
    if (canConvertToNullPtr(*binaryExpr.lhs) || canConvertToNullPtr(*binaryExpr.rhs))
        return true;
    const Type commonType = getCommonType(leftType, rightType);
    if (commonType == Type::Pointer && (isIntegerType(leftType) || isIntegerType(rightType)) &&
            (binaryExpr.op == BinaryOp::Add || binaryExpr.op == BinaryOp::Subtract))
        return true;
    return false;
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::handleAddSubtractPtrToIntegerTypes(
    Parsing::BinaryExpr& binaryExpr, const Type leftType, const Type rightType) const
{
    if (isVoidPointer(*binaryExpr.lhs->type) || isVoidPointer(*binaryExpr.rhs->type)) {
        addError("Cannot add or subtract void pointer", binaryExpr.location);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    }
    if (isIntegerType(rightType)) {
        if (varTable.isPointerToInCompleteStructuredType(*binaryExpr.lhs->type)) {
            addError("Cannot do pointer arithmetic on incomplete types", binaryExpr.lhs->location);
            return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
        }
        if (rightType != Type::I64)
            binaryExpr.rhs = convertOrCastToType(binaryExpr.rhs, Type::I64);
        binaryExpr.type = Parsing::deepCopy(*binaryExpr.lhs->type);
    }
    else if (isIntegerType(leftType)) {
        if (varTable.isPointerToInCompleteStructuredType(*binaryExpr.rhs->type)) {
            addError("Cannot do pointer arithmetic on incomplete types", binaryExpr.rhs->location);
            return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
        }
        if (binaryExpr.op == BinaryOp::Subtract) {
            addError("Cannot subtract a pointer form an integer type", binaryExpr.location);
            return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
        }
        if (leftType != Type::I64)
            binaryExpr.lhs = convertOrCastToType(binaryExpr.lhs, Type::I64);
        binaryExpr.type = Parsing::deepCopy(*binaryExpr.rhs->type);
        auto returnExpr = std::make_unique<Parsing::BinaryExpr>(binaryExpr.location,
            binaryExpr.op, std::move(binaryExpr.rhs), std::move(binaryExpr.lhs));
        if (binaryExpr.type)
            returnExpr->type =  std::move(binaryExpr.type);
        return returnExpr;
    }
    return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::handlePtrToPtrBinaryOpers(Parsing::BinaryExpr& binaryExpr) const
{
    if (Parsing::areEquivalentTypes(*binaryExpr.lhs->type, *binaryExpr.rhs->type)) {
        if (isBinaryComparison(binaryExpr.op)) {
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
            return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
        }
        if (varTable.isPointerToInCompleteStructuredType(*binaryExpr.lhs->type)) {
            addError("Cannot do pointer arithmetic on incomplete types", binaryExpr.location);
            return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
        }
        if (isVoidPointer(*binaryExpr.lhs->type) || isVoidPointer(*binaryExpr.rhs->type)) {
            addError("Cannot apply subtraction to void pointers", binaryExpr.location);
            return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
        }
        if (binaryExpr.op == BinaryOp::Subtract) {
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I64);
            return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
        }
    }
    if ((binaryExpr.op == BinaryOp::Equal || binaryExpr.op == BinaryOp::NotEqual) &&
        (isVoidPointer(*binaryExpr.lhs->type) || isVoidPointer(*binaryExpr.rhs->type))) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    }
    addError("Cannot apply operator to pointers", binaryExpr.location);
    return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::handleBinaryPtr(
    Parsing::BinaryExpr& binaryExpr,
    const Type leftType,
    const Type rightType) const
{
    if (isIllegalPtrBinaryOperation(binaryExpr.op)) {
        addError("Cannot apply operator to pointer", binaryExpr.location);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    }

    if (leftType == Type::Pointer && rightType == Type::Pointer)
        return handlePtrToPtrBinaryOpers(binaryExpr);

    if (leftType == Type::Double || rightType == Type::Double) {
        addError("Cannot have binary on double and pointer", binaryExpr.location);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    }

    if (isUnallowedComparisonBetweenPtrAndInteger(binaryExpr.op)) {
        addError("Unallowed comparison operator between pointer and integer types", binaryExpr.location);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
    }

    if (binaryExpr.op == BinaryOp::Add || binaryExpr.op == BinaryOp::Subtract)
        return handleAddSubtractPtrToIntegerTypes(binaryExpr, leftType, rightType);

    if (!areValidNonArithmeticTypesInBinaryExpr(binaryExpr, leftType, rightType)) {
        addError("Cannot apply operator to non-arithmetic types", binaryExpr.location);
        return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
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
    return std::make_unique<Parsing::BinaryExpr>(std::move(binaryExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertAssignExpr(Parsing::AssignmentExpr& assignmentExpr)
{
    using Oper = Parsing::AssignmentExpr::Operator;

    assignmentExpr.lhs = convertArrayType(*assignmentExpr.lhs);
    assignmentExpr.rhs = convertArrayType(*assignmentExpr.rhs);

    if (hasError())
        return std::make_unique<Parsing::AssignmentExpr>(std::move(assignmentExpr));
    if (assignmentExpr.op == Oper::Assign)
        return converSimpleAssignExpr(assignmentExpr);

    if (!isLegalAssignExpr(assignmentExpr))
        return std::make_unique<Parsing::AssignmentExpr>(std::move(assignmentExpr));

    if (assignmentExpr.lhs->type->type == Type::Pointer && isIntegerType(assignmentExpr.rhs->type->type))
        assignmentExpr.rhs = convertOrCastToType(assignmentExpr.rhs, Type::I64);
    assignmentExpr.type = Parsing::deepCopy(*assignmentExpr.lhs->type);
    return std::make_unique<Parsing::AssignmentExpr>(std::move(assignmentExpr));
}

bool TypeResolutionExpr::isLegalAssignExpr(const Parsing::AssignmentExpr& assignmentExpr) const
{
    using Oper = Parsing::AssignmentExpr::Operator;

    const Type leftType = assignmentExpr.lhs->type->type;
    const Type rightType = assignmentExpr.rhs->type->type;
    if (rightType == Type::Pointer) {
        addError("Cannot have pointer as right hand side of compound assign", assignmentExpr.rhs->location);
        return false;
    }
    if (isVoidPointer(*assignmentExpr.lhs->type)) {
        addError("Cannot compound assign to void ptr", assignmentExpr.lhs->location);
        return false;
    }
    if (isStructuredType(leftType) || isStructuredType(rightType)) {
        addError("Cannot compound assign to structured type", assignmentExpr.location);
        return false;
    }
    if (rightType == Type::Void) {
        addError("Cannot assign with type void.", assignmentExpr.rhs->location);
        return false;
    }
    const Type commonType = getCommonType(leftType, rightType);
    const BinaryOp binaryOp = convertAssignOperation(assignmentExpr.op);
    if (commonType == Type::Double && isIllegalFloatingBinaryOperator(binaryOp)) {
        addError("Is double compound assign operator", assignmentExpr.location);
        return false;
    }
    if (leftType == Type::Pointer) {
        if (isIllegalPtrBinaryOperation(binaryOp)) {
            addError("Is illegal pointer assign operator", assignmentExpr.location);
            return false;
        }
        if ((assignmentExpr.op == Oper::PlusAssign || assignmentExpr.op == Oper::MinusAssign) &&
                 isIntegerType(rightType)) {
            return true;
        }
        addError("Cannot convert one type to pointer", assignmentExpr.location);
        return false;
    }
    return true;
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::converSimpleAssignExpr(
    Parsing::AssignmentExpr& assignmentExpr) const
{
    assignmentExpr.rhs = converOrAssign(
        *assignmentExpr.lhs->type,
        *assignmentExpr.rhs->type,
        assignmentExpr.rhs,
        m_errors);
    assignmentExpr.type = Parsing::deepCopy(*assignmentExpr.lhs->type);
    return std::make_unique<Parsing::AssignmentExpr>(std::move(assignmentExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertCastExpr(Parsing::CastExpr& castExpr)
{
    castExpr.innerExpr = convertArrayType(*castExpr.innerExpr);

    if (hasError())
        return std::make_unique<Parsing::CastExpr>(std::move(castExpr));
    const Type outerType = castExpr.type->type;
    const Type innerType = castExpr.innerExpr->type->type;
    if (isStructuredType(outerType))
        addError("Cannot cast to structured type", castExpr.location);
    if (outerType != Type::Void && isStructuredType(innerType))
        addError("Cannot cast from structured type", castExpr.location);
    if (Parsing::areEquivalentTypes(*castExpr.type, *castExpr.innerExpr->type))
        return std::make_unique<Parsing::CastExpr>(std::move(castExpr));
    if (innerType == Type::Void)
        addError("Cannot cast to void", castExpr.location);
    if (isPointerToVoidArray(*castExpr.type))
        addError("Cannot cast to pointer to void array", castExpr.location);
    if (isArrayOfVoidPointer(*castExpr.type))
        addError("Cannot cast to void pointer array", castExpr.location);
    if (outerType == Type::Double && innerType == Type::Pointer)
        addError("Cannot convert pointer to double", castExpr.location);
    if (outerType == Type::Pointer && innerType == Type::Double)
        addError("Cannot convert double to pointer", castExpr.location);
    if (isArithmetic(outerType) && isArithmetic(innerType))
        return convertOrCastToType(castExpr.innerExpr, outerType);
    if (outerType == Type::Pointer && innerType == Type::Pointer) {
        auto innerTypeCopy = Parsing::deepCopy(*castExpr.type);
        castExpr.innerExpr->type = std::move(innerTypeCopy);
        return std::make_unique<Parsing::CastExpr>(std::move(castExpr));
    }
    return std::make_unique<Parsing::CastExpr>(std::move(castExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::validateAndConvertPtrsInTernaryExpr(
    Parsing::TernaryExpr& ternaryExpr,
    const Type trueType,
    const Type falseType) const
{
    if (trueType == Type::Pointer && falseType == Type::Pointer) {
        if (isVoidPointer(*ternaryExpr.trueExpr->type) || isVoidPointer(*ternaryExpr.falseExpr->type)) {
            ternaryExpr.type = std::make_unique<Parsing::PointerType>(
                std::make_unique<Parsing::VarType>(Type::Void));
            return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));
        }
        if (!Parsing::areEquivalentTypes(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
            addError("Ternary true and false expression must have same type", ternaryExpr.location);
        ternaryExpr.type = std::move(Parsing::deepCopy(*ternaryExpr.trueExpr->type));
        return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));
    }
    if (!areValidNonArithmeticTypesInTernaryExpr(ternaryExpr)) {
        addError("Are not valid non arithmetic types in ternary", ternaryExpr.location);
        return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));
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
    return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertTernaryExpr(Parsing::TernaryExpr& ternaryExpr)
{
    ternaryExpr.condition = convertArrayType(*ternaryExpr.condition);
    ternaryExpr.trueExpr = convertArrayType(*ternaryExpr.trueExpr);
    ternaryExpr.falseExpr = convertArrayType(*ternaryExpr.falseExpr);

    if (hasError())
        return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));

    if (!isScalarType(*ternaryExpr.condition->type))
        addError("Ternary condition must have scalar type", ternaryExpr.condition->location);

    const Type trueType = ternaryExpr.trueExpr->type->type;
    const Type falseType = ternaryExpr.falseExpr->type->type;
    if (isStructuredType(trueType) || isStructuredType(falseType)) {
        if (!isStructuredType(trueType) || !isStructuredType(falseType)) {
            addError("Cannot have structured and unstructured type in ternary.", ternaryExpr.location);
            return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));
        }
        const auto trueTypeStructured = dynCast<const Parsing::StructuredType>(
            ternaryExpr.trueExpr->type.get());
        const auto falseTypeStructured = dynCast<const Parsing::StructuredType>(
            ternaryExpr.falseExpr->type.get());
        if (trueTypeStructured->identifier != falseTypeStructured->identifier) {
            addError("Cannot have different structured types in ternary.", ternaryExpr.location);
            return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));
        }
    }
    if (trueType == Type::Void && falseType == Type::Void) {
        ternaryExpr.type = Parsing::deepCopy(*ternaryExpr.trueExpr->type);
        return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));
    }
    if (trueType == Type::Void || falseType == Type::Void) {
        addError("Ternary true and false expression must not have type void", ternaryExpr.location);
        return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));
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
    if (commonType == trueType)
        ternaryExpr.type = Parsing::deepCopy(*ternaryExpr.falseExpr->type);
    else
        ternaryExpr.type = Parsing::deepCopy(*ternaryExpr.trueExpr->type);
    return std::make_unique<Parsing::TernaryExpr>(std::move(ternaryExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertAddrOfExpr(Parsing::AddrOffExpr& addrOffExpr)
{
    addrOffExpr.reference = convert(*addrOffExpr.reference);

    if (hasError())
        return std::make_unique<Parsing::AddrOffExpr>(std::move(addrOffExpr));
    if (addrOffExpr.reference->kind == Parsing::Expr::Kind::AddrOf) {
        addError("Cannot have address-of of address-of operation", addrOffExpr.location);
        return std::make_unique<Parsing::AddrOffExpr>(std::move(addrOffExpr));
    }
    addrOffExpr.type = std::make_unique<Parsing::PointerType>(
        Parsing::deepCopy(*addrOffExpr.reference->type));
    return std::make_unique<Parsing::AddrOffExpr>(std::move(addrOffExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertDerefExpr(Parsing::DereferenceExpr& dereferenceExpr)
{
    dereferenceExpr.reference = convertArrayType(*dereferenceExpr.reference);

    if (hasError())
        return std::make_unique<Parsing::DereferenceExpr>(std::move(dereferenceExpr));
    if (dereferenceExpr.reference->type->type != Type::Pointer) {
        addError("Cannot dereference non pointer", dereferenceExpr.location);
        return std::make_unique<Parsing::DereferenceExpr>(std::move(dereferenceExpr));
    }
    const auto referencedPtrType = dynCast<const Parsing::PointerType>(dereferenceExpr.reference->type.get());
    dereferenceExpr.type = Parsing::deepCopy(*referencedPtrType->referenced);
    return std::make_unique<Parsing::DereferenceExpr>(std::move(dereferenceExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertSubscriptExpr(Parsing::SubscriptExpr& subscriptExpr)
{
    subscriptExpr.referencing = convertArrayType(*subscriptExpr.referencing);
    subscriptExpr.index = convertArrayType(*subscriptExpr.index);

    if (hasError())
        return std::make_unique<Parsing::SubscriptExpr>(std::move(subscriptExpr));

    if (varTable.isPointerToInCompleteStructuredType(*subscriptExpr.referencing->type)) {
        addError("Cannot subscript incomplete pointer type", subscriptExpr.referencing->location);
        return std::make_unique<Parsing::SubscriptExpr>(std::move(subscriptExpr));
    }
    if (isVoidPointer(*subscriptExpr.referencing->type)) {
        addError("Cannot subscript void pointer", subscriptExpr.referencing->location);
        return std::make_unique<Parsing::SubscriptExpr>(std::move(subscriptExpr));
    }

    std::unique_ptr<Parsing::Expr> result = nullptr;

    const Type referencingType = subscriptExpr.referencing->type->type;
    const Type indexType = subscriptExpr.index->type->type;

    if (referencingType == Type::Pointer && isIntegerType(indexType))
        result = std::make_unique<Parsing::SubscriptExpr>(std::move(subscriptExpr));
    else if (indexType == Type::Pointer && isIntegerType(referencingType)) {
        result = std::make_unique<Parsing::SubscriptExpr>(subscriptExpr.location,
            std::move(subscriptExpr.index), std::move(subscriptExpr.referencing));
    }
    else {
        addError("Cannot convert subscript to non pointer", subscriptExpr.location);
        return std::make_unique<Parsing::SubscriptExpr>(std::move(subscriptExpr));
    }
    const auto subscriptExprPtr = dynCast<Parsing::SubscriptExpr>(result.get());
    if (subscriptExprPtr->index->type->type != Type::I64)
        subscriptExprPtr->index = convertOrCastToType(subscriptExprPtr->index, Type::I64);
    const auto ptrType = dynCast<Parsing::PointerType>(subscriptExprPtr->referencing->type.get());
    result->type = Parsing::deepCopy(*ptrType->referenced);
    return result;
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertSizeOfExprExpr(Parsing::SizeOfExprExpr& sizeOfExprExpr)
{
    sizeOfExprExpr.innerExpr = convert(*sizeOfExprExpr.innerExpr);
    if (hasError())
        return std::make_unique<Parsing::SizeOfExprExpr>(std::move(sizeOfExprExpr));
    if (sizeOfExprExpr.innerExpr && sizeOfExprExpr.innerExpr->type) {
        if (isVoidArray(*sizeOfExprExpr.innerExpr->type))
            addError("Cannot call sizeof on void array expression", sizeOfExprExpr.location);
        if (sizeOfExprExpr.innerExpr->type->type == Type::Void)
            addError("Cannot call sizeof on void expression", sizeOfExprExpr.location);
    }
    if (varTable.isInCompleteStructuredType(*sizeOfExprExpr.innerExpr->type))
        addError("Cannot call sizeof on incomplete type expr", sizeOfExprExpr.location);
    return std::make_unique<Parsing::SizeOfExprExpr>(std::move(sizeOfExprExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertSizeOfExprType(
    Parsing::SizeOfTypeExpr& sizeOfTypeExpr) const
{
    if (isVoidArray(*sizeOfTypeExpr.sizeType))
        addError("Cannot call sizeof on void array type", sizeOfTypeExpr.location);
    if (sizeOfTypeExpr.sizeType->type == Type::Void)
        addError("Cannot call sizeof on void type", sizeOfTypeExpr.location);
    if (varTable.isInCompleteStructuredType(*sizeOfTypeExpr.sizeType))
        addError("Cannot call sizeof on void type", sizeOfTypeExpr.location);
    return std::make_unique<Parsing::SizeOfTypeExpr>(std::move(sizeOfTypeExpr));
}

const Parsing::TypeBase* TypeResolutionExpr::validateStructuredAccessors(
    const Parsing::TypeBase* structuredType,
    const std::string& identifier,
    const i64 location) const
{
    if (structuredType->kind != Parsing::TypeBase::Kind::Structured) {
        addError("Initiated extern variable", location);
        return nullptr;
    }
    const auto structType = dynCast<const Parsing::StructuredType>(structuredType);
    const auto entry = varTable.lookupEntry(structType->identifier);
    if (entry == nullptr) {
        addError("Cannot call structured accessor on non structured type", location);
        return nullptr;
    }
    return getTypeFromMembers(entry->memberMap, identifier);
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertDotExpr(Parsing::DotExpr& dotExpr)
{
    dotExpr.structuredExpr = convertArrayType(*dotExpr.structuredExpr);
    if (hasError())
        return std::make_unique<Parsing::DotExpr>(std::move(dotExpr));
    const Parsing::TypeBase* type = validateStructuredAccessors(
        dotExpr.structuredExpr->type.get(), dotExpr.identifier, dotExpr.location);
    if (type == nullptr) {
        addError("Could not find member for " + dotExpr.identifier, dotExpr.location);
        return std::make_unique<Parsing::DotExpr>(std::move(dotExpr));
    }
    dotExpr.type = Parsing::deepCopy(*type);
    return std::make_unique<Parsing::DotExpr>(std::move(dotExpr));
}

std::unique_ptr<Parsing::Expr> TypeResolutionExpr::convertArrowExpr(Parsing::ArrowExpr& arrowExpr)
{
    arrowExpr.pointerExpr = convertArrayType(*arrowExpr.pointerExpr);
    if (hasError())
        return std::make_unique<Parsing::ArrowExpr>(std::move(arrowExpr));
    if (arrowExpr.pointerExpr->type->kind != Parsing::TypeBase::Kind::Pointer) {
        addError("Arrow prefix must be on pointer", arrowExpr.pointerExpr->location);
        return std::make_unique<Parsing::ArrowExpr>(std::move(arrowExpr));
    }
    const auto pointerType = dynCast<const Parsing::PointerType>(arrowExpr.pointerExpr->type.get());
    const Parsing::TypeBase* type = validateStructuredAccessors(
        pointerType->referenced.get(), arrowExpr.identifier, arrowExpr.location);
    if (type == nullptr) {
        addError("Could not find member for " + arrowExpr.identifier, arrowExpr.location);
        return std::make_unique<Parsing::ArrowExpr>(std::move(arrowExpr));
    }
    arrowExpr.type = Parsing::deepCopy(*type);
    return std::make_unique<Parsing::ArrowExpr>(std::move(arrowExpr));
}

bool areValidNonArithmeticTypesInTernaryExpr(const Parsing::TernaryExpr& ternaryExpr)
{
    if (Parsing::areEquivalentTypes(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
        return true;
    if (canConvertToNullPtr(*ternaryExpr.trueExpr) || canConvertToNullPtr(*ternaryExpr.falseExpr))
        return true;
    return false;
}

Parsing::TypeBase* getTypeFromMembers(
    const std::unordered_map<std::string, MemberEntry>& memberMap,
    const std::string& identifier)
{
    const auto it = memberMap.find(identifier);
    if (it == memberMap.end())
        return nullptr;
    return it->second.type.get();
}
} // Semantics