#pragma once

#include "ASTExpr.hpp"

namespace Semantics {

void assignTypeToArithmeticBinaryExpr(Parsing::BinaryExpr& binaryExpr);
bool canConvertToNullPtr(const Parsing::ConstExpr& constExpr);
bool canConvertToNullPtr(const Parsing::Expr& expr);
bool canConvertToPtr(const Parsing::ConstExpr& constExpr);
bool isBinaryComparison(const Parsing::BinaryExpr& binaryExpr);

} // Parsing