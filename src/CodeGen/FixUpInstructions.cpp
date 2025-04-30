#include "FixUpInstructions.hpp"

namespace CodeGen {


void FixUpInstructions::fixUp() {
    m_copy.reserve(m_insts.size() * 3 + 1);
    if (0 < stackAlloc)
        m_copy.push_back(std::make_unique<AllocStackInst>(stackAlloc));
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



void FixUpInstructions::visit(MoveInst& moveInst) {
    if (areBothOnTheStack(moveInst)) {
        auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10);
        auto first = std::make_unique<MoveInst>(moveInst.src, regR10);
        auto second = std::make_unique<MoveInst>(regR10, moveInst.dst);
        insert(std::move(first), std::move(second));
    }
    else
        insert(std::make_unique<MoveInst>(moveInst));
}

void FixUpInstructions::visit(BinaryInst& binaryInst) {
    if (isBinaryShift(binaryInst))
        binaryShift(binaryInst);
    else if (binaryInst.oper == BinaryInst::Operator::Mul)
        binaryMul(binaryInst);
    else
        binaryOthers(binaryInst);
}

void FixUpInstructions::binaryShift(BinaryInst& binaryInst) {
    auto regCX = std::make_shared<RegisterOperand>(RegisterOperand::Type::CX);
    auto regCL = std::make_shared<RegisterOperand>(RegisterOperand::Type::CL);
    auto first = std::make_unique<MoveInst>(binaryInst.lhs, regCX);
    auto second = std::make_unique<BinaryInst>(binaryInst.oper, regCL, binaryInst.rhs);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::binaryMul(BinaryInst& binaryInst) {
    auto regR11 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R11);
    auto first = std::make_unique<MoveInst>(binaryInst.rhs, regR11);
    auto second = std::make_unique<BinaryInst>(binaryInst.oper, binaryInst.lhs, regR11);
    auto third = std::make_unique<MoveInst>(regR11, binaryInst.rhs);
    insert(std::move(first), std::move(second), std::move(third));
}

void FixUpInstructions::binaryOthers(BinaryInst& binaryInst) {
    auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10);
    auto first = std::make_unique<MoveInst>(binaryInst.lhs, regR10);
    auto second = std::make_unique<BinaryInst>(binaryInst.oper, regR10, binaryInst.rhs);
    insert(std::move(first), std::move(second));
}

void FixUpInstructions::visit(CmpInst& cmpInst) {
    if (areBothOnTheStack(cmpInst)) {
        auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10);
        auto first = std::make_unique<MoveInst>(cmpInst.lhs, regR10);
        auto second = std::make_unique<CmpInst>(regR10, cmpInst.rhs);
        insert(std::move(first), std::move(second));
    }
    else if (cmpInst.rhs->kind == Operand::Kind::Imm) {
        auto regR11 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R11);
        auto first = std::make_unique<MoveInst>(cmpInst.rhs, regR11);
        auto second = std::make_unique<CmpInst>(cmpInst.lhs, regR11);
        insert(std::move(first), std::move(second));
    }
    else
        insert(std::make_unique<CmpInst>(cmpInst));
}

void FixUpInstructions::visit(IdivInst& idivInst) {
    auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10);
    auto first = std::make_unique<MoveInst>(idivInst.operand, regR10);
    auto second = std::make_unique<IdivInst>(regR10);
    insert(std::move(first), std::move(second));
}

} // namespace CodeGen
