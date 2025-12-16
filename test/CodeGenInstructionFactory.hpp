#pragma once

#include <memory>

#include "AsmAST.hpp"

namespace CodeGen {

class CodeGenInstructionFactory {
    using RegType = Operand::RegKind;
    using OperKind = Operand::Kind;
    using Kind = Inst::Kind;

    const RegType m_doubleSrc = RegType::XMM0;
    const RegType m_doubleDst = RegType::XMM1;
    const RegType m_integerSrc = RegType::R8;
    const RegType m_integerDst = RegType::R9;
public:
    CodeGenInstructionFactory(
        const RegType doubleSrc, const RegType doubleDst,
        const RegType integerSrc, const RegType integerDst
    )
        : m_doubleSrc(doubleSrc), m_doubleDst(doubleDst),
          m_integerSrc(integerSrc), m_integerDst(integerDst) {}
    CodeGenInstructionFactory() = default;

    std::unique_ptr<Inst> create(Inst::Kind kind, OperKind srcKind, OperKind dstKind, AsmType asmType);
    std::unique_ptr<Inst> create(Inst::Kind kind, OperKind src, OperKind dst);
    std::unique_ptr<Inst> createBinary(
        BinaryInst::Operator kind, AsmType asmType, OperKind srcKind, OperKind dstKind);
    std::unique_ptr<Operand> createOperand(OperKind kind, AsmType asmType);
};

}