#include "CodeGenInstructionFactory.hpp"

#include "CodeGen/Operators.hpp"

namespace CodeGen {

std::unique_ptr<Inst> CodeGenInstructionFactory::create(
    const Inst::Kind kind, const OperKind srcKind, const OperKind dstKind, const AsmType asmType)
{
    const Operand* src = createOperand(srcKind, asmType);
    const Operand* dst = createOperand(dstKind, asmType);
    switch (kind) {
        case Kind::Move:
            return std::make_unique<MoveInst>(src, dst, asmType);
        case Kind::MoveZeroExtend:
            return std::make_unique<MoveZeroExtendInst>(src, dst, src->type, dst->type);
        case Kind::MoveSX:
            return std::make_unique<MoveSXInst>(src, dst, src->type, dst->type);
        case Kind::Lea:
            return std::make_unique<LeaInst>(src, dst, asmType);
        case Kind::Idiv:
            return std::make_unique<IdivInst>(src, asmType);
        case Kind::Div:
            return std::make_unique<DivInst>(src, asmType);
        case Kind::Cvttsd2si:
            return std::make_unique<Cvttsd2siInst>(src, dst, asmType);
        case Kind::Cvtsi2sd:
            return std::make_unique<Cvtsi2sdInst>(src, dst, asmType);
        case Kind::Cmp:
            return std::make_unique<CmpInst>(src, dst, asmType);
        default:
            std::abort();
    }
}

std::unique_ptr<Inst> CodeGenInstructionFactory::create(
    const Inst::Kind kind, const OperKind src, const OperKind dst)
{
    return create(kind, src, dst, asmLongWord);
}

std::unique_ptr<Inst> CodeGenInstructionFactory::createBinary(
    BinaryInst::Operator kind, AsmType asmType,
    const OperKind srcKind, const OperKind dstKind)
{
    const Operand* src = createOperand(srcKind, asmType);
    const Operand* dst = createOperand(dstKind, asmType);
    return std::make_unique<BinaryInst>(src, dst, kind, asmType);
}

const Operand* CodeGenInstructionFactory::createOperand(const OperKind kind, AsmType asmType)
{
    switch (kind) {
        case OperKind::Imm: {
            operands.emplace_back(std::make_unique<ImmOperand>(0l, asmType));
            break;
        }
        case OperKind::Register: {
            operands.emplace_back(std::make_unique<RegisterOperand>(RegType::R8, asmType));
            break;
        }
        case OperKind::Pseudo: {
            operands.emplace_back(std::make_unique<PseudoOperand>(
                Identifier("x"), ReferringTo::Local, asmType, false));
            break;
        }
        case OperKind::Memory: {
            operands.emplace_back(std::make_unique<MemoryOperand>(RegType::R8, 0, asmType));
            break;
        }
        case OperKind::Data: {
            operands.emplace_back(std::make_unique<DataOperand>(
                asmType, 0, Identifier("x"), false));
            break;
        }
    }
    return operands.back().get();
}
} // CodeGen