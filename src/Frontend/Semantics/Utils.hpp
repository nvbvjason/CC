#pragma once

#include "ASTExpr.hpp"

namespace Semantics {

template<typename TargetType>
TargetType getValueFromConst(const Parsing::ConstExpr& constExpr)
{
    switch (constExpr.type->type) {
        case Type::I32:     return std::get<i32>(constExpr.value);
        case Type::I64:     return std::get<i64>(constExpr.value);
        case Type::U32:     return std::get<u32>(constExpr.value);
        case Type::U64:     return std::get<u64>(constExpr.value);
        case Type::Double:  return std::get<double>(constExpr.value);
        default:
            std::abort();
    }
}

std::unique_ptr<Parsing::Expr> convertOrCastToType(const Parsing::Expr& expr, Type targetType);
void assignTypeToArithmeticBinaryExpr(Parsing::BinaryExpr& binaryExpr);
bool isZeroArithmeticType(const Parsing::ConstExpr& constExpr);
bool canConvertToNullPtr(const Parsing::Expr& expr);
bool canConvertToPtr(const Parsing::ConstExpr& constExpr);
bool isBinaryComparison(Parsing::BinaryExpr::Operator oper);
std::unique_ptr<Parsing::Expr> convertToArithmeticType(const Parsing::Expr& expr, Type targetType);

} // Parsing