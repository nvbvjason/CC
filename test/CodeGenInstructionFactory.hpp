#pragma once

#include <memory>

#include "AsmAST.hpp"

namespace CodeGen {

class CodeGenInstructionFactory {
    using RegType = Operand::RegKind;
    using OperKind = Operand::Kind;
    using Kind = Inst::Kind;

    const RegType doubleSrc = RegType::XMM0;
    const RegType doubleDst = RegType::XMM1;
    const RegType integerSrc = RegType::R8;
    const RegType integerDst = RegType::R9;
    std::vector<std::unique_ptr<Operand>> operands;
public:
    CodeGenInstructionFactory(
        const RegType doubleSrc, const RegType doubleDst,
        const RegType integerSrc, const RegType integerDst
    )
        : doubleSrc(doubleSrc),   doubleDst(doubleDst),
          integerSrc(integerSrc), integerDst(integerDst) {}
    CodeGenInstructionFactory() = default;

    std::unique_ptr<Inst> create(Inst::Kind kind, OperKind srcKind, OperKind dstKind, AsmType asmType);
    std::unique_ptr<Inst> create(Inst::Kind kind, OperKind src, OperKind dst);
    std::unique_ptr<Inst> createBinary(
        BinaryInst::Operator kind, AsmType asmType, OperKind srcKind, OperKind dstKind);
    const Operand* createOperand(OperKind kind, AsmType asmType);
};

}