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
    void visit(BinaryInst& binary) override;
    void visit(CmpInst& cmp) override;
    void visit(IdivInst& idiv) override;
    void visit(DeallocStackInst&) override {}
    void visit(PushInst&) override {}
    void visit(CallInst&) override {}

    // Unchanged instructions handled in fixUp
    void visit(UnaryInst&) override {}
    void visit(SetCCInst&) override {}
    void visit(CdqInst&) override {}
    void visit(JmpInst&) override {}
    void visit(JmpCCInst&) override {}
    void visit(LabelInst&) override {}
    void visit(AllocStackInst&) override {}
    void visit(ReturnInst&) override {}

    void fixUp();
private:
    template<typename... InstPtrs>
    void insert(InstPtrs&&... others) {
        (m_copy.push_back(std::forward<InstPtrs>(others)), ...);
    }

    void binaryShift(BinaryInst& binaryInst);
    void binaryMul(BinaryInst& binaryInst);
    void binaryOthers(BinaryInst& binaryInst);

    static inline bool isBinaryShift(const BinaryInst& binaryInst);
    static inline bool areBothOnTheStack(MoveInst& moveInst);
    static inline bool areBothOnTheStack(CmpInst& cmpInst);
};

inline bool FixUpInstructions::isBinaryShift(const BinaryInst& binaryInst)
{
    return binaryInst.oper == BinaryInst::Operator::LeftShift ||
           binaryInst.oper == BinaryInst::Operator::RightShift;
}

inline bool FixUpInstructions::areBothOnTheStack(MoveInst& moveInst)
{
    return moveInst.src->kind == Operand::Kind::Stack &&
           moveInst.dst->kind == Operand::Kind::Stack;
}

inline bool FixUpInstructions::areBothOnTheStack(CmpInst& cmpInst)
{
    return cmpInst.lhs->kind == Operand::Kind::Stack &&
           cmpInst.rhs->kind == Operand::Kind::Stack;
}

} // namespace CodeGen

#endif // CC_CODEGEN_FIX_UP_INSTRUCTIONS_HPP
