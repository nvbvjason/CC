#pragma once

#include "AsmAST.hpp"
#include "ASTIr.hpp"

#include <cmath>
#include <unordered_map>

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

    std::unordered_map<double, std::string, DoubleHash, DoubleEqual> m_constantDoubles;
    using RegType = Operand::RegKind;
    std::vector<std::unique_ptr<Inst>> insts;
    Program m_programCodegen;
    std::vector<std::unique_ptr<TopLevel>> m_toplevel;
public:
    void genProgram(const Ir::Program &program, Program &programCodegen);
    [[nodiscard]] std::unique_ptr<TopLevel> genTopLevel(const Ir::TopLevel& topLevel);
    void genFunctionPushOntoStack(const Ir::Function& function, std::vector<bool> pushedIntoRegs);
    [[nodiscard]] std::unique_ptr<TopLevel> genFunction(const Ir::Function& function);
    [[nodiscard]] std::vector<bool> genFunctionPushIntoRegs(const Ir::Function& function);
    void genInst(const std::unique_ptr<Ir::Instruction>& inst);
    void genUnary(const Ir::UnaryInst& irUnary);
    void genUnaryBasic(const Ir::UnaryInst& irUnary);
    void genNegateDouble(const Ir::UnaryInst& irUnary);
    void genUnaryNot(const Ir::UnaryInst& irUnary);
    void genUnaryNotDouble(const Ir::UnaryInst& irUnary);
    void genUnaryNotInteger(const Ir::UnaryInst& irUnary);

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

    void genBinary(const Ir::BinaryInst& irBinary);
    void genBinaryDivide(const Ir::BinaryInst& irBinary);
    void genBinaryDivideDouble(const Ir::BinaryInst& irBinary);
    void genBinaryDivideSigned(const Ir::BinaryInst& irBinary);
    void genUnsignedBinaryDivide(const Ir::BinaryInst& irBinary);
    void genBinaryRemainder(const Ir::BinaryInst& irBinary);
    void genSignedBinaryRemainder(const Ir::BinaryInst& irBinary);
    void genUnsignedBinaryRemainder(const Ir::BinaryInst& irBinary);
    void genBinaryCond(const Ir::BinaryInst& irBinary);
    void genBinaryCondInteger(const Ir::BinaryInst& irBinary);
    void genBinaryCondDouble(const Ir::BinaryInst& irBinary);
    void genBinaryBasic(const Ir::BinaryInst& irBinary);
    void genBinaryShift(const Ir::BinaryInst& irBinary);

    void genJump(const Ir::JumpInst& irJump);
    void genJumpIfZero(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfZeroDouble(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfZeroInteger(const Ir::JumpIfZeroInst& jumpIfZero);
    void genJumpIfNotZero(const Ir::JumpIfNotZeroInst& jumpIfNotZero);
    void genJumpIfNotZeroDouble(const Ir::JumpIfNotZeroInst& jumpIfNotZero);
    void genJumpIfNotZeroInteger(const Ir::JumpIfNotZeroInst& jumpIfNotZero);
    void genCopy(const Ir::CopyInst& copy);
    void genGetAddress(const Ir::GetAddressInst& getAddress);
    void genLoad(const Ir::LoadInst& load);
    void genStore(const Ir::StoreInst& store);
    void genLabel(const Ir::LabelInst& irLabel);

    void genFunCall(const Ir::FunCallInst& funcCall);
    std::vector<bool> genFuncCallPushArgsRegs(const Ir::FunCallInst& funcCall);
    void genFunCallPushArgs(const Ir::FunCallInst& funcCall);

    std::shared_ptr<Operand> genDoubleLocalConst(double value, i32 alignment);
    std::shared_ptr<Operand> getOperandFromConstant(const std::shared_ptr<Ir::Value>& value);
    std::shared_ptr<Operand> genOperand(const std::shared_ptr<Ir::Value>& value);
    std::shared_ptr<Operand> getZeroOperand(AsmType type);

    static std::shared_ptr<ImmOperand> getImmOperandFromValue(const Ir::ValueConst& valueConst);

private:
    void zeroOutReg(const std::shared_ptr<RegisterOperand>& reg);
};

std::unique_ptr<TopLevel> genStaticVariable(const Ir::StaticVariable& staticVariable);
i32 getStackPadding(size_t numArgs);

[[nodiscard]] i32 replacingPseudoRegisters(const Function& function);
void fixUpInstructions(Function& function, i32 stackAlloc);
std::string makeTemporaryPseudoName();

} // CodeGen