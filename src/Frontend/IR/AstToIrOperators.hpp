#pragma once

#ifndef CC_IR_AST_TO_IR_OPERATORS_HPP
#define CC_IR_AST_TO_IR_OPERATORS_HPP

#include "ASTExpr.hpp"
#include "ASTIr.hpp"

namespace Ir {
bool isPostfixOp(Parsing::UnaryExpr::Operator oper);
bool isPrefixOp(Parsing::UnaryExpr::Operator oper);
BinaryInst::Operation getPostPrefixOperation(Parsing::UnaryExpr::Operator oper);
UnaryInst::Operation convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);
BinaryInst::Operation convertBinaryOperation(Parsing::BinaryExpr::Operator binaryOperation);

} // Ir
#endif // CC_IR_AST_TO_IR_OPERATORS_HPP