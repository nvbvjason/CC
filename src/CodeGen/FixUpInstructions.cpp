#include "FixUpInstructions.hpp"
#include "DynCast.hpp"

namespace CodeGen {

void FixUpInstructions::fixStackAlignment()
{
    if (0 < -stackAlloc) {
        i32 allocationSize = -stackAlloc;
        allocationSize += 16 - allocationSize % 16;
        m_copy.emplace_back(std::make_unique<BinaryInst>(
            std::make_shared<ImmOperand>(allocationSize, AsmType::LongWord),
            std::make_shared<RegisterOperand>(RegType::SP, AsmType::QuadWord),
            BinaryInst::Operator::Sub, AsmType::QuadWord));
    }
}

void FixUpInstructions::fixUp()
{
    using Inst = Inst::Kind;
    m_copy.reserve(m_insts.size() * 3 + 1);
    fixStackAlignment();
    while (!m_insts.empty()) {
        auto inst = std::move(m_insts.front());
        m_insts.erase(m_insts.begin());
        switch (inst->kind) {
            case Inst::Move:
                fixMove(*dynCast<MoveInst>(inst.get()));
                break;
            case Inst::MoveSX:
                fixMoveSX(*dynCast<MoveSXInst>(inst.get()));
                break;
            case Inst::MoveZeroExtend:
                fixMoveZero(*dynCast<MoveZeroExtendInst>(inst.get()));
                break;
            case Inst::Lea:
                fixLea(*dynCast<LeaInst>(inst.get()));
                break;
            case Inst::Binary:
                fixBinary(*dynCast<BinaryInst>(inst.get()));
                break;
            case Inst::Cmp:
                fixCmp(*dynCast<CmpInst>(inst.get()));
                break;
            case Inst::Idiv:
                fixIdiv(*dynCast<IdivInst>(inst.get()));
                break;
            case Inst::Div:
                fixDiv(*dynCast<DivInst>(inst.get()));
                break;
            case Inst::Cvttsd2si:
                fixCvttsd2si(*dynCast<Cvttsd2siInst>(inst.get()));
                break;
            case Inst::Cvtsi2sd:
                fixCvtsi2sd(*dynCast<Cvtsi2sdInst>(inst.get()));
                break;
            case Inst::PushPseudo:
                break;
            default:
                m_copy.emplace_back(std::move(inst));
        }
    }
    m_insts.swap(m_copy);
}

void FixUpInstructions::fixMove(MoveInst& moveInst)
{
    if (areBothOnTheStack(moveInst)) {
        std::shared_ptr<Operand> src = genSrcOperand(moveInst.type);
        insert(std::make_unique<MoveInst>(moveInst.src, src, moveInst.type));
        insert(std::make_unique<MoveInst>(src, moveInst.dst, moveInst.type));
        return;
    }
    insert(std::make_unique<MoveInst>(moveInst));
}

void FixUpInstructions::fixMoveSX(MoveSXInst& moveSX)
{
    std::shared_ptr<Operand> src = moveSX.src;
    if (src->kind == Operand::Kind::Imm) {
        insert(std::make_unique<MoveInst>(
            src, genSrcOperand(AsmType::LongWord), AsmType::LongWord));
        src = genSrcOperand(AsmType::LongWord);
    }
    if (isOnTheStack(moveSX.dst->kind)) {
        std::shared_ptr<Operand> dst = genDstOperand(AsmType::QuadWord);
        insert(std::make_unique<MoveSXInst>(src, dst, src->type, dst->type));
        insert(std::make_unique<MoveInst>(dst, moveSX.dst, AsmType::QuadWord));
        return;
    }
    insert(std::make_unique<MoveSXInst>(src, moveSX.dst, src->type, moveSX.dst->type));
}

void FixUpInstructions::fixMoveZero(MoveZeroExtendInst& moveZero)
{
    if (moveZero.dst->kind == Operand::Kind::Register) {
        insert(std::make_unique<MoveInst>(moveZero.src, moveZero.dst, AsmType::QuadWord));
        return;
    }
    insert(std::make_unique<MoveInst>(moveZero.src, genDstOperand(AsmType::LongWord), AsmType::LongWord));
    insert(std::make_unique<MoveInst>(genDstOperand(AsmType::QuadWord), moveZero.dst, AsmType::QuadWord));
}

void FixUpInstructions::fixLea(LeaInst& lea)
{
    if (isOnTheStack(lea.dst->kind)) {
        std::shared_ptr<Operand> dst = genDstOperand(AsmType::QuadWord);
        insert(std::make_unique<LeaInst>(lea.src, dst, AsmType::QuadWord));
        insert(std::make_unique<MoveInst>(dst, lea.dst, AsmType::QuadWord));
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
    else if (binary.type == AsmType::Double)
        binaryDoubleOthers(binary);
    else
        binaryOthers(binary);
}

void FixUpInstructions::binaryShift(BinaryInst& binaryInst)
{
    auto regCX = std::make_shared<RegisterOperand>(RegType::CX, binaryInst.type);
    auto regCL = std::make_shared<RegisterOperand>(RegType::CX, AsmType::Byte);

    insert(std::make_unique<MoveInst>(binaryInst.lhs, regCX, binaryInst.type));
    insert(std::make_unique<BinaryInst>(regCL, binaryInst.rhs, binaryInst.oper, binaryInst.type));
}

void FixUpInstructions::binaryMul(BinaryInst& binaryInst)
{
    if (isOnTheStack(binaryInst.rhs->kind)) {
        std::shared_ptr<Operand> dst = genDstOperand(binaryInst.type);
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
    std::shared_ptr<Operand> dst = genDstOperand(binaryInst.type);
    insert(std::make_unique<MoveInst>(binaryInst.rhs, dst, binaryInst.type));
    insert(std::make_unique<BinaryInst>(binaryInst.lhs, dst, binaryInst.oper, binaryInst.type));
    insert(std::make_unique<MoveInst>(dst, binaryInst.rhs, binaryInst.type));
}

void FixUpInstructions::binaryOthers(BinaryInst& binaryInst)
{
    if (areBothOnTheStack(binaryInst)) {
        std::shared_ptr<Operand> src = genSrcOperand(binaryInst.type);
        insert(std::make_unique<MoveInst>(binaryInst.lhs, src, binaryInst.type));
        insert(std::make_unique<BinaryInst>(src, binaryInst.rhs, binaryInst.oper, binaryInst.type));
        return;
    }
    insert(std::make_unique<BinaryInst>(binaryInst));
}

void FixUpInstructions::fixCmp(CmpInst& cmpInst)
{
    if (cmpInst.rhs->kind != Operand::Kind::Register && cmpInst.type == AsmType::Double) {
        std::shared_ptr<Operand> dst = genDstOperand(cmpInst.type);
        insert(std::make_unique<MoveInst>(cmpInst.rhs, dst, cmpInst.type));
        insert(std::make_unique<CmpInst>(cmpInst.lhs, dst, cmpInst.type));
    } else if (cmpInst.rhs->kind == Operand::Kind::Imm) {
        std::shared_ptr<Operand> dst = genDstOperand(cmpInst.type);
        insert(std::make_unique<MoveInst>(cmpInst.rhs, dst, cmpInst.type));
        insert(std::make_unique<CmpInst>(cmpInst.lhs, dst, cmpInst.type));
    } else if (areBothOnTheStack(cmpInst)) {
        std::shared_ptr<Operand> src = genSrcOperand(cmpInst.type);
        insert(std::make_unique<MoveInst>(cmpInst.lhs, src, cmpInst.type));
        insert(std::make_unique<CmpInst>(src, cmpInst.rhs, cmpInst.type));
    } else
        insert(std::make_unique<CmpInst>(cmpInst));
}

void FixUpInstructions::fixIdiv(IdivInst& idiv)
{
    if (isOnTheStack(idiv.operand->kind) || idiv.operand->kind == Operand::Kind::Imm) {
        std::shared_ptr<Operand> src = genSrcOperand(idiv.type);
        insert(std::make_unique<MoveInst>(idiv.operand, src, idiv.type));
        insert(std::make_unique<IdivInst>(src, idiv.type));
        return;
    }
    insert(std::make_unique<IdivInst>(idiv));
}

void FixUpInstructions::fixDiv(DivInst& div)
{
    if (isOnTheStack(div.operand->kind) || div.operand->kind == Operand::Kind::Imm) {
        std::shared_ptr<Operand> src = genSrcOperand(div.type);
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
    std::shared_ptr<Operand> dst = genDstOperand(cvttsd2si.dstType);
    insert(std::make_unique<Cvttsd2siInst>(cvttsd2si.src, dst, cvttsd2si.dstType));
    insert(std::make_unique<MoveInst>(dst, cvttsd2si.dst, cvttsd2si.dstType));
}

void FixUpInstructions::fixCvtsi2sd(Cvtsi2sdInst& cvtsi2sd)
{
    std::shared_ptr<Operand> src = cvtsi2sd.src;
    if (src->kind == Operand::Kind::Imm) {
        std::shared_ptr<Operand> srcReg = genSrcOperand(cvtsi2sd.srcType);
        insert(std::make_unique<MoveInst>(src, srcReg, cvtsi2sd.srcType));
        src = srcReg;
    }
    if (cvtsi2sd.dst->kind == Operand::Kind::Register) {
        insert(std::make_unique<Cvtsi2sdInst>(src, cvtsi2sd.dst, AsmType::Double));
        return;
    }
    std::shared_ptr<Operand> dst = genDstOperand(AsmType::Double);
    insert(std::make_unique<Cvtsi2sdInst>(src, dst, cvtsi2sd.srcType));
    insert(std::make_unique<MoveInst>(dst, cvtsi2sd.dst, AsmType::Double));
}

std::shared_ptr<RegisterOperand> FixUpInstructions::genSrcOperand(AsmType type)
{
    if (type == AsmType::Double)
        return std::make_shared<RegisterOperand>(RegType::XMM14, type);
    return std::make_shared<RegisterOperand>(RegType::R10, type);
}

std::shared_ptr<RegisterOperand> FixUpInstructions::genDstOperand(AsmType type)
{
    if (type == AsmType::Double)
        return std::make_shared<RegisterOperand>(RegType::XMM15, type);
    return std::make_shared<RegisterOperand>(RegType::R11, type);
}

} // namespace CodeGen