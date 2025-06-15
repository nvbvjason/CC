#pragma once

#ifndef CC_CODEGEN_FIX_UP_INSTRUCTIONS_HPP
#define CC_CODEGEN_FIX_UP_INSTRUCTIONS_HPP

#include "AsmAST.hpp"

#include <vector>

namespace CodeGen {

class FixUpInstructions final : public InstVisitor {
    std::vector<std::unique_ptr<Inst>>& m_insts;
    std::vector<std::unique_ptr<Inst>> m_copy;
    i32 stackAlloc;

public:
    FixUpInstructions(std::vector<std::unique_ptr<Inst>>& insts, const i32 stackAlloc)
        : m_insts(insts), stackAlloc(stackAlloc) {}

    void visit(MoveInst& moveInst) override;
    void visit(MoveSXInst& moveSXInst) override;
    void visit(MoveZeroExtendInst& moveZero) override;
    void visit(BinaryInst& binary) override;
    void visit(CmpInst& cmpInst) override;
    void visit(IdivInst& idivInst) override;
    void visit(DivInst& div) override;
    void visit(Cvttsd2siInst& cvttsd2si) override;
    void visit(Cvtsi2sdInst& cvtsi2sd) override;

    void visit(PushInst&) override {}
    void visit(CallInst&) override {}
    void visit(UnaryInst&) override {}
    void visit(SetCCInst&) override {}
    void visit(CdqInst&) override {}
    void visit(JmpInst&) override {}
    void visit(JmpCCInst&) override {}
    void visit(LabelInst&) override {}
    void visit(ReturnInst&) override {}

    void fixUp();
private:
    template<typename... InstPtrs>
    void insert(InstPtrs&&... others) {
        (m_copy.push_back(std::forward<InstPtrs>(others)), ...);
    }

    void binaryShift(BinaryInst& binaryInst);
    void binaryMul(BinaryInst& binaryInst);
    void binaryDoubleOthers(BinaryInst& binaryInst);
    void binaryOthers(BinaryInst& binaryInst);
    std::shared_ptr<RegisterOperand> genSrcOperand(AsmType type);
    std::shared_ptr<RegisterOperand> genDstOperand(AsmType type);

    static inline bool isBinaryShift(const BinaryInst& binaryInst);
    static inline bool areBothOnTheStack(const MoveInst& moveInst);
    static inline bool areBothOnTheStack(const CmpInst& cmpInst);
};

inline bool FixUpInstructions::isBinaryShift(const BinaryInst& binaryInst)
{
    return binaryInst.oper == BinaryInst::Operator::LeftShiftSigned ||
           binaryInst.oper == BinaryInst::Operator::RightShiftSigned ||
           binaryInst.oper == BinaryInst::Operator::RightShiftUnsigned ||
           binaryInst.oper == BinaryInst::Operator::LeftShiftUnsigned;
}

inline bool FixUpInstructions::areBothOnTheStack(const MoveInst& moveInst)
{
    using Kind = Operand::Kind;
    return moveInst.src->kind == Kind::Data ||
           moveInst.dst->kind == Kind::Stack &&
           moveInst.src->kind == Kind::Stack ||
           moveInst.dst->kind == Kind::Data;
}

inline bool FixUpInstructions::areBothOnTheStack(const CmpInst& cmpInst)
{
    using Kind = Operand::Kind;
    return cmpInst.lhs->kind == Kind::Data ||
           cmpInst.rhs->kind == Kind::Stack &&
           cmpInst.lhs->kind == Kind::Stack ||
           cmpInst.rhs->kind == Kind::Data;
}

} // namespace CodeGen

#endif // CC_CODEGEN_FIX_UP_INSTRUCTIONS_HPP