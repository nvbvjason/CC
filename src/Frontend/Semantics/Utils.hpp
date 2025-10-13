#pragma once

#include "ASTExpr.hpp"

namespace Semantics {

void assignTypeToArithmeticBinaryExpr(Parsing::BinaryExpr& binaryExpr);
std::unique_ptr<Parsing::Expr> deepCopy(const Parsing::Expr& expr);
bool canConvertToNullPtr(const Parsing::ConstExpr& constExpr);
bool canConvertToNullPtr(const Parsing::Expr& expr);
bool canConvertToPtr(const Parsing::ConstExpr& constExpr);
bool isBinaryComparison(const Parsing::BinaryExpr& binaryExpr);

} // Parsing