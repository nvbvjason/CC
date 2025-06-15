#pragma once

#ifndef CC_CONCRETE_TREE_HPP
#define CC_CONCRETE_TREE_HPP

#include "AsmAST.hpp"
#include "ASTIr.hpp"

namespace CodeGen {
class GenerateAsmTree {
    using RegType = RegisterOperand::Kind;
    std::vector<std::unique_ptr<Inst>> insts;
    Program m_programCodegen;
    std::vector<std::unique_ptr<TopLevel>> m_toplevel;
public:
    void genProgram(const Ir::Program &program, Program &programCodegen);
    [[nodiscard]] std::unique_ptr<TopLevel> genTopLevel(const Ir::TopLevel& topLevel);
    [[nodiscard]] std::unique_ptr<TopLevel> genFunction(const Ir::Function& function);
    [[nodiscard]] std::vector<bool> genFunctionPushIntoRegs(const Ir::Function& function);
    void genInst(const std::unique_ptr<Ir::Instruction>& inst);
    void genUnary(const Ir::UnaryInst& irUnary);
    void genNegateDouble(const Ir::UnaryInst& irUnary);
    void genUnaryNot(const Ir::UnaryInst& irUnary);

    void genBinary(const Ir::BinaryInst& irBinary);

    void genReturn(const Ir::ReturnInst& returnInst);
    void genSignExtend(const Ir::SignExtendInst& signExtend);
    void genTruncate(const Ir::TruncateInst& truncate);
    void genZeroExtend(const Ir::ZeroExtendInst& zeroExtend);

    void genDoubleToInt(const Ir::DoubleToIntInst& doubleToInt);
    void genDoubleToUInt(const Ir::DoubleToUIntInst& doubleToUInt);
    void genDoubleToUIntLong(const Ir::DoubleToUIntInst& doubleToUInt);
    void genDoubleToUIntQuad(const Ir::DoubleToUIntInst& doubleToUInt);
    void genIntToDouble(const Ir::IntToDoubleInst& intToDouble);
    void genUIntToDouble(const Ir::UIntToDoubleInst& uintToDouble);
    void genUIntToDoubleLong(const Ir::UIntToDoubleInst& uintToDouble);
    void genUIntToDoubleQuad(const Ir::UIntToDoubleInst& uintToDouble);

    void genBinaryDivide(const Ir::BinaryInst& irBinary);
    void genBinaryDivideDouble(const Ir::BinaryInst& irBinary);
    void genBinaryDivideSigned(const Ir::BinaryInst& irBinary);
    void genUnsignedBinaryDivide(const Ir::BinaryInst& irBinary);
    void genBinaryRemainder(const Ir::BinaryInst& irBinary);
    void genSignedBinaryRemainder(const Ir::BinaryInst& irBinary);
    void genUnsignedBinaryRemainder(const Ir::BinaryInst& irBinary);
    void genBinaryCond(const Ir::BinaryInst& irBinary);
    void genBinaryBasic(const Ir::BinaryInst& irBinary);
    void genBinaryShift(const Ir::BinaryInst& irBinary);

    void genJump(const Ir::JumpInst& irJump);
    void genJumpIfZero(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfZeroDouble(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfZeroInteger(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfNotZero(const Ir::JumpIfNotZeroInst& jumpIfNotZero);
    void genJumpIfNotZeroDouble(const Ir::JumpIfNotZeroInst& jumpIfNotZero);
    void genJumpIfNotZeroInteger(const Ir::JumpIfNotZeroInst& jumpIfNotZero);
    void genCopy(const Ir::CopyInst& type);
    void genLabel(const Ir::LabelInst& irLabel);

    void genFunCall(const Ir::FunCallInst& funcCall);
    void genFunCallPushArgs(const Ir::FunCallInst& funcCall);
    std::shared_ptr<Operand> genOperand(const std::shared_ptr<Ir::Value>& value);
};

std::unique_ptr<TopLevel> genStaticVariable(const Ir::StaticVariable& staticVariable);

std::shared_ptr<ImmOperand> getZeroImmOfType(AsmType type);
i32 getStackPadding(size_t numArgs);
UnaryInst::Operator unaryOperator(Ir::UnaryInst::Operation type);
BinaryInst::Operator binaryOperator(Ir::BinaryInst::Operation type);
BinaryInst::Operator getShiftOperator(Ir::BinaryInst::Operation type, bool isSigned);
BinaryInst::CondCode condCode(Ir::BinaryInst::Operation oper, bool isSigned);
AsmType getAsmType(Type type);

[[nodiscard]] i32 replacingPseudoRegisters(const Function& function);
void fixUpInstructions(Function& function, i32 stackAlloc);

inline AsmType getAsmType(Type type)
{
    if (type == Type::I32 || type == Type::U32)
        return AsmType::LongWord;
    if (type == Type::I64 || type == Type::U64)
        return AsmType::QuadWord;
    return AsmType::Double;
}

std::string makeTemporaryPseudoName();

}

#endif // CC_CONCRETE_TREE_HPP