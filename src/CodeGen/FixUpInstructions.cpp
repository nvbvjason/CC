#include "FixUpInstructions.hpp"

namespace CodeGen {


void FixUpInstructions::fixUp()
{
    m_copy.reserve(m_insts.size() * 3 + 1);
    if (0 < -stackAlloc) {
        i32 allocationSize = -stackAlloc;
        allocationSize += 16 - allocationSize % 16;
        m_copy.emplace_back(std::make_unique<BinaryInst>(
            std::make_shared<ImmOperand>(8),
            std::make_shared<RegisterOperand>(RegisterOperand::Type::SP, 4),
            BinaryInst::Operator::Sub, AssemblyType::QuadWord));
        //m_copy.push_back(std::make_unique<AllocStackInst>(allocationSize));
    }
    while (!m_insts.empty()) {
        auto inst = std::move(m_insts.front());
        m_insts.erase(m_insts.begin());
        if (inst->kind == Inst::Kind::Move)
            visit(*static_cast<MoveInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::Binary)
            visit(*static_cast<BinaryInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::Cmp)
            visit(*static_cast<CmpInst*>(inst.get()));
        else if (inst->kind == Inst::Kind::Idiv)
            visit(*static_cast<IdivInst*>(inst.get()));
        else
            m_copy.push_back(std::move(inst));
    }
    m_insts.swap(m_copy);
}



void FixUpInstructions::visit(MoveInst& moveInst)
{
    if (areBothOnTheStack(moveInst)) {
        auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10, 4);
        auto first = std::make_unique<MoveInst>(moveInst.src, regR10, moveInst.type);
        auto second = std::make_unique<MoveInst>(regR10, moveInst.dst, moveInst.type);
        insert(std::move(first), std::move(second));
    }
    else
        insert(std::make_unique<MoveInst>(moveInst));
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
    auto regCX = std::make_shared<RegisterOperand>(RegisterOperand::Type::CX, 4);
    auto regCL = std::make_shared<RegisterOperand>(RegisterOperand::Type::CX, 1);
    auto first = std::make_unique<MoveInst>(binaryInst.lhs, regCX, binaryInst.type);
    auto second = std::make_unique<BinaryInst>(regCL, binaryInst.rhs, binaryInst.oper, binaryInst.type);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::binaryMul(BinaryInst& binaryInst)
{
    auto regR11 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R11, 4);
    auto first = std::make_unique<MoveInst>(binaryInst.rhs, regR11, binaryInst.type);
    auto second = std::make_unique<BinaryInst>(binaryInst.lhs, regR11, binaryInst.oper, binaryInst.type);
    auto third = std::make_unique<MoveInst>(regR11, binaryInst.rhs, binaryInst.type);
    insert(std::move(first), std::move(second), std::move(third));
}

void FixUpInstructions::binaryOthers(BinaryInst& binaryInst)
{
    auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10, 4);
    auto first = std::make_unique<MoveInst>(binaryInst.lhs, regR10, binaryInst.type);
    auto second = std::make_unique<BinaryInst>(regR10, binaryInst.rhs, binaryInst.oper, binaryInst.type);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::visit(CmpInst& cmpInst)
{
    if (areBothOnTheStack(cmpInst)) {
        auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10, 4);
        auto first = std::make_unique<MoveInst>(cmpInst.lhs, regR10, cmpInst.type);
        auto second = std::make_unique<CmpInst>(regR10, cmpInst.rhs, cmpInst.type);
        insert(std::move(first), std::move(second));
    }
    else if (cmpInst.rhs->kind == Operand::Kind::Imm) {
        auto regR11 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R11, 4);
        auto first = std::make_unique<MoveInst>(cmpInst.rhs, regR11, cmpInst.type);
        auto second = std::make_unique<CmpInst>(cmpInst.lhs, regR11, cmpInst.type);
        insert(std::move(first), std::move(second));
    }
    else
        insert(std::make_unique<CmpInst>(cmpInst));
}

void FixUpInstructions::visit(IdivInst& idivInst)
{
    auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10, 4);
    auto first = std::make_unique<MoveInst>(idivInst.operand, regR10, idivInst.type);
    auto second = std::make_unique<IdivInst>(regR10, idivInst.type);
    insert(std::move(first), std::move(second));
}

} // namespace CodeGen
