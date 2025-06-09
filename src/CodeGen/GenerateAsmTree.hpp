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
    using RegType = RegisterOperand::Kind;
    std::vector<std::unique_ptr<Inst>> insts;
public:
    void generateProgram(const Ir::Program &program, Program &programCodegen);
    std::unique_ptr<TopLevel> generateTopLevel(const Ir::TopLevel& topLevel);
    std::unique_ptr<TopLevel> generateFunction(const Ir::Function& function);
    void transformInst(const std::unique_ptr<Ir::Instruction>& inst);
    void unaryInst(const Ir::UnaryInst& irUnary);
    void genUnaryNotInst(const Ir::UnaryInst& irUnary);

    void genBinaryInst(const Ir::BinaryInst& irBinary);

    void genZeroExtendInst(const Ir::ZeroExtendInst& zeroExtend);
    void genSignExtendInst(const Ir::SignExtendInst& signExtend);
    void genTruncateInst(const Ir::TruncateInst& truncate);
    void genBinaryDivideInst(const Ir::BinaryInst& irBinary);
    void genSignedBinaryDivideInst(const Ir::BinaryInst& irBinary);
    void genUnsignedBinaryDivideInst(const Ir::BinaryInst& irBinary);
    void genBinaryRemainderInst(const Ir::BinaryInst& irBinary);
    void genSignedBinaryRemainderInst(const Ir::BinaryInst& irBinary);
    void genUnsignedBinaryRemainderInst(const Ir::BinaryInst& irBinary);
    void genBinaryCondInst(const Ir::BinaryInst& irBinary);
    void genBinaryBasicInst(const Ir::BinaryInst& irBinary);

    void genJumpInst(const Ir::JumpInst& irJump);
    void genJumpIfZeroInst(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfNotZeroInst(const Ir::JumpIfNotZeroInst& jumpIfNotZero);
    void genCopyInst(const Ir::CopyInst& type);
    void genLabelInst(const Ir::LabelInst& irLabel);
    void returnInst(const Ir::ReturnInst& returnInst);

    void pushFunCallArgs(const Ir::FunCallInst& funcCall);
    void generateFunCallInst(const Ir::FunCallInst& funcCall);
    std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value);
};

std::unique_ptr<TopLevel> generateStaticVariable(const Ir::StaticVariable& staticVariable);

std::shared_ptr<ImmOperand> getZeroImmOfType(Type type);
i32 getStackPadding(size_t numArgs);
UnaryInst::Operator unaryOperator(Ir::UnaryInst::Operation type);
BinaryInst::Operator binaryOperator(Ir::BinaryInst::Operation type);
BinaryInst::CondCode condCode(Ir::BinaryInst::Operation oper, bool isSigned);
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