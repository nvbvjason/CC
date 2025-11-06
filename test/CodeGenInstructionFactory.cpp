#include "CodeGenInstructionFactory.hpp"

namespace CodeGen {

std::unique_ptr<Inst> CodeGenInstructionFactory::create(
    const Inst::Kind kind, const OperKind srcKind, const OperKind dstKind, const AsmType asmType)
{
    std::unique_ptr<Operand> src = createOperand(srcKind, asmType);
    std::unique_ptr<Operand> dst = createOperand(dstKind, asmType);
    switch (kind) {
        case Kind::Move:
            return std::make_unique<MoveInst>(std::move(src), std::move(dst), asmType);
        case Kind::MoveZeroExtend:
            return std::make_unique<MoveZeroExtendInst>(std::move(src), std::move(dst), asmType);
        case Kind::MoveSX:
            return std::make_unique<MoveSXInst>(std::move(src), std::move(dst));
        case Kind::Lea:
            return std::make_unique<LeaInst>(std::move(src), std::move(dst), asmType);
        case Kind::Idiv:
            return std::make_unique<IdivInst>(std::move(src), asmType);
        case Kind::Div:
            return std::make_unique<DivInst>(std::move(src), asmType);
        case Kind::Cvttsd2si:
            return std::make_unique<Cvttsd2siInst>(std::move(src), std::move(dst), asmType);
        case Kind::Cvtsi2sd:
            return std::make_unique<Cvtsi2sdInst>(std::move(src), std::move(dst), asmType);
        case Kind::Cmp:
            return std::make_unique<CmpInst>(std::move(src), std::move(dst), asmType);
        default:
            std::abort();
    }
}

std::unique_ptr<Inst> CodeGenInstructionFactory::create(
    const Inst::Kind kind, const OperKind src, const OperKind dst)
{
    return create(kind, src, dst, LongWordType());
}

std::unique_ptr<Inst> CodeGenInstructionFactory::createBinary(
    BinaryInst::Operator kind, AsmType asmType,
    const OperKind srcKind, const OperKind dstKind)
{
    std::unique_ptr<Operand> src = createOperand(srcKind, asmType);
    std::unique_ptr<Operand> dst = createOperand(dstKind, asmType);
    return std::make_unique<BinaryInst>(std::move(src), std::move(dst), kind, asmType);
}

std::unique_ptr<Operand> CodeGenInstructionFactory::createOperand(const OperKind kind, AsmType asmType)
{
    switch (kind) {
        case OperKind::Imm:
            return std::make_unique<ImmOperand>(0l, asmType);
        case OperKind::Register:
            return std::make_unique<RegisterOperand>(RegType::R8, asmType);
        case OperKind::Pseudo:
            return std::make_unique<PseudoOperand>(Identifier("x"), ReferingTo::Local, asmType, false);
        case OperKind::Memory:
            return std::make_unique<MemoryOperand>(RegType::R8, 0, asmType);
        case OperKind::Data:
            return std::make_unique<DataOperand>(Identifier("x"), asmType, false);
    }
    std::abort();
}
}