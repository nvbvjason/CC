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
                const auto irUnary = dynamic_cast<Ir::UnaryInst*>(inst.get());
                unaryInst(functionCodeGen->instructions, irUnary);
                break;
            }
            case Ir::Instruction::Type::Return: {
                const auto irReturn = dynamic_cast<Ir::ReturnInst*>(inst.get());
                returnInst(functionCodeGen->instructions, irReturn);
                break;
            }
            case Ir::Instruction::Type::Binary: {
                const auto irBinary = dynamic_cast<Ir::BinaryInst*>(inst.get());
                binaryInst(functionCodeGen->instructions, irBinary);
                break;
            }
            default:
                throw std::runtime_error("Unsupported instruction type");
        }
    }
    return functionCodeGen;
}

void unaryInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary)
{
    std::shared_ptr<Operand> src = operand(irUnary->source);
    std::shared_ptr<Operand> dst = operand(irUnary->destination);
    const auto moveInst = std::make_shared<MoveInst>(src, dst);
    insts.push_back(moveInst);

    UnaryInst::Operator oper = unaryOperator(irUnary->operation);
    auto unaryInst = std::make_unique<UnaryInst>(oper, dst);
    insts.push_back(std::move(unaryInst));
}

void binaryInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
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


void binaryDivideInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(RegisterOperand::Kind::AX);
    const auto firstMoveInst = std::make_shared<MoveInst>(src1, regAX);
    insts.push_back(firstMoveInst);

    const auto cdq = std::make_shared<CdqInst>();
    insts.push_back(cdq);

    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    const auto idiv = std::make_shared<IdivInst>(src2);
    insts.push_back(idiv);

    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    const auto secondMoveInst = std::make_shared<MoveInst>(regAX, dst);
    insts.push_back(secondMoveInst);
}

void binaryRemainderInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(RegisterOperand::Kind::AX);
    const auto firstMoveInst = std::make_shared<MoveInst>(src1, regAX);
    insts.push_back(firstMoveInst);

    const auto cdq = std::make_shared<CdqInst>();
    insts.push_back(cdq);

    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    const auto idiv = std::make_shared<IdivInst>(src2);
    insts.push_back(idiv);

    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    const auto regDX = std::make_shared<RegisterOperand>(RegisterOperand::Kind::DX);
    const auto secondMoveInst = std::make_shared<MoveInst>(regDX, dst);
    insts.push_back(secondMoveInst);
}

void binaryOtherInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    const auto moveInst = std::make_shared<MoveInst>(src1, dst);
    insts.push_back(moveInst);

    BinaryInst::Operator oper = binaryOperator(irBinary->operation);
    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    auto binaryInst = std::make_unique<BinaryInst>(oper, src2, dst);
    insts.push_back(std::move(binaryInst));
}

void returnInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::ReturnInst* inst)
{
    std::shared_ptr<Operand> val = operand(inst->returnValue);
    std::shared_ptr<Operand> operandRegister = std::make_shared<RegisterOperand>(RegisterOperand::Kind::AX);
    auto moveInst = std::make_shared<MoveInst>(val, operandRegister);
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
        case Ir::UnaryInst::Operation::Invalid:
            return UnaryInst::Operator::Invalid;
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

BinaryInst::Operator binaryOperator(Ir::BinaryInst::Operation type)
{
    switch (type)
    {
        case Ir::BinaryInst::Operation::Add:
            return BinaryInst::Operator::Add;
        case Ir::BinaryInst::Operation::Subtract:
            return BinaryInst::Operator::Sub;
        case Ir::BinaryInst::Operation::Multiply:
            return BinaryInst::Operator::Mul;
        case Ir::BinaryInst::Operation::Divide:
            return BinaryInst::Operator::Div;
        case Ir::BinaryInst::Operation::Remainder:
            return BinaryInst::Operator::Mod;
        case Ir::BinaryInst::Operation::Invalid:
            return BinaryInst::Operator::Invalid;
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value)
{
    switch (value->type) {
        case Ir::Value::Type::Constant: {
            const auto valueConst = dynamic_cast<Ir::ValueConst*>(value.get());
            return std::make_shared<ImmOperand>(valueConst->value);
        }
        case Ir::Value::Type::Variable: {
            const auto valueConst = dynamic_cast<Ir::ValueVar*>(value.get());
            return std::make_shared<PseudoOperand>(valueConst->value);
        }
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

i32 replacingPseudoRegisters(Program& programCodegen)
{
    auto instructions = programCodegen.function->instructions;
    PseudoRegisterReplacer pseudoRegisterReplacer;
    for (std::shared_ptr<Inst>& inst : instructions)
        inst->accept(pseudoRegisterReplacer);
    return pseudoRegisterReplacer.stackPointer();
}

void fixUpInstructions(Program& programCodegen, i32 stackAlloc)
{
    std::list<std::shared_ptr<Inst>>& instructions = programCodegen.function->instructions;
    stackAlloc = -stackAlloc;
    auto stackAllocationNode = std::make_shared<AllocStackInst>(stackAlloc);
    instructions.insert(instructions.begin(), std::move(stackAllocationNode));
    for (auto it = instructions.begin(); it != instructions.end(); ++it) {
        const auto& inst = *it;
        if (inst->kind != Inst::Kind::Move)
            continue;
        const auto moveInst = dynamic_cast<MoveInst*>(inst.get());
        if (moveInst->source->kind == Operand::Kind::Stack &&
            moveInst->destination->kind == Operand::Kind::Stack) {
            auto src = moveInst->source;
            auto dst = moveInst->destination;
            auto first = std::make_shared<MoveInst>(
                src, std::make_shared<RegisterOperand>(RegisterOperand::Kind::R10)
            );
            auto second = std::make_shared<MoveInst>(
                std::make_shared<RegisterOperand>(RegisterOperand::Kind::R10), dst
            );
            instructions.insert(it, {first, second});
            it = instructions.erase(it);
        }
    }
}

}// namespace CodeGen
