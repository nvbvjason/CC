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

void program(const IR::Program &program, Program &programCodegen);
std::unique_ptr<Function> function(const IR::Function *function);

void returnInstruction(std::vector<Instruction>& instructions, const IR::Instruction &instruction);
void unaryInstruction(std::vector<Instruction>& instructions, const IR::Instruction &instruction);

UnaryOperator unaryOperator(IR::Unary::OperationType type);

Operand constantOperand(const IR::Value& value);
Operand varOperand(const IR::Value& value);

}

#endif // CC_CONCRETE_TREE_HPP