#pragma once

#include "ASTBase.hpp"
#include "ASTExpr.hpp"

namespace Parsing {

[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const TypeBase& typeBase);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const VarType& typeBase);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const FuncType& funcType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const PointerType& pointerType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const ArrayType& arrayType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const StructType& structType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const UnionType& unionType);

[[nodiscard]] bool areEquivalentTypes(const TypeBase& left, const TypeBase& right);
[[nodiscard]] bool areEquivalentTypes(const VarType& left, const VarType& right);
[[nodiscard]] bool areEquivalentTypes(const FuncType& left, const FuncType& right);
[[nodiscard]] bool areEquivalentTypes(const PointerType& left, const PointerType& right);
[[nodiscard]] bool areEquivalentTypes(const ArrayType& left, const ArrayType& right);
[[nodiscard]] bool areEquivalentTypes(const StructType& left, const StructType& right);
[[nodiscard]] bool areEquivalentTypes(const UnionType& left, const UnionType& right);
[[nodiscard]] bool areEquivalentArrayConversion(const TypeBase& left, const TypeBase& right);

[[nodiscard]] std::unique_ptr<TypeBase> convertArrayFirstDimToPtr(const TypeBase& typeBase);

[[nodiscard]] std::unique_ptr<Expr> deepCopy(const Expr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const ConstExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const StringExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const VarExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const CastExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const UnaryExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const BinaryExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const AssignmentExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const TernaryExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const FuncCallExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const DereferenceExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const AddrOffExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const SubscriptExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const SizeOfExprExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const SizeOfTypeExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const DotExpr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const ArrowExpr& expr);

template<typename TargetType>
TargetType getValueFromConst(const ConstExpr& constExpr)
{
    switch (constExpr.type->type) {
        case Type::I8:      return std::get<i8>(constExpr.value);
        case Type::U8:      return std::get<u8>(constExpr.value);
        case Type::I32:     return std::get<i32>(constExpr.value);
        case Type::U32:     return std::get<u32>(constExpr.value);
        case Type::I64:     return std::get<i64>(constExpr.value);
        case Type::U64:     return std::get<u64>(constExpr.value);
        case Type::Double:  return std::get<double>(constExpr.value);
        case Type::Char:    return std::get<char>(constExpr.value);
        default:
            std::abort();
    }
}

std::unique_ptr<Expr> convertOrCastToType(const Expr& expr, Type targetType);
void assignTypeToArithmeticBinaryExpr(BinaryExpr& binaryExpr);
bool isZeroArithmeticType(const ConstExpr& constExpr);
bool canConvertToNullPtr(const Expr& expr);
bool canConvertToPtr(const ConstExpr& constExpr);
bool isBinaryComparison(BinaryExpr::Operator oper);
std::unique_ptr<Expr> convertToArithmeticType(const Expr& expr, Type targetType);

bool isVoidPointer(const TypeBase& type);
bool isVoidArray(const TypeBase& type);
bool isArrayOfVoidPointer(const TypeBase& type);
bool isPointerToVoidArray(const TypeBase& type);
bool isScalarType(const TypeBase& type);
i64 getArraySize(TypeBase* type);
Type getArrayType(const TypeBase* type);
i64 getArrayAlignment(i64 size, Type type);
} // Parsing