#pragma once

#include "ASTExpr.hpp"
#include "TypeConversion.hpp"

namespace Semantics {

inline bool canConvertToNullPtr(const Parsing::ConstExpr& constExpr)
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

inline bool canConvertToNullPtr(const Parsing::Expr& expr)
{
    if (expr.kind != Parsing::Expr::Kind::Constant)
        return false;
    const auto constExpr = dynCast<const Parsing::ConstExpr>(&expr);
    return canConvertToNullPtr(*constExpr);
}

inline bool canConvertToPtr(const Parsing::ConstExpr& constExpr)
{
    if (!isInteger(constExpr.type->type))
        return false;
    return canConvertToNullPtr(constExpr);
}

} // Parsing