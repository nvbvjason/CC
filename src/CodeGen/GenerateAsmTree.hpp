#pragma once

#ifndef CC_CONCRETE_TREE_HPP
#define CC_CONCRETE_TREE_HPP

#include "AsmAST.hpp"
#include "ASTIr.hpp"

#include <cmath>
#include <unordered_set>

namespace CodeGen {
class GenerateAsmTree {
    struct DoubleHash {
        size_t operator()(const double d) const noexcept {
            return std::hash<u64>{}(std::bit_cast<u64>(d));
        }
    };
    struct DoubleEqual {
        bool operator()(const double a, const double b) const noexcept {
            if (std::isnan(a)) return std::isnan(b);
            return std::bit_cast<u64>(a) == std::bit_cast<u64>(b);
        }
    };
    using RegType = RegisterOperand::Kind;
    std::vector<std::unique_ptr<Inst>> insts;
    std::unordered_set<double, DoubleHash, DoubleEqual> numbers;
public:
    void generateProgram(const Ir::Program &program, Program &programCodegen);
    std::unique_ptr<TopLevel> generateTopLevel(const Ir::TopLevel& topLevel);
    std::unique_ptr<TopLevel> generateFunction(const Ir::Function& function);
    void transformInst(const std::unique_ptr<Ir::Instruction>& inst);
    void unaryInst(const Ir::UnaryInst& irUnary);
    void genUnaryNotInst(const Ir::UnaryInst& irUnary);

    void genBinaryInst(const Ir::BinaryInst& irBinary);

    void returnInst(const Ir::ReturnInst& returnInst);
    void genSignExtendInst(const Ir::SignExtendInst& signExtend);
    void genTruncateInst(const Ir::TruncateInst& truncate);
    void genZeroExtendInst(const Ir::ZeroExtendInst& zeroExtend);

    void genDoubleToIntInst(const Ir::DoubleToIntInst& doubleToInt);
    void genDoubleToUIntInst(const Ir::DoubleToUIntInst& doubleToUInt);
    void genIntToDoubleInst(const Ir::IntToDoubleInst& intToDouble);
    void genUIntToDoubleInst(const Ir::UIntToDoubleInst& uintToDouble);

    void genBinaryDivideInst(const Ir::BinaryInst& irBinary);
    void genBinaryDivideDoubleInst(const Ir::BinaryInst& irBinary);
    void genSignedBinaryDivideInst(const Ir::BinaryInst& irBinary);
    void genUnsignedBinaryDivideInst(const Ir::BinaryInst& irBinary);
    void genBinaryRemainderInst(const Ir::BinaryInst& irBinary);
    void genSignedBinaryRemainderInst(const Ir::BinaryInst& irBinary);
    void genUnsignedBinaryRemainderInst(const Ir::BinaryInst& irBinary);
    void genBinaryCondInst(const Ir::BinaryInst& irBinary);
    void genBinaryBasicInst(const Ir::BinaryInst& irBinary);
    void genBinaryShiftInst(const Ir::BinaryInst& irBinary);

    void genJumpInst(const Ir::JumpInst& irJump);
    void genJumpIfZeroInst(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfZeroDoubleInst(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfZeroIntegerInst(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfNotZeroInst(const Ir::JumpIfNotZeroInst& jumpIfNotZero);
    void genCopyInst(const Ir::CopyInst& type);
    void genLabelInst(const Ir::LabelInst& irLabel);

    void pushFunCallArgs(const Ir::FunCallInst& funcCall);
    void generateFunCallInst(const Ir::FunCallInst& funcCall);
    std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value);
};

std::unique_ptr<TopLevel> generateStaticVariable(const Ir::StaticVariable& staticVariable);

std::shared_ptr<ImmOperand> getZeroImmOfType(AsmType type);
i32 getStackPadding(size_t numArgs);
UnaryInst::Operator unaryOperator(Ir::UnaryInst::Operation type);
BinaryInst::Operator binaryOperator(Ir::BinaryInst::Operation type);
BinaryInst::Operator getShiftOperator(Ir::BinaryInst::Operation type, bool isSigned);
BinaryInst::CondCode condCode(Ir::BinaryInst::Operation oper, bool isSigned);
AsmType getAssemblyType(Type type);


[[nodiscard]] i32 replacingPseudoRegisters(const Function& function);
void fixUpInstructions(Function& function, i32 stackAlloc);

inline AsmType getAssemblyType(Type type)
{
    if (type == Type::I32 || type == Type::U32)
        return AsmType::LongWord;
    return AsmType::QuadWord;
}

std::string makeTemporaryPseudoName();

}

#endif // CC_CONCRETE_TREE_HPP