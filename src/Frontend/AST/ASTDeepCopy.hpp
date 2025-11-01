#pragma once

#include "ASTBase.hpp"

namespace Parsing {

[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const TypeBase& typeBase);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const VarType& typeBase);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const FuncType& funcType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const PointerType& pointerType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const ArrayType& arrayType);

[[nodiscard]] bool areEquivalent(const TypeBase& left, const TypeBase& right);
[[nodiscard]] bool areEquivalent(const VarType& left, const VarType& right);
[[nodiscard]] bool areEquivalent(const FuncType& left, const FuncType& right);
[[nodiscard]] bool areEquivalent(const PointerType& left, const PointerType& right);
[[nodiscard]] bool areEquivalent(const ArrayType& left, const ArrayType& right);
[[nodiscard]] bool areEquivalentArrayConversion(const TypeBase& left, const TypeBase& right);

[[nodiscard]] std::unique_ptr<Expr> deepCopy(const Expr& expr);
[[nodiscard]] std::unique_ptr<Expr> deepCopy(const ConstExpr& expr);
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

} // Parsing