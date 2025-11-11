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
        binaryExpr.lhs = convertOrCastToType(*binaryExpr.lhs, commonType);
    if (commonType != rightType)
        binaryExpr.rhs = convertOrCastToType(*binaryExpr.rhs, commonType);
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

std::unique_ptr<Parsing::Expr> convertOrCastToType(const Parsing::Expr& expr, const Type targetType)
{
    if (expr.kind != Parsing::Expr::Kind::Constant) {
        return std::make_unique<Parsing::CastExpr>(
            expr.location,
            std::make_unique<Parsing::VarType>(targetType),
            Parsing::deepCopy(expr));
    }

    const auto constExpr = dynCast<const Parsing::ConstExpr>(&expr);
    std::variant<i32, i64, u32, u64, double> convertedValue;

    switch (targetType) {
        case Type::I32:
            convertedValue = getValueFromConst<i32>(*constExpr);
            break;
        case Type::I64:
            convertedValue = getValueFromConst<i64>(*constExpr);
            break;
        case Type::U32:
            convertedValue = getValueFromConst<u32>(*constExpr);
            break;
        case Type::U64:
            convertedValue = getValueFromConst<u64>(*constExpr);
            break;
        case Type::Double:
            convertedValue = getValueFromConst<double>(*constExpr);
            break;
        default:
            std::abort();
    }

    return std::make_unique<Parsing::ConstExpr>(
        constExpr->location,
        convertedValue,
        std::make_unique<Parsing::VarType>(targetType));
}

} // Semantics