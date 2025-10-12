#pragma once

#include "AsmAST.hpp"

#include <vector>

namespace CodeGen {

class FixUpInstructions final {
    using RegType = Operand::RegKind;

    std::vector<std::unique_ptr<Inst>>& m_insts;
    std::vector<std::unique_ptr<Inst>> m_copy;
    i32 stackAlloc;
public:
    FixUpInstructions(std::vector<std::unique_ptr<Inst>>& insts, const i32 stackAlloc)
        : m_insts(insts), stackAlloc(stackAlloc) {}

    void fixMove(MoveInst& moveInst);
    void fixMoveSX(MoveSXInst& moveSX);
    void fixMoveZero(MoveZeroExtendInst& moveZero);
    void fixLea(LeaInst& lea);
    void fixBinary(BinaryInst& binary);
    void fixCmp(CmpInst& cmpInst);
    void fixIdiv(IdivInst& idiv);
    void fixDiv(DivInst& div);
    void fixCvttsd2si(Cvttsd2siInst& cvttsd2si);
    void fixCvtsi2sd(Cvtsi2sdInst& cvtsi2sd);

    void fixUp();

    static std::shared_ptr<RegisterOperand> genSrcOperand(AsmType type);
    static std::shared_ptr<RegisterOperand> genDstOperand(AsmType type);
private:
    template<typename... InstPtrs>
    void insert(InstPtrs&&... others)
    {
        (m_copy.push_back(std::forward<InstPtrs>(others)), ...);
    }

    void binaryShift(BinaryInst& binaryInst);
    void binaryMul(BinaryInst& binaryInst);
    void binaryDoubleOthers(BinaryInst& binaryInst);
    void binaryOthers(BinaryInst& binaryInst);

    static constexpr bool isBinaryShift(const BinaryInst& binaryInst);
    static constexpr bool areBothOnTheStack(const MoveInst& move);
    static constexpr bool areBothOnTheStack(const CmpInst& cmp);
    static constexpr bool isOnTheStack(Operand::Kind kind);
};

constexpr bool FixUpInstructions::isBinaryShift(const BinaryInst& binaryInst)
{
    using Operator = BinaryInst::Operator;
    return binaryInst.oper == Operator::LeftShiftSigned ||
           binaryInst.oper == Operator::RightShiftSigned ||
           binaryInst.oper == Operator::RightShiftUnsigned ||
           binaryInst.oper == Operator::LeftShiftUnsigned;
}

constexpr bool FixUpInstructions::areBothOnTheStack(const MoveInst& move)
{
    return isOnTheStack(move.src->kind) && isOnTheStack(move.dst->kind);
}

constexpr bool FixUpInstructions::areBothOnTheStack(const CmpInst& cmp)
{
    return isOnTheStack(cmp.lhs->kind) && isOnTheStack(cmp.rhs->kind);
}

constexpr bool FixUpInstructions::isOnTheStack(const Operand::Kind kind)
{
    using Kind = Operand::Kind;
    return kind == Kind::Data || kind == Kind::Memory;
}

} // namespace CodeGen