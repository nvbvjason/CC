#pragma once

#ifndef CC_CONCRETE_TREE_HPP
#define CC_CONCRETE_TREE_HPP

#include "AsmAST.hpp"
#include "IR/ASTIr.hpp"

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

void generateProgram(const Ir::Program &program, Program &programCodegen);
std::unique_ptr<TopLevel> generateTopLevel(const Ir::TopLevel& topLevel);
std::unique_ptr<TopLevel> generateFunction(const Ir::Function& function);
std::unique_ptr<TopLevel> generateStaticVariable(const Ir::StaticVariable& staticVariable);
void transformInst(const std::unique_ptr<Function>& functionCodeGen, const std::unique_ptr<Ir::Instruction>& inst);
void unaryInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary);
void generateUnaryNotInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary);

void binaryInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);

void generateBinaryDivideInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);
void generateBinaryRemainderInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);
void generateBinaryCondInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);
void generateBinaryBasicInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);
void generateBinaryShiftInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary);

void generateJumpInst(std::vector<std::unique_ptr<Inst>>& insts,
                      const Ir::JumpInst* irJump);
void generateJumpIfZeroInst(std::vector<std::unique_ptr<Inst>>& insts,
                            const Ir::JumpIfZeroInst* irBinary);
void generateJumpIfNotZeroInst(std::vector<std::unique_ptr<Inst>>& insts,
                               const Ir::JumpIfNotZeroInst* irBinary);
void generateCopyInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::CopyInst* type);
void generateLabelInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::LabelInst* irLabel);
void returnInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::ReturnInst* inst);

i32 getStackPadding(size_t numArgs);
void pushFunCallArgs(std::vector<std::unique_ptr<Inst>>& insts, const Ir::FunCallInst* type);
void generateFunCallInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::FunCallInst* type);

UnaryInst::Operator unaryOperator(Ir::UnaryInst::Operation type);
BinaryInst::Operator binaryOperator(Ir::BinaryInst::Operation type);
BinaryInst::CondCode condCode(Ir::BinaryInst::Operation type);

std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value);

[[nodiscard]] i32 getStaticVariableInitial(const Ir::StaticVariable& staticVariable);

[[nodiscard]] i32 replacingPseudoRegisters(Function& function);
void fixUpInstructions(Function& function, i32 stackAlloc);

}

#endif // CC_CONCRETE_TREE_HPP