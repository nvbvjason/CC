#pragma once

#include "AsmAST.hpp"

#include <unordered_map>

namespace CodeGen {

class PseudoRegisterReplacer final : public InstVisitor {
    std::unordered_map<std::string, i64> m_pseudoMap;
    i64 m_stackPtr = 0;
public:
    [[nodiscard]] i64 stackPointer() const { return m_stackPtr; }

    void visit(MoveInst& move) override;
    void visit(MoveSXInst& moveSX) override;
    void visit(MoveZeroExtendInst& moveZero) override;
    void visit(LeaInst& lea) override;
    void visit(UnaryInst& unary) override;
    void visit(BinaryInst& binary) override;
    void visit(IdivInst& idiv) override;
    void visit(DivInst& div) override;
    void visit(CmpInst& cmpInst) override;
    void visit(SetCCInst& setCCInst) override;
    void visit(PushPseudoInst&) override;
    void visit(PushInst& pushInst) override;
    void visit(Cvttsd2siInst& cvttsd2siInst) override;
    void visit(Cvtsi2sdInst& cvtsi2sdInst) override;

    void visit(CallInst&) override {}
    void visit(CdqInst&) override {}
    void visit(ReturnInst&) override {}
    void visit(JmpInst&) override {}
    void visit(JmpCCInst&) override {}
    void visit(LabelInst&) override {}
private:
    void fitToAlignment();
    void replaceIfPseudo(std::shared_ptr<Operand>& operand);
};

std::tuple<ReferingTo, AsmType, bool, std::string, i64> getPseudoValues(const std::shared_ptr<Operand>& operand);
} // CodeGen