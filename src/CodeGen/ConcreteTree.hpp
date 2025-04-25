#pragma once

#ifndef CC_CONCRETE_TREE_HPP
#define CC_CONCRETE_TREE_HPP

#include "AbstractTree.hpp"
#include "IR/AbstractTree.hpp"

/*
=== Top-Level ===

Program(function_definition)  Program(function_definition)
Function(name, instructions)  Function(name, instructions)


=== Instructions ===

Return(val)                          Mov(val, Reg(AX))
                                     Ret
Unary(unary_operator, src, dst)      Mov(src, dst)
                                     Unary(unary_operator, dst)


=== Arithmetic Operators ===

Complement              Not
Negate                  Neg


=== Operands ===

Constant(int)           Imm(int)
Var(identifier)         Pseudo(identifier)

*/

namespace CodeGen {

void program(const Ir::Program &program, Program &programCodegen);
std::unique_ptr<Function> function(const Ir::Function *function);

void unaryInst(std::vector<std::shared_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary);
void returnInst(std::vector<std::shared_ptr<Inst>>& insts, const Ir::ReturnInst* inst);

UnaryInst::Operator unaryOperator(Ir::UnaryInst::Operation type);

std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value);

}

#endif // CC_CONCRETE_TREE_HPP