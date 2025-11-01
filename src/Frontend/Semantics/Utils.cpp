#include "ASTDeepCopy.hpp"
#include "ASTTypes.hpp"
#include "Utils.hpp"
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
    if (isBinaryComparison(binaryExpr.op)) {
        if (commonType == Type::Double || isSigned(commonType))
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        else
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::U32);
        return;
    }
    binaryExpr.type = std::make_unique<Parsing::VarType>(commonType);
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
    if (!isIntegerType(constExpr.type->type))
        return false;
    return canConvertToNullPtr(constExpr);
}

bool isBinaryComparison(const Parsing::BinaryExpr::Operator oper)
{
    using Operator = Parsing::BinaryExpr::Operator;
    return oper == Operator::Equal       || oper == Operator::NotEqual ||
           oper == Operator::LessThan    || oper == Operator::LessOrEqual ||
           oper == Operator::GreaterThan || oper == Operator::GreaterOrEqual;
}

} // Semantics