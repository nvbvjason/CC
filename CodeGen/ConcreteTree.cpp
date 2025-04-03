#include "ConcreteTree.hpp"

#include "AbstractTree.hpp"

namespace CodeGen {

void programCodegen(const Tacky::Program &program, Program &programCodegen)
{
    programCodegen.function = static_cast<std::unique_ptr<Function>>(functionCodegen(program.function.get()));
}

Function *functionCodegen(const Tacky::Function *function)
{
    auto functionCodeGen = new Function();
    functionCodeGen->name = function->identifier;
    for (const Tacky::Instruction &instruction : function->instructions) {
        switch (instruction.type) {
            case Tacky::InstructionType::Unary:
                unaryInstructionCodegen(functionCodeGen->instructions, instruction);
                break;
            case Tacky::InstructionType::Return:
                returnInstructionCodegen(functionCodeGen->instructions, instruction);
                break;
            default:
                throw std::runtime_error("Unsupported instruction type");
        }
    }
    return functionCodeGen;
}

void unaryInstructionCodegen(std::vector<Instruction>& instructions, const Tacky::Instruction &instruction)
{
    Instruction movInstruction;
    instructions.push_back(movInstruction);
    instructions.back().type = InstructionType::Mov;
    Mov mov;
    mov.src.type = OperandType::Pseudo;
    auto tackyUnary = std::get<std::unique_ptr<Tacky::Unary>>(instruction.value).get();
    mov.src.value = std::get<std::string>(tackyUnary->source->value);
    mov.dst.type = OperandType::Pseudo;
    mov.dst.value = std::get<std::string>(tackyUnary->destination->value);
    instructions.back().value = mov;

    Instruction unaryInstruction;
    instructions.push_back(unaryInstruction);
    instructions.back().type = InstructionType::Unary;
    Unary unary;
    unary.oper = unaryOperatorCodegen(tackyUnary->operation);
    unary.operand.type = OperandType::Pseudo;
    unary.operand.value = std::get<std::string>(tackyUnary->destination->value);
    instructions.back().value = unary;
}

void returnInstructionCodegen(std::vector<Instruction>& instructions, const Tacky::Instruction &instruction)
{
    Instruction movInstruction;
    instructions.push_back(movInstruction);
    instructions.back().type = InstructionType::Mov;
    Mov mov;
    mov.src.type = OperandType::Pseudo;

    mov.src.value = std::get<std::string>(instruction.value);
    mov.dst.type = OperandType::Register;
    mov.dst.value = Register::AX;
    instructions.back().value = mov;

    Instruction returnInstruction;
    instructions.push_back(returnInstruction);
    instructions.back().type = InstructionType::Ret;
    Return ret;
    instructions.back().value = ret;
}

UnaryOperator unaryOperatorCodegen(const Tacky::UnaryOperationType type)
{
    switch (type)
    {
        case Tacky::UnaryOperationType::Complement:
            return UnaryOperator::Not;
        case Tacky::UnaryOperationType::Negate:
            return UnaryOperator::Neg;
        case Tacky::UnaryOperationType::Invalid:
            return UnaryOperator::Invalid;
    }
    throw std::invalid_argument("Invalid UnaryOperator type");
}

Operand constantOperandCodeGen(const Tacky::Value& value)
{
    Operand result;
    result.type = OperandType::Imm;
    result.value = std::get<i32>(value.value);
    return result;
}

Operand varOperandCodeGen(const Tacky::Value& value)
{
    Operand result;
    result.type = OperandType::Pseudo;
    result.value = std::get<std::string>(value.value);
    return result;
}

}