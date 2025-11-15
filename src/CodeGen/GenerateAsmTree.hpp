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
            if (std::isnan(a))
                return std::isnan(b);
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
    void genUIntToDoubleByte(const Ir::UIntToDoubleInst& uintToDouble);
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

    void genAddPtr(const Ir::AddPtrInst& addPtrInst);
    void genAddPtrConstIndex(const Ir::AddPtrInst& addPtrInst);
    void genAddPtrVariableIndex1_2_4_8(const Ir::AddPtrInst& addPtrInst);
    void genAddPtrVariableIndexAndOtherScale(const Ir::AddPtrInst& addPtrInst);

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
    void genCopyToOffSet(const Ir::CopyToOffsetInst& copyToOffset);
    void genAllocate(const Ir::AllocateInst& allocate);

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
    void emplaceUnary(const std::shared_ptr<Operand>& target, UnaryInst::Operator oper, const AsmType type)
    {
        insts.emplace_back(std::make_unique<UnaryInst>(target, oper, type));
    }
    void emplaceBinary(const std::shared_ptr<Operand>& left,
                       const std::shared_ptr<Operand>& right,
                       const BinaryInst::Operator oper,
                       const AsmType type)
    {
        insts.emplace_back(std::make_unique<BinaryInst>(left, right, oper, type));
    }
    void emplaceCvtsi2sd(const std::shared_ptr<Operand>& src,
                         const std::shared_ptr<Operand>& dst,
                         const AsmType type)
    {
        insts.emplace_back(std::make_unique<Cvtsi2sdInst>(src, dst, type));
    }
    void emplaceCvttsd2si(const std::shared_ptr<Operand>& src,
                          const std::shared_ptr<Operand>& dst,
                          const AsmType type)
    {
        insts.emplace_back(std::make_unique<Cvttsd2siInst>(src, dst, type));
    }
    void emplaceDiv(const std::shared_ptr<Operand>& src, const AsmType type)
    {
        insts.emplace_back(std::make_unique<DivInst>(src, type));
    }
    void emplaceCdq(const AsmType type)
    {
        insts.emplace_back(std::make_unique<CdqInst>(type));
    }
    void emplaceIdiv(const std::shared_ptr<Operand>& src, const AsmType type)
    {
        insts.emplace_back(std::make_unique<IdivInst>(src, type));
    }
    void emplaceMove(const std::shared_ptr<Operand>& src,
                     const std::shared_ptr<Operand>& dst,
                     const AsmType type)
    {
        insts.emplace_back(std::make_unique<MoveInst>(src, dst, type));
    }
    void emplaceMoveZeroExtend(const std::shared_ptr<Operand>& src,
                               const std::shared_ptr<Operand>& dst,
                               const AsmType srcType,
                               const AsmType dstType)
    {
        insts.emplace_back(std::make_unique<MoveZeroExtendInst>(src, dst, srcType, dstType));
    }
    void emplaceMoveSX(const std::shared_ptr<Operand>& src,
                       const std::shared_ptr<Operand>& dst,
                       const AsmType srcType,
                       const AsmType dstType)
    {
        insts.emplace_back(std::make_unique<MoveSXInst>(src, dst, srcType, dstType));
    }
    void emplacePushPseudo(const i64 size, const AsmType type, const std::string& iden)
    {
        insts.emplace_back(std::make_unique<PushPseudoInst>(size, 16, type, Identifier(iden)));
    }
    void emplacePush(const std::shared_ptr<Operand>& src)
    {
        insts.emplace_back(std::make_unique<PushInst>(src));
    }
    void emplaceLea(const std::shared_ptr<Operand>& src,
                    const std::shared_ptr<Operand>& dst,
                    const AsmType type)
    {
        insts.emplace_back(std::make_unique<LeaInst>(src, dst, type));
    }
    void emplaceCmp(const std::shared_ptr<Operand>& lhs,
                    const std::shared_ptr<Operand>& rhs,
                    const AsmType type)
    {
        insts.emplace_back(std::make_unique<CmpInst>(lhs, rhs, type));
    }
    void emplaceSetCC(BinaryInst::CondCode cond, const std::shared_ptr<Operand>& src)
    {
        insts.emplace_back(std::make_unique<SetCCInst>(cond, src));
    }
    void emplaceJmp(const Identifier& iden)
    {
        insts.emplace_back(std::make_unique<JmpInst>(iden));
    }
    void emplaceJmpCC(const Inst::CondCode cond, const Identifier& iden)
    {
        insts.emplace_back(std::make_unique<JmpCCInst>(cond, iden));
    }
    void emplaceLabel(const Identifier& iden)
    {
        insts.emplace_back(std::make_unique<LabelInst>(iden));
    }
    void emplaceCall(const Identifier& iden)
    {
        insts.emplace_back(std::make_unique<CallInst>(iden));
    }
    void emplaceReturn()
    {
        insts.emplace_back(std::make_unique<ReturnInst>());
    }
};

std::unique_ptr<TopLevel> genStaticVariable(const Ir::StaticVariable& staticVariable);
std::unique_ptr<TopLevel> genStaticArray(const Ir::StaticArray& staticArray);
std::unique_ptr<TopLevel> genStaticString(const Ir::StaticConstant& staticConstant);
u64 getSingleInitValue(Type type, const Ir::ValueConst* value);
i32 getStackPadding(size_t numArgs);

std::string makeTemporaryPseudoName();

} // CodeGen