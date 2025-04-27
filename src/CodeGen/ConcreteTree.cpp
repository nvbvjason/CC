#include "ConcreteTree.hpp"
#include "IR/ConcreteTree.hpp"
#include "PseudoRegisterReplacer.hpp"
#include "AbstractTree.hpp"

#include <stdexcept>

namespace CodeGen {

void program(const Ir::Program &program, Program &programCodegen)
{
    programCodegen.function = function(program.function.get());
}

std::unique_ptr<Function> function(const Ir::Function *function)
{
    auto functionCodeGen = std::make_unique<Function>();
    functionCodeGen->name = function->identifier;
    for (const std::shared_ptr<Ir::Instruction>& inst : function->instructions) {
        switch (inst->type) {
            case Ir::Instruction::Type::Unary: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irUnary = static_cast<Ir::UnaryInst*>(inst.get());
                unaryInst(functionCodeGen->instructions, irUnary);
                break;
            }
            case Ir::Instruction::Type::Return: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irReturn = static_cast<Ir::ReturnInst*>(inst.get());
                returnInst(functionCodeGen->instructions, irReturn);
                break;
            }
            case Ir::Instruction::Type::Binary: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irBinary = static_cast<Ir::BinaryInst*>(inst.get());
                binaryInst(functionCodeGen->instructions, irBinary);
                break;
            }
            default:
                throw std::runtime_error("Unsupported instruction type");
        }
    }
    return functionCodeGen;
}

void unaryInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary)
{
    std::shared_ptr<Operand> src = operand(irUnary->source);
    std::shared_ptr<Operand> dst = operand(irUnary->destination);
    auto moveInst = std::make_unique<MoveInst>(src, dst);
    insts.push_back(std::move(moveInst));

    UnaryInst::Operator oper = unaryOperator(irUnary->operation);
    auto unaryInst = std::make_unique<UnaryInst>(oper, dst);
    insts.push_back(std::move(unaryInst));
}

void binaryInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    switch (irBinary->operation) {
        case Ir::BinaryInst::Operation::Add:
        case Ir::BinaryInst::Operation::Subtract:
        case Ir::BinaryInst::Operation::Multiply:
            binaryOtherInst(insts, irBinary);
            break;
        case Ir::BinaryInst::Operation::Divide:
            binaryDivideInst(insts, irBinary);
            break;
        case Ir::BinaryInst::Operation::Remainder:
            binaryRemainderInst(insts, irBinary);
            break;
        default:
            throw std::runtime_error("Unsupported binary operation");
    }
}

void binaryDivideInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(RegisterOperand::Type::AX);
    auto firstMoveInst = std::make_unique<MoveInst>(src1, regAX);
    insts.push_back(std::move(firstMoveInst));

    auto cdq = std::make_unique<CdqInst>();
    insts.push_back(std::move(cdq));

    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    auto idiv = std::make_unique<IdivInst>(src2);
    insts.push_back(std::move(idiv));

    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    auto secondMoveInst = std::make_unique<MoveInst>(regAX, dst);
    insts.push_back(std::move(secondMoveInst));
}

void binaryRemainderInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(RegisterOperand::Type::AX);
    auto firstMoveInst = std::make_unique<MoveInst>(src1, regAX);
    insts.push_back(std::move(firstMoveInst));

    auto cdq = std::make_unique<CdqInst>();
    insts.push_back(std::move(cdq));

    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    auto idiv = std::make_unique<IdivInst>(src2);
    insts.push_back(std::move(idiv));

    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    const auto regDX = std::make_shared<RegisterOperand>(RegisterOperand::Type::DX);
    auto secondMoveInst = std::make_unique<MoveInst>(regDX, dst);
    insts.push_back(std::move(secondMoveInst));
}

void binaryOtherInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    auto moveInst = std::make_unique<MoveInst>(src1, dst);
    insts.push_back(std::move(moveInst));

    BinaryInst::Operator oper = binaryOperator(irBinary->operation);
    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    auto binaryInst = std::make_unique<BinaryInst>(oper, src2, dst);
    insts.push_back(std::move(binaryInst));
}

void returnInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::ReturnInst* inst)
{
    std::shared_ptr<Operand> val = operand(inst->returnValue);
    std::shared_ptr<Operand> operandRegister = std::make_shared<RegisterOperand>(RegisterOperand::Type::AX);
    auto moveInst = std::make_unique<MoveInst>(val, operandRegister);
    insts.push_back(std::move(moveInst));

    auto instRet = std::make_unique<ReturnInst>();
    insts.push_back(std::move(instRet));
}

