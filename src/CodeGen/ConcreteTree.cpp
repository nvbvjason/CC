#include "ConcreteTree.hpp"
#include "AbstractTree.hpp"

#include <stdexcept>

namespace CodeGen {

void program(const IR::Program &program, Program &programCodegen)
{
    programCodegen.function = function(program.function.get());
}

std::unique_ptr<Function> function(const IR::Function *function)
{
    auto functionCodeGen = std::make_unique<Function>();
    functionCodeGen->name = function->identifier;
    for (const IR::Instruction &instruction : function->instructions) {
        switch (instruction.type) {
            case IR::Instruction::Type::Unary:
                unaryInstruction(functionCodeGen->instructions, instruction);
                break;
            case IR::Instruction::Type::Return:
                returnInstruction(functionCodeGen->instructions, instruction);
                break;
            default:
                throw std::runtime_error("Unsupported instruction type");
        }
    }
    return functionCodeGen;
}

void unaryInstruction(std::vector<Instruction>& instructions, const IR::Instruction &instruction)
{
    Instruction movInstruction;
    instructions.push_back(movInstruction);
    instructions.back().type = InstructionType::Mov;
    Mov mov;
    mov.src.type = OperandType::Pseudo;
    auto tackyUnary = std::get<std::unique_ptr<IR::Unary>>(instruction.value).get();
    mov.src.value = std::get<std::string>(tackyUnary->source.value);
    mov.dst.type = OperandType::Pseudo;
    mov.dst.value = std::get<std::string>(tackyUnary->destination.value);
    instructions.back().value = mov;

    Instruction unaryInstruction;

    instructions.push_back(unaryInstruction);
    instructions.back().type = InstructionType::Unary;
    Unary unary;
    unary.oper = unaryOperator(tackyUnary->operation);
    unary.operand.type = OperandType::Pseudo;
    unary.operand.value = std::get<std::string>(tackyUnary->destination.value);
    instructions.back().value = unary;
}

void returnInstruction(std::vector<Instruction>& instructions, const IR::Instruction &instruction)
{
    Instruction movInstruction;
    instructions.push_back(movInstruction);
    instructions.back().type = InstructionType::Mov;
    Mov mov;

    mov.src.type = OperandType::Pseudo;
    mov.src.value = std::get<std::string>(std::get<std::unique_ptr<IR::Value>>(instruction.value)->value);
    mov.dst.type = OperandType::Register;
    mov.dst.value = Register::AX;

    instructions.back().value = mov;

    Instruction returnInstruction;
    instructions.push_back(returnInstruction);
    instructions.back().type = InstructionType::Ret;
    Return ret;
    instructions.back().value = ret;
}

UnaryOperator unaryOperator(const IR::Unary::OperationType type)
{
    switch (type)
    {
        case IR::Unary::OperationType::Complement:
            return UnaryOperator::Not;
        case IR::Unary::OperationType::Negate:
            return UnaryOperator::Neg;
        case IR::Unary::OperationType::Invalid:
            return UnaryOperator::Invalid;
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

Operand constantOperand(const IR::Value& value)
{
    Operand result;
    result.type = OperandType::Imm;
    result.value = std::get<i32>(value.value);
    return result;
}

Operand varOperand(const IR::Value& value)
{
    Operand result;
    result.type = OperandType::Pseudo;
    result.value = std::get<std::string>(value.value);
    return result;
}

}