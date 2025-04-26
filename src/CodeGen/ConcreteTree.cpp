#include "ConcreteTree.hpp"
#include "IR/ConcreteTree.hpp"

#include "AbstractTree.hpp"

#include <stdexcept>
#include <unordered_map>

#include "Parsing/AstVisualizer.hpp"

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
    auto moveInst = std::make_shared<MoveInst>(src, dst);
    insts.push_back(std::move(moveInst));

    UnaryInst::Operator oper = unaryOperator(irUnary->operation);
    auto unaryInst = std::make_unique<UnaryInst>(oper, dst);
    insts.push_back(std::move(unaryInst));
}

void returnInst(std::list<std::shared_ptr<Inst>>& insts, const Ir::ReturnInst* inst)
{
    std::shared_ptr<Operand> val = operand(inst->returnValue);
    std::shared_ptr<Operand> operandRegister = std::make_shared<OperandRegister>(OperandRegister::Kind::R10);
    auto moveInst = std::make_shared<MoveInst>(val, operandRegister);
    insts.push_back(std::move(moveInst));
    auto instRet = std::make_unique<InstRet>();
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

std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value)
{
    switch (value->type) {
        case Ir::Value::Type::Constant: {
            const auto valueConst = dynamic_cast<Ir::ValueConst*>(value.get());
            return std::make_shared<OperandImm>(valueConst->value);
        }
        case Ir::Value::Type::Variable: {
            const auto valueConst = dynamic_cast<Ir::ValueVar*>(value.get());
            return std::make_shared<OperandPseudo>(valueConst->value);
        }
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

void replacePseudoRegister(std::unordered_map<std::string, i32>& map, i32& stackPtr, std::shared_ptr<Operand>& operand)
{
    if (operand->kind == Operand::Kind::Pseudo) {
        const auto operandPseudo = dynamic_cast<OperandPseudo*>(operand.get());
        if (!map.contains(operandPseudo->identifier)) {
            stackPtr -= 4;
            map[operandPseudo->identifier] = stackPtr;
        }
        operand = std::make_shared<OperandStack>(map.at(operandPseudo->identifier));
    }
}

i32 replacingPseudoRegisters(Program &programCodegen)
{
    std::list<std::shared_ptr<Inst>> instructions = programCodegen.function->instructions;
    std::unordered_map<std::string, i32> map;
    i32 stackPtr = 0;
    for (std::shared_ptr<Inst>& inst : instructions) {
        if (inst->kind == Inst::Kind::Move) {
            const auto moveInst = dynamic_cast<MoveInst*>(inst.get());
            replacePseudoRegister(map, stackPtr, moveInst->source);
            replacePseudoRegister(map, stackPtr, moveInst->destination);
        }
        if (inst->kind == Inst::Kind::Unary) {
            const auto unaryInst = dynamic_cast<UnaryInst*>(inst.get());
            replacePseudoRegister(map, stackPtr, unaryInst->destination);
        }
    }
    return stackPtr;
}

void fixUpInstructions(Program &programCodegen, i32 stackAlloc)
{
    std::list<std::shared_ptr<Inst>>& instructions = programCodegen.function->instructions;
    auto stackAllocationNode = std::make_shared<InstAllocStack>(stackAlloc);
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
                src, std::make_shared<OperandRegister>(OperandRegister::Kind::R10)
            );
            auto second = std::make_shared<MoveInst>(
                std::make_shared<OperandRegister>(OperandRegister::Kind::R10), dst
            );
            instructions.insert(it, {first, second});
            it = instructions.erase(it);
        }
    }
}

}