UnaryInst::Operator unaryOperator(const Ir::UnaryInst::Operation type)
{
    switch (type)
    {
        case Ir::UnaryInst::Operation::Complement:
            return UnaryInst::Operator::Not;
        case Ir::UnaryInst::Operation::Negate:
            return UnaryInst::Operator::Neg;
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

BinaryInst::Operator binaryOperator(const Ir::BinaryInst::Operation type)
{
    switch (type)
    {
        case Ir::BinaryInst::Operation::Add:
            return BinaryInst::Operator::Add;
        case Ir::BinaryInst::Operation::Subtract:
            return BinaryInst::Operator::Sub;
        case Ir::BinaryInst::Operation::Multiply:
            return BinaryInst::Operator::Mul;
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value)
{
    switch (value->type) {
        case Ir::Value::Type::Constant: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueConst = static_cast<Ir::ValueConst*>(value.get());
            return std::make_shared<ImmOperand>(valueConst->value);
        }
        case Ir::Value::Type::Variable: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueConst = static_cast<Ir::ValueVar*>(value.get());
            return std::make_shared<PseudoOperand>(valueConst->value);
        }
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

i32 replacingPseudoRegisters(Program& programCodegen)
{
    PseudoRegisterReplacer pseudoRegisterReplacer;
    for (std::unique_ptr<Inst>& inst : programCodegen.function->instructions)
        inst->accept(pseudoRegisterReplacer);
    return pseudoRegisterReplacer.stackPointer();
}

void fixUpMoveInst(std::vector<std::unique_ptr<Inst>>& instructions,
                   std::vector<std::unique_ptr<Inst>>::iterator& it,
                   const std::unique_ptr<Inst>& inst)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    const auto moveInst = static_cast<MoveInst*>(inst.get());
    if (moveInst->source->kind == Operand::Kind::Stack &&
        moveInst->destination->kind == Operand::Kind::Stack) {
        auto src = moveInst->source;
        auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10);
        auto first = std::make_unique<MoveInst>(src, regR10);

        auto dst = moveInst->destination;
        auto second = std::make_unique<MoveInst>(regR10, dst);

        *it = std::move(first);
        constexpr i32 movePastFirst = 1;
        it = instructions.insert(it + movePastFirst, std::move(second));
    }
}

void fixUpImulInst(std::vector<std::unique_ptr<Inst>>& instructions,
                   std::vector<std::unique_ptr<Inst>>::iterator& it,
                   const BinaryInst* binaryInst)
{
    auto dst = binaryInst->rhs;
    auto regR11 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R11);
    auto first = std::make_unique<MoveInst>(dst, regR11);

    BinaryInst::Operator oper = binaryInst->oper;
    auto second = std::make_unique<BinaryInst>(oper, dst, regR11);

    auto src = binaryInst->lhs;
    auto third = std::make_unique<MoveInst>(regR11, src);

    *it = std::move(first);
    constexpr i32 movePast = 1;
    it = instructions.insert(it + movePast, std::move(second));
    it = instructions.insert(it + movePast, std::move(third));
}

void fixUpBinaryInst(std::vector<std::unique_ptr<Inst>>& instructions,
                    std::vector<std::unique_ptr<Inst>>::iterator& it,
                    const std::unique_ptr<Inst>& inst)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    const auto binaryInst = static_cast<BinaryInst*>(inst.get());
    if (binaryInst->oper == BinaryInst::Operator::Mul) {
        fixUpImulInst(instructions, it, binaryInst);
        return;
    }
    auto src = binaryInst->lhs;
    auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10);
    auto first = std::make_unique<MoveInst>(src, regR10);

    BinaryInst::Operator oper = binaryInst->oper;
    auto dst = binaryInst->rhs;
    auto second = std::make_unique<BinaryInst>(oper, regR10, dst);

    *it = std::move(first);
    constexpr i32 movePastFirst = 1;
    it = instructions.insert(it + movePastFirst, std::move(second));
}

void fixUpIdivInst(std::vector<std::unique_ptr<Inst>>& instructions,
                   std::vector<std::unique_ptr<Inst>>::iterator& it,
                   const std::unique_ptr<Inst>& inst)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    const auto idivInst = static_cast<IdivInst*>(inst.get());
    auto src = idivInst->operand;
    auto regR10 = std::make_shared<RegisterOperand>(RegisterOperand::Type::R10);
    auto first = std::make_unique<MoveInst>(src, regR10);

    auto second = std::make_unique<IdivInst>(regR10);

    *it = std::move(first);
    constexpr i32 movePastFirst = 1;
    it = instructions.insert(it + movePastFirst, std::move(second));
}

void fixUpInstructions(Program& programCodegen, i32 stackAlloc)
{
    std::vector<std::unique_ptr<Inst>>& instructions = programCodegen.function->instructions;
    stackAlloc = -stackAlloc;
    auto stackAllocationNode = std::make_unique<AllocStackInst>(stackAlloc);
    instructions.insert(instructions.begin(), std::move(stackAllocationNode));
    for (auto it = instructions.begin(); it != instructions.end(); ++it) {
        const auto& inst = *it;
        if (inst->kind == Inst::Kind::Move)
            fixUpMoveInst(instructions, it, inst);
        else if (inst->kind == Inst::Kind::Binary)
            fixUpBinaryInst(instructions, it, inst);
        else if (inst->kind == Inst::Kind::Idiv)
            fixUpIdivInst(instructions, it, inst);
    }
}

}// namespace CodeGen
