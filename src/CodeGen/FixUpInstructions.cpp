#include "FixUpInstructions.hpp"

namespace CodeGen {


void FixUpInstructions::fixUp()
{
    m_copy.reserve(m_insts.size() * 3 + 1);
    if (0 < -stackAlloc) {
        i32 allocationSize = -stackAlloc;
        allocationSize += 16 - allocationSize % 16;
        m_copy.emplace_back(std::make_unique<BinaryInst>(
            std::make_shared<ImmOperand>(allocationSize),
            std::make_shared<RegisterOperand>(RegisterOperand::Kind::SP, AsmType::QuadWord),
            BinaryInst::Operator::Sub, AsmType::QuadWord));
    }
    while (!m_insts.empty()) {
        auto inst = std::move(m_insts.front());
        m_insts.erase(m_insts.begin());
        if (inst->kind == Inst::Kind::Move)
            visit(*static_cast<MoveInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::MoveSX)
            visit(*static_cast<MoveSXInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::MovZeroExtend)
            visit(*static_cast<MoveZeroExtendInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::Binary)
            visit(*static_cast<BinaryInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::Cmp)
            visit(*static_cast<CmpInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::Idiv)
            visit(*static_cast<IdivInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::Div)
            visit(*static_cast<DivInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::Cvttsd2si)
            visit(*static_cast<DivInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::Cvtsi2sd)
            visit(*static_cast<DivInst*>(inst.get()));
        else
            m_copy.emplace_back(std::move(inst));
    }
    m_insts.swap(m_copy);
}

void FixUpInstructions::visit(MoveInst& moveInst)
{
    if (areBothOnTheStack(moveInst)) {
        auto first = std::make_unique<MoveInst>(moveInst.src, genSrcOperand(moveInst.type), moveInst.type);
        auto second = std::make_unique<MoveInst>(genSrcOperand(moveInst.type), moveInst.dst, moveInst.type);
        insert(std::move(first), std::move(second));
        return;
    }
    insert(std::make_unique<MoveInst>(moveInst));
}

void FixUpInstructions::visit(MoveSXInst& moveSXInst)
{
    auto first = std::make_unique<MoveInst>(moveSXInst.src, genSrcOperand(AsmType::LongWord), AsmType::LongWord);
    auto second = std::make_unique<MoveSXInst>(genSrcOperand(AsmType::LongWord), genDstOperand(AsmType::QuadWord));
    auto third = std::make_unique<MoveInst>(genDstOperand(AsmType::QuadWord), moveSXInst.dst, AsmType::QuadWord);
    insert(std::move(first), std::move(second), std::move(third));
}

void FixUpInstructions::visit(MoveZeroExtendInst& moveZero)
{
    if (moveZero.dst->kind == Operand::Kind::Register) {
        auto first = std::make_unique<MoveInst>(
            std::move(moveZero.src), std::move(moveZero.dst), AsmType::LongWord);
        insert(std::move(first));
        return;
    }
    auto first = std::make_unique<MoveInst>(moveZero.src, genDstOperand(AsmType::LongWord), AsmType::LongWord);
    auto reg11Q = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R11, AsmType::QuadWord);
    auto second = std::make_unique<MoveInst>( genDstOperand(AsmType::QuadWord), moveZero.dst, AsmType::QuadWord);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::visit(BinaryInst& binary)
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
    auto regCX = std::make_shared<RegisterOperand>(RegisterOperand::Kind::CX, binaryInst.type);
    auto regCL = std::make_shared<RegisterOperand>(RegisterOperand::Kind::CX, AsmType::Byte);
    auto first = std::make_unique<MoveInst>(binaryInst.lhs, regCX, binaryInst.type);
    auto second = std::make_unique<BinaryInst>(regCL, binaryInst.rhs, binaryInst.oper, binaryInst.type);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::binaryMul(BinaryInst& binaryInst)
{
    auto first = std::make_unique<MoveInst>(binaryInst.rhs, genDstOperand(binaryInst.type), binaryInst.type);
    auto second = std::make_unique<BinaryInst>(
        binaryInst.lhs, genDstOperand(binaryInst.type), binaryInst.oper, binaryInst.type);
    auto third = std::make_unique<MoveInst>(genDstOperand(binaryInst.type), binaryInst.rhs, binaryInst.type);
    insert(std::move(first), std::move(second), std::move(third));
}

void FixUpInstructions::binaryDoubleOthers(BinaryInst& binaryInst)
{
    if (binaryInst.rhs->kind != Operand::Kind::Register) {
        auto first = std::make_unique<MoveInst>(binaryInst.rhs, genDstOperand(binaryInst.type), binaryInst.type);
        auto second = std::make_unique<BinaryInst>(
            binaryInst.lhs, genDstOperand(binaryInst.type), binaryInst.oper, binaryInst.type);
        insert(std::move(first), std::move(second));
    }
    else
        insert(std::make_unique<BinaryInst>(binaryInst));
}

void FixUpInstructions::binaryOthers(BinaryInst& binaryInst)
{
    auto first = std::make_unique<MoveInst>(binaryInst.lhs, genSrcOperand(binaryInst.type), binaryInst.type);
    auto second = std::make_unique<BinaryInst>(
        genSrcOperand(binaryInst.type), binaryInst.rhs, binaryInst.oper, binaryInst.type);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::visit(CmpInst& cmpInst)
{
    if (cmpInst.rhs->kind != Operand::Kind::Register && cmpInst.type == AsmType::Double) {
        auto first = std::make_unique<MoveInst>(cmpInst.rhs, genDstOperand(cmpInst.type), cmpInst.type);
        auto second = std::make_unique<CmpInst>(cmpInst.lhs, genDstOperand(cmpInst.type), cmpInst.type);
        insert(std::move(first), std::move(second));
    }
    else if (areBothOnTheStack(cmpInst)) {
        auto first = std::make_unique<MoveInst>(cmpInst.lhs, genSrcOperand(cmpInst.type), cmpInst.type);
        auto second = std::make_unique<CmpInst>(genSrcOperand(cmpInst.type), cmpInst.rhs, cmpInst.type);
        insert(std::move(first), std::move(second));
    }
    else if (cmpInst.rhs->kind == Operand::Kind::Imm) {
        auto first = std::make_unique<MoveInst>(cmpInst.rhs, genDstOperand(cmpInst.type), cmpInst.type);
        auto second = std::make_unique<CmpInst>(cmpInst.lhs, genDstOperand(cmpInst.type), cmpInst.type);
        insert(std::move(first), std::move(second));
    }
    else
        insert(std::make_unique<CmpInst>(cmpInst));
}

void FixUpInstructions::visit(IdivInst& idivInst)
{
    auto first = std::make_unique<MoveInst>(idivInst.operand, genSrcOperand(idivInst.type), idivInst.type);
    auto second = std::make_unique<IdivInst>(genSrcOperand(idivInst.type), idivInst.type);
    insert(std::move(first), std::move(second));
}


void FixUpInstructions::visit(DivInst& div)
{
    auto first = std::make_unique<MoveInst>(div.operand, genSrcOperand(div.type), div.type);
    auto second = std::make_unique<DivInst>(genSrcOperand(div.type), div.type);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::visit(Cvttsd2siInst& cvttsd2si)
{
    auto first = std::make_unique<Cvttsd2siInst>(
        cvttsd2si.src, genSrcOperand(cvttsd2si.dstType), cvttsd2si.dstType);
    auto second = std::make_unique<MoveInst>(genDstOperand(cvttsd2si.dstType), cvttsd2si.dst, cvttsd2si.dstType);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::visit(Cvtsi2sdInst& cvtsi2sd)
{
    auto first = std::make_unique<MoveInst>(cvtsi2sd.src, genSrcOperand(cvtsi2sd.srcType), cvtsi2sd.srcType);
    auto second = std::make_unique<Cvtsi2sdInst>(
        genSrcOperand(cvtsi2sd.srcType), genDstOperand(cvtsi2sd.srcType), cvtsi2sd.srcType);
    auto third = std::make_unique<MoveInst>(genDstOperand(cvtsi2sd.srcType), cvtsi2sd.dst, cvtsi2sd.srcType);
    insert(std::move(first), std::move(second), std::move(third));
}

std::shared_ptr<RegisterOperand> FixUpInstructions::genSrcOperand(AsmType type)
{
    if (type == AsmType::Double)
        return std::make_shared<RegisterOperand>(RegisterOperand::Kind::XMM14, type);
    return std::make_shared<RegisterOperand>(RegisterOperand::Kind::R10, type);
}

std::shared_ptr<RegisterOperand> FixUpInstructions::genDstOperand(AsmType type)
{
    if (type == AsmType::Double)
        return std::make_shared<RegisterOperand>(RegisterOperand::Kind::XMM15, type);
    return std::make_shared<RegisterOperand>(RegisterOperand::Kind::R11, type);
}
} // namespace CodeGen