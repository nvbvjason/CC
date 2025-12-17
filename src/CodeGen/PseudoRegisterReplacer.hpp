#pragma once

#include "AsmAST.hpp"

#include <unordered_map>

namespace CodeGen {

class PseudoRegisterReplacer final {
    Program& program;
    std::unordered_map<std::string, i64> pseudoMap;
    i64 stackPtr = 0;
public:
    explicit PseudoRegisterReplacer(Program& program)
        : program(program) {}

    PseudoRegisterReplacer() = delete;

    [[nodiscard]] i64 stackPointer() const { return stackPtr; }

    void replace(Inst& inst);

    void replace(MoveInst& move);
    void replace(MoveSXInst& moveSX);
    void replace(MoveZeroExtendInst& moveZero);
    void replace(LeaInst& lea);
    void replace(UnaryInst& unary);
    void replace(BinaryInst& binary);
    void replace(IdivInst& idiv);
    void replace(DivInst& div);
    void replace(CmpInst& cmpInst);
    void replace(SetCCInst& setCCInst);
    void replace(const PushPseudoInst& pushPseudoInst);
    void replace(PushInst& pushInst);
    void replace(Cvttsd2siInst& cvttsd2siInst);
    void replace(Cvtsi2sdInst& cvtsi2sdInst);
private:
    void fitTo8Alignment();
    void fitTo16Alignment();
    [[nodiscard]] const Operand* replaceIfPseudo(const Operand* operand);
};

std::tuple<ReferringTo, AsmType, bool, std::string, i64> getPseudoValues(const std::unique_ptr<Operand>& operand);
} // CodeGen