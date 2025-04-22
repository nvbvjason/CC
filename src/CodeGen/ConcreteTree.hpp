#pragma once

#ifndef CC_CONCRETE_TREE_HPP
#define CC_CONCRETE_TREE_HPP

#include "AbstractTree.hpp"
#include "IR/AbstractTree.hpp"

/*
Top-Level

Program(function_definition)  Program(function_definition)
Function(name, instructions)  Function(name, instructions)


Instructions

Return(val)                          Mov(val, Reg(AX))
                                     Ret
Unary(unary_operator, src, dst)      Mov(src, dst)
                                     Unary(unary_operator, dst)


Arithmetic Operators

Complement              Not
Negate                  Neg


Operands

Constant(int)           Imm(int)
Var(identifier)         Pseudo(identifier)

*/

namespace CodeGen {

void programCodegen(const IR::Program &program, Program &programCodegen);
std::unique_ptr<Function> functionCodegen(const IR::Function *function);

void returnInstructionCodegen(std::vector<Instruction>& instructions, const IR::Instruction &instruction);
void unaryInstructionCodegen(std::vector<Instruction>& instructions, const IR::Instruction &instruction);

UnaryOperator unaryOperatorCodegen(IR::UnaryOperationType type);

Operand constantOperandCodeGen(const IR::Value& value);
Operand varOperandCodeGen(const IR::Value& value);

}

#endif // CC_CONCRETE_TREE_HPP