#pragma once

#ifndef CC_CONCRETE_TREE_HPP
#define CC_CONCRETE_TREE_HPP

#include "AsmAST.hpp"
//#include "Frontend/SymbolTable.hpp"
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
class GenerateAsmTree {
    std::vector<std::unique_ptr<Inst>> insts;
public:
    void generateProgram(const Ir::Program &program, Program &programCodegen);
    std::unique_ptr<TopLevel> generateTopLevel(const Ir::TopLevel& topLevel);
    std::unique_ptr<TopLevel> generateFunction(const Ir::Function& function);
    void transformInst(const std::unique_ptr<Ir::Instruction>& inst);
    void unaryInst(const Ir::UnaryInst& irUnary);
    void generateUnaryNotInst(const Ir::UnaryInst& irUnary);

    void binaryInst(const Ir::BinaryInst& irBinary);

    void generateSignExtendInst(const Ir::SignExtendInst& signExtend);
    void generateTruncateInst(const Ir::TruncateInst& truncate);
    void generateBinaryDivideInst(const Ir::BinaryInst& irBinary);
    void generateBinaryRemainderInst(const Ir::BinaryInst& irBinary);
    void generateBinaryCondInst(const Ir::BinaryInst& irBinary);
    void generateBinaryBasicInst(const Ir::BinaryInst& irBinary);

    void generateJumpInst(const Ir::JumpInst& irJump);
    void generateJumpIfZeroInst(const Ir::JumpIfZeroInst& jumpIfZero);
    void generateJumpIfNotZeroInst(const Ir::JumpIfNotZeroInst& jumpIfNotZero);
    void generateCopyInst(const Ir::CopyInst& type);
    void generateLabelInst(const Ir::LabelInst& irLabel);
    void returnInst(const Ir::ReturnInst& returnInst);

    void pushFunCallArgs(const Ir::FunCallInst& funcCall);
    void generateFunCallInst(const Ir::FunCallInst& funcCall);
    std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value);
};

std::unique_ptr<TopLevel> generateStaticVariable(const Ir::StaticVariable& staticVariable);

i32 getStackPadding(size_t numArgs);
UnaryInst::Operator unaryOperator(Ir::UnaryInst::Operation type);
BinaryInst::Operator binaryOperator(Ir::BinaryInst::Operation type);
BinaryInst::CondCode condCode(Ir::BinaryInst::Operation type);
AssemblyType getAssemblyType(Type type);


[[nodiscard]] i32 replacingPseudoRegisters(const Function& function);
void fixUpInstructions(Function& function, i32 stackAlloc);

inline AssemblyType getAssemblyType(Type type)
{
    if (type == Type::I32)
        return AssemblyType::LongWord;
    return AssemblyType::QuadWord;
}

std::string makeTemporaryPseudoName();

}

#endif // CC_CONCRETE_TREE_HPP