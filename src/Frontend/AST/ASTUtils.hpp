#pragma once

#include "ASTBase.hpp"
#include "ASTExpr.hpp"
#include "Frontend/Error.hpp"

namespace Parsing {

BinaryExpr::Operator convertAssignOperation(AssignmentExpr::Operator assignOperation);
bool isPostfixOp(UnaryExpr::Operator oper);
bool isPrefixOp(UnaryExpr::Operator oper);

[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const TypeBase& typeBase);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const VarType& varType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const FuncType& funcType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const PointerType& pointerType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const ArrayType& arrayType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const StructuredType& structuredType);

[[nodiscard]] bool areEquivalentTypes(const TypeBase& left, const TypeBase& right);
[[nodiscard]] bool areEquivalentTypes(const VarType& left, const VarType& right);
[[nodiscard]] bool areEquivalentTypes(const FuncType& left, const FuncType& right);
[[nodiscard]] bool areEquivalentTypes(const PointerType& left, const PointerType& right);
[[nodiscard]] bool areEquivalentTypes(const ArrayType& left, const ArrayType& right);
[[nodiscard]] bool areEquivalentTypes(const StructuredType& left, const StructuredType& right);
[[nodiscard]] bool areEquivalentArrayConversion(const TypeBase& left, const TypeBase& right);

[[nodiscard]] std::unique_ptr<TypeBase> convertArrayFirstDimToPtr(const TypeBase& typeBase);

std::unique_ptr<Expr> convertOrCastToType(std::unique_ptr<Expr>& expr, Type targetType);
void assignTypeToArithmeticBinaryExpr(BinaryExpr& binaryExpr);
bool isZeroArithmeticType(const ConstExpr& constExpr);
bool canConvertToNullPtr(const Expr& expr);
bool canConvertToPtr(const ConstExpr& constExpr);
bool isBinaryComparison(BinaryExpr::Operator oper);
std::unique_ptr<Expr> convertToArithmeticType(const Expr& expr, Type targetType);
std::unique_ptr<Expr> converOrAssign(const TypeBase& left,
                                     const TypeBase& right,
                                     std::unique_ptr<Expr>& expr,
                                     std::vector<Error>& errors);

bool isVoidPointer(const TypeBase& type);
bool isVoidArray(const TypeBase& type);
bool isArithmeticTypeBase(const TypeBase& type);
bool isStructuredTypeBase(const TypeBase& type);
bool isArrayOfVoidPointer(const TypeBase& type);
bool isPointerToVoidArray(const TypeBase& type);
bool isScalarType(const TypeBase& type);
i64 getArrayLength(const TypeBase* type);
Type getArrayType(const TypeBase* type);
const TypeBase* getArrayBaseType(const TypeBase& highestType);
const TypeBase* getPointerBaseType(const TypeBase& highestType);
i64 getArrayAlignment(i64 size, Type type);

} // Parsing