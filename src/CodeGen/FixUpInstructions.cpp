#include "FixUpInstructions.hpp"
#include "DynCast.hpp"
#include "Operators.hpp"

namespace CodeGen {

void FixUpInstructions::fixStackAlignment()
{
    if (0 < -stackAlloc) {
        i32 allocationSize = -stackAlloc;
        allocationSize += 16 - allocationSize % 16;
        copy.emplace_back(std::make_unique<BinaryInst>(
            program.getImmOperand(allocationSize, asmLongWord),
            program.getRegisterOperand(RegType::SP, asmQuadWord),
            BinaryInst::Operator::Sub, asmQuadWord));
    }
}

void FixUpInstructions::fixUp()
{
    using Inst = Inst::Kind;
    copy.reserve(insts.size() * 3);
    fixStackAlignment();
    while (!insts.empty()) {
        auto inst = std::move(insts.front());
        insts.erase(insts.begin());
        switch (inst->kind) {
            case Inst::Move: {
                const auto move = dynCast<MoveInst>(inst.get());
                fixMove(*move);
                break;
            }
            case Inst::MoveSX: {
                const auto moveSX = dynCast<MoveSXInst>(inst.get());
                fixMoveSX(*moveSX);
                break;
            }
            case Inst::MoveZeroExtend: {
                const auto moveZeroExtend = dynCast<MoveZeroExtendInst>(inst.get());
                fixMoveZero(*moveZeroExtend);
                break;
            }
            case Inst::Lea: {
                const auto lea = dynCast<LeaInst>(inst.get());
                fixLea(*lea);
                break;
            }
            case Inst::Binary:
                fixBinary(*dynCast<BinaryInst>(inst.get()));
                break;
            case Inst::Cmp: {
                const auto cmp = dynCast<CmpInst>(inst.get());
                fixCmp(*cmp);
                break;
            }
            case Inst::Idiv: {
                const auto idivInst = dynCast<IdivInst>(inst.get());
                fixIdiv(*idivInst);
                break;
            }
            case Inst::Div: {
                const auto div = dynCast<DivInst>(inst.get());
                fixDiv(*div);
                break;
            }
            case Inst::Cvttsd2si: {
                const auto cvttsd2si = dynCast<Cvttsd2siInst>(inst.get());
                fixCvttsd2si(*cvttsd2si);
                break;
            }
            case Inst::Cvtsi2sd: {
                const auto cvtsi2sd = dynCast<Cvtsi2sdInst>(inst.get());
                fixCvtsi2sd(*cvtsi2sd);
                break;
            }
            case Inst::PushPseudo:
                break;
            default:
                copy.emplace_back(std::move(inst));
        }
    }
    insts.swap(copy);
}

void FixUpInstructions::fixMove(MoveInst& moveInst)
{
    if (areBothOnTheStack(moveInst)) {
         const Operand* src = genSrcOperand(moveInst.type);
        insert(std::make_unique<MoveInst>(moveInst.src, src, moveInst.type));
        insert(std::make_unique<MoveInst>(src, moveInst.dst, moveInst.type));
        return;
    }
    insert(std::make_unique<MoveInst>(moveInst));
}

void FixUpInstructions::fixMoveSX(MoveSXInst& moveSX)
{
     const Operand* src = moveSX.src;
    if (src->kind == Operand::Kind::Imm) {
        insert(std::make_unique<MoveInst>(
            src, genSrcOperand(asmLongWord), asmLongWord));
        src = genSrcOperand(asmLongWord);
    }
    if (isOnTheStack(moveSX.dst->kind)) {
         const Operand* dst = genDstOperand(asmQuadWord);
        insert(std::make_unique<MoveSXInst>(src, dst, src->type, dst->type));
        insert(std::make_unique<MoveInst>(dst, moveSX.dst, asmQuadWord));
        return;
    }
    insert(std::make_unique<MoveSXInst>(src, moveSX.dst, src->type, moveSX.dst->type));
}

void FixUpInstructions::fixMoveZero(MoveZeroExtendInst& moveZero)
{
    if (moveZero.src->type == asmByte) {
        insert(std::make_unique<MoveInst>(moveZero.src, genSrcOperand(moveZero.srcType), moveZero.srcType));
        insert(std::make_unique<MoveZeroExtendInst>(
            genSrcOperand(moveZero.srcType), genDstOperand(moveZero.dstType),
            moveZero.srcType, moveZero.dstType));
        insert(std::make_unique<MoveInst>(genDstOperand(moveZero.dstType), moveZero.dst, moveZero.dstType));
        return;
    }
    if (moveZero.dst->kind == Operand::Kind::Register) {
        insert(std::make_unique<MoveInst>(moveZero.src, moveZero.dst, moveZero.dstType));
        return;
    }
    insert(std::make_unique<MoveInst>(moveZero.src, genDstOperand(moveZero.srcType), moveZero.srcType));
    insert(std::make_unique<MoveInst>(genDstOperand(moveZero.dstType), moveZero.dst, moveZero.dstType));
}

void FixUpInstructions::fixLea(LeaInst& lea)
{
    if (isOnTheStack(lea.dst->kind)) {
         const Operand* dst = genDstOperand(asmQuadWord);
        insert(std::make_unique<LeaInst>(lea.src, dst, asmQuadWord));
        insert(std::make_unique<MoveInst>(dst, lea.dst, asmQuadWord));
        return;
    }
    insert(std::make_unique<LeaInst>(lea));
}

void FixUpInstructions::fixBinary(BinaryInst& binary)
{
    if (isBinaryShift(binary))
        binaryShift(binary);
    else if (binary.oper == BinaryInst::Operator::Mul)
        binaryMul(binary);
    else if (binary.type == asmDouble)
        binaryDoubleOthers(binary);
    else
        binaryOthers(binary);
}

void FixUpInstructions::binaryShift(BinaryInst& binaryInst)
{
    const Operand* regCX = program.getRegisterOperand(RegType::CX, binaryInst.type);
    const Operand* regCL = program.getRegisterOperand(RegType::CX, asmByte);

    insert(std::make_unique<MoveInst>(binaryInst.lhs, regCX, binaryInst.type));
    insert(std::make_unique<BinaryInst>(regCL, binaryInst.rhs, binaryInst.oper, binaryInst.type));
}

void FixUpInstructions::binaryMul(BinaryInst& binaryInst)
{
    if (isOnTheStack(binaryInst.rhs->kind)) {
         const Operand* dst = genDstOperand(binaryInst.type);
        insert(std::make_unique<MoveInst>(binaryInst.rhs, dst, binaryInst.type));
        insert(std::make_unique<BinaryInst>(binaryInst.lhs, dst, binaryInst.oper, binaryInst.type));
        insert(std::make_unique<MoveInst>(dst, binaryInst.rhs, binaryInst.type));
        return;
    }
    insert(std::make_unique<BinaryInst>(binaryInst));
}

void FixUpInstructions::binaryDoubleOthers(BinaryInst& binaryInst)
{
    if (binaryInst.rhs->kind == Operand::Kind::Register) {
        insert(std::make_unique<BinaryInst>(binaryInst));
        return;
    }
     const Operand* dst = genDstOperand(binaryInst.type);
    insert(std::make_unique<MoveInst>(binaryInst.rhs, dst, binaryInst.type));
    insert(std::make_unique<BinaryInst>(binaryInst.lhs, dst, binaryInst.oper, binaryInst.type));
    insert(std::make_unique<MoveInst>(dst, binaryInst.rhs, binaryInst.type));
}

void FixUpInstructions::binaryOthers(BinaryInst& binaryInst)
{
    if (areBothOnTheStack(binaryInst)) {
         const Operand* src = genSrcOperand(binaryInst.type);
        insert(std::make_unique<MoveInst>(binaryInst.lhs, src, binaryInst.type));
        insert(std::make_unique<BinaryInst>(src, binaryInst.rhs, binaryInst.oper, binaryInst.type));
        return;
    }
    insert(std::make_unique<BinaryInst>(binaryInst));
}

void FixUpInstructions::fixCmp(CmpInst& cmpInst)
{
    if (cmpInst.rhs->kind != Operand::Kind::Register && cmpInst.type == asmDouble) {
         const Operand* dst = genDstOperand(cmpInst.type);
        insert(std::make_unique<MoveInst>(cmpInst.rhs, dst, cmpInst.type));
        insert(std::make_unique<CmpInst>(cmpInst.lhs, dst, cmpInst.type));
    } else if (cmpInst.rhs->kind == Operand::Kind::Imm) {
         const Operand* dst = genDstOperand(cmpInst.type);
        insert(std::make_unique<MoveInst>(cmpInst.rhs, dst, cmpInst.type));
        insert(std::make_unique<CmpInst>(cmpInst.lhs, dst, cmpInst.type));
    } else if (areBothOnTheStack(cmpInst)) {
         const Operand* src = genSrcOperand(cmpInst.type);
        insert(std::make_unique<MoveInst>(cmpInst.lhs, src, cmpInst.type));
        insert(std::make_unique<CmpInst>(src, cmpInst.rhs, cmpInst.type));
    } else
        insert(std::make_unique<CmpInst>(cmpInst));
}

void FixUpInstructions::fixIdiv(IdivInst& idiv)
{
    if (isOnTheStack(idiv.operand->kind) || idiv.operand->kind == Operand::Kind::Imm) {
         const Operand* src = genSrcOperand(idiv.type);
        insert(std::make_unique<MoveInst>(idiv.operand, src, idiv.type));
        insert(std::make_unique<IdivInst>(src, idiv.type));
        return;
    }
    insert(std::make_unique<IdivInst>(idiv));
}

void FixUpInstructions::fixDiv(DivInst& div)
{
    if (isOnTheStack(div.operand->kind) || div.operand->kind == Operand::Kind::Imm) {
         const Operand* src = genSrcOperand(div.type);
        insert(std::make_unique<MoveInst>(div.operand, src, div.type));
        insert(std::make_unique<DivInst>(src, div.type));
        return;
    }
    insert(std::make_unique<DivInst>(div));
}

void FixUpInstructions::fixCvttsd2si(Cvttsd2siInst& cvttsd2si)
{
    if (cvttsd2si.dst->kind == Operand::Kind::Register) {
        insert(std::make_unique<Cvttsd2siInst>(cvttsd2si));
        return;
    }
     const Operand* dst = genDstOperand(cvttsd2si.dstType);
    insert(std::make_unique<Cvttsd2siInst>(cvttsd2si.src, dst, cvttsd2si.dstType));
    insert(std::make_unique<MoveInst>(dst, cvttsd2si.dst, cvttsd2si.dstType));
}

void FixUpInstructions::fixCvtsi2sd(Cvtsi2sdInst& cvtsi2sd)
{
     const Operand* src = cvtsi2sd.src;
    if (src->kind == Operand::Kind::Imm) {
         const Operand* srcReg = genSrcOperand(cvtsi2sd.srcType);
        insert(std::make_unique<MoveInst>(src, srcReg, cvtsi2sd.srcType));
        src = srcReg;
    }
    if (cvtsi2sd.dst->kind == Operand::Kind::Register) {
        insert(std::make_unique<Cvtsi2sdInst>(src, cvtsi2sd.dst, asmDouble));
        return;
    }
    const Operand* dst = genDstOperand(asmDouble);
    insert(std::make_unique<Cvtsi2sdInst>(src, dst, cvtsi2sd.srcType));
    insert(std::make_unique<MoveInst>(dst, cvtsi2sd.dst, asmDouble));
}

const Operand* FixUpInstructions::genSrcOperand(const AsmType type) const
{
    if (type == asmDouble)
        return program.getRegisterOperand(RegType::XMM14, type);
    return program.getRegisterOperand(RegType::R10, type);
}

const Operand* FixUpInstructions::genDstOperand(const AsmType type) const
{
    if (type == asmDouble)
        return program.getRegisterOperand(RegType::XMM15, type);
    return program.getRegisterOperand(RegType::R11, type);
}

} // namespace CodeGen