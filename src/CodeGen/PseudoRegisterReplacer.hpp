#pragma once

#include "AsmAST.hpp"

#include <unordered_map>

namespace CodeGen {

class PseudoRegisterReplacer final : public InstVisitor {
    Program& program;
    std::unordered_map<std::string, i64> pseudoMap;
    i64 stackPtr = 0;
public:
    explicit PseudoRegisterReplacer(Program& program)
        : program(program) {}

    PseudoRegisterReplacer() = delete;

    [[nodiscard]] i64 stackPointer() const { return stackPtr; }

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
    void fitTo8Alignment();
    void fitTo16Alignment();
    [[nodiscard]] const Operand* replaceIfPseudo(const Operand* operand);
};

std::tuple<ReferringTo, AsmType, bool, std::string, i64> getPseudoValues(const std::unique_ptr<Operand>& operand);
} // CodeGen