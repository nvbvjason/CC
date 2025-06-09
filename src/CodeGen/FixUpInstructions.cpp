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
            std::make_shared<RegisterOperand>(RegisterOperand::Kind::SP, AssemblyType::QuadWord),
            BinaryInst::Operator::Sub, AssemblyType::QuadWord));
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
        else
            m_copy.push_back(std::move(inst));
    }
    m_insts.swap(m_copy);
}

void FixUpInstructions::visit(MoveInst& moveInst)
{
    if (areBothOnTheStack(moveInst)) {
        auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R10, moveInst.type);
        auto first = std::make_unique<MoveInst>(moveInst.src, regR10, moveInst.type);
        auto second = std::make_unique<MoveInst>(regR10, moveInst.dst, moveInst.type);
        insert(std::move(first), std::move(second));
    }
    else
        insert(std::make_unique<MoveInst>(moveInst));
}

void FixUpInstructions::visit(MoveSXInst& moveSXInst)
{
    auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R10, AssemblyType::LongWord);
    auto first = std::make_unique<MoveInst>(moveSXInst.src, regR10, AssemblyType::LongWord);
    auto regR11 = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R11, AssemblyType::QuadWord);
    auto second = std::make_unique<MoveSXInst>(regR10, regR11);
    auto third = std::make_unique<MoveInst>(regR11, moveSXInst.dst, AssemblyType::QuadWord);

    insert(std::move(first), std::move(second), std::move(third));
}

void FixUpInstructions::visit(MoveZeroExtendInst& moveZero)
{
    if (moveZero.dst->kind == Operand::Kind::Register) {
        auto first = std::make_unique<MoveInst>(
            std::move(moveZero.src), std::move(moveZero.dst), moveZero.type);
        insert(std::move(first));
        return;
    }
    auto reg11 = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R11, moveZero.type);
    auto first = std::make_unique<MoveInst>(moveZero.src, reg11, moveZero.type);
    auto second = std::make_unique<MoveInst>(reg11, moveZero.dst, moveZero.type);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::visit(BinaryInst& binary)
{
    if (isBinaryShift(binary))
        binaryShift(binary);
    else if (binary.oper == BinaryInst::Operator::Mul)
        binaryMul(binary);
    else
        binaryOthers(binary);
}

void FixUpInstructions::binaryShift(BinaryInst& binaryInst)
{
    auto regCX = std::make_shared<RegisterOperand>(RegisterOperand::Kind::CX, binaryInst.type);
    auto regCL = std::make_shared<RegisterOperand>(RegisterOperand::Kind::CX, AssemblyType::Byte);
    auto first = std::make_unique<MoveInst>(binaryInst.lhs, regCX, binaryInst.type);
    auto second = std::make_unique<BinaryInst>(regCL, binaryInst.rhs, binaryInst.oper, binaryInst.type);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::binaryMul(BinaryInst& binaryInst)
{
    auto regR11 = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R11, binaryInst.type);
    auto first = std::make_unique<MoveInst>(binaryInst.rhs, regR11, binaryInst.type);
    auto second = std::make_unique<BinaryInst>(binaryInst.lhs, regR11, binaryInst.oper, binaryInst.type);
    auto third = std::make_unique<MoveInst>(regR11, binaryInst.rhs, binaryInst.type);
    insert(std::move(first), std::move(second), std::move(third));
}

void FixUpInstructions::binaryOthers(BinaryInst& binaryInst)
{
    auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R10, binaryInst.type);
    auto first = std::make_unique<MoveInst>(binaryInst.lhs, regR10, binaryInst.type);
    auto second = std::make_unique<BinaryInst>(regR10, binaryInst.rhs, binaryInst.oper, binaryInst.type);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::visit(CmpInst& cmpInst)
{
    if (areBothOnTheStack(cmpInst)) {
        auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R10, cmpInst.type);
        auto first = std::make_unique<MoveInst>(cmpInst.lhs, regR10, cmpInst.type);
        auto second = std::make_unique<CmpInst>(regR10, cmpInst.rhs, cmpInst.type);
        insert(std::move(first), std::move(second));
    }
    else if (cmpInst.rhs->kind == Operand::Kind::Imm) {
        auto regR11 = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R11, cmpInst.type);
        auto first = std::make_unique<MoveInst>(cmpInst.rhs, regR11, cmpInst.type);
        auto second = std::make_unique<CmpInst>(cmpInst.lhs, regR11, cmpInst.type);
        insert(std::move(first), std::move(second));
    }
    else
        insert(std::make_unique<CmpInst>(cmpInst));
}

void FixUpInstructions::visit(IdivInst& idivInst)
{
    auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Kind::R10, idivInst.type);
    auto first = std::make_unique<MoveInst>(idivInst.operand, regR10, idivInst.type);
    auto second = std::make_unique<IdivInst>(regR10, idivInst.type);
    insert(std::move(first), std::move(second));
}


void FixUpInstructions::visit(DivInst& div)
{

}

} // namespace CodeGen