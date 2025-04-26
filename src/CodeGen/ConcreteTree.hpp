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
Binary(Divide, src1, src2, dst)      Mov(src1, Reg(AX))
                                     Cdq
                                     Idiv(src2)
                                     Mov(Reg(AX), dst)
Binary(Remainder, src1, src2, dst)   Mov(src1, Reg(AX))
                                     Cdq
                                     Idiv(src2)
                                     Mov(Reg(DX), dst)
Binary(binary_operator, src1,        Mov(src1, dst)
        src2, dst)                   Binary(binary_operator, src2, dst)


=== Arithmetic Operators ===

Complement              Not
Negate                  Neg
Add                     Add
Subtract                Sub
Multiply                Mult

=== Operands ===

Constant(int)           Imm(int)
Var(identifier)         Pseudo(identifier)

*/

namespace CodeGen {

void program(const Ir::Program &program, Program &programCodegen);
std::unique_ptr<Function> function(const Ir::Function *function);

void unaryInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary);

void binaryInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);
void binaryDivideInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);
void binaryRemainderInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);
void binaryOtherInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);

void returnInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::ReturnInst* inst);

UnaryInst::Operator unaryOperator(Ir::UnaryInst::Operation type);
BinaryInst::Operator binaryOperator(Ir::BinaryInst::Operation type);

std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value);

[[nodiscard]] i32 replacingPseudoRegisters(Program &programCodegen);
void fixUpInstructions(Program &programCodegen, i32 stackAlloc);

}

#endif // CC_CONCRETE_TREE_HPP