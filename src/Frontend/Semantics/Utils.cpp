#include "Utils.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "TypeConversion.hpp"

namespace Semantics {

void assignTypeToArithmeticBinaryExpr(Parsing::BinaryExpr& binaryExpr)
{
    using BinaryOp = Parsing::BinaryExpr::Operator;
    const Type leftType = binaryExpr.lhs->type->type;
    const Type rightType = binaryExpr.rhs->type->type;
    const Type commonType = getCommonType(leftType, rightType);
    if (commonType != leftType)
        binaryExpr.lhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.lhs));
    if (commonType != rightType)
        binaryExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.rhs));
    if (binaryExpr.op == BinaryOp::LeftShift || binaryExpr.op == BinaryOp::RightShift) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(leftType);
        return;
    }
    if (isBinaryComparison(binaryExpr)) {
        if (commonType == Type::Double || isSigned(commonType))
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        else
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::U32);
        return;
    }
    binaryExpr.type = std::make_unique<Parsing::VarType>(commonType);
}

std::unique_ptr<Parsing::Expr> deepCopy(const Parsing::Expr& expr)
{
    using Kind = Parsing::Expr::Kind;
    if (expr.kind == Kind::Var) {
        const auto varExpr = dynCast<const Parsing::VarExpr>(&expr);
        auto copy = std::make_unique<Parsing::VarExpr>(varExpr->name);
        if (expr.type != nullptr)
            copy->type = Parsing::deepCopy(*expr.type);
        copy->referingTo = varExpr->referingTo;
        return copy;
    }
    if (expr.kind == Kind::Dereference) {
        const auto dereferenceExpr = dynCast<const Parsing::DereferenceExpr>(&expr);
        std::unique_ptr<Parsing::Expr> inner = deepCopy(*dereferenceExpr->reference);
        if (expr.type != nullptr)
            inner->type = Parsing::deepCopy(*expr.type);
        return std::make_unique<Parsing::DereferenceExpr>(std::move(inner));
    }
    std::abort();
}

bool canConvertToNullPtr(const Parsing::ConstExpr& constExpr)
{
    const Type type  = constExpr.type->type;
    if (type == Type::I32)
        return 0 == std::get<i32>(constExpr.value);
    if (type == Type::U32)
        return 0 == std::get<u32>(constExpr.value);
    if (type == Type::I64)
        return 0 == std::get<i64>(constExpr.value);
    if (type == Type::U64)
        return 0 == std::get<u64>(constExpr.value);
    return false;
}

bool canConvertToNullPtr(const Parsing::Expr& expr)
{
    if (expr.kind != Parsing::Expr::Kind::Constant)
        return false;
    const auto constExpr = dynCast<const Parsing::ConstExpr>(&expr);
    return canConvertToNullPtr(*constExpr);
}

bool canConvertToPtr(const Parsing::ConstExpr& constExpr)
{
    if (!isInteger(constExpr.type->type))
        return false;
    return canConvertToNullPtr(constExpr);
}

bool isBinaryComparison(const Parsing::BinaryExpr& binaryExpr)
{
    using Operator = Parsing::BinaryExpr::Operator;
    return binaryExpr.op == Operator::Equal || binaryExpr.op == Operator::NotEqual ||
           binaryExpr.op == Operator::LessThan || binaryExpr.op == Operator::LessOrEqual ||
           binaryExpr.op == Operator::GreaterThan || binaryExpr.op == Operator::GreaterOrEqual;
}

} // Semantics