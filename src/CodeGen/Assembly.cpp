#include "Assembly.hpp"

#include <string>
#include <format>

namespace CodeGen {

std::string assembleProgram(const Program& program)
{
    std::string result;
    assembleFunction(result, program.function);
    result += "    .section .note.GNU-stack,\"\",@progbits\n";
    return result;
}

void assembleFunction(std::string& result, const std::shared_ptr<Function>& functionNode)
{
    result += "    .globl " + functionNode->name + '\n';
    result += functionNode->name + ":\n";
    result += "    pushq    %rbp\n";
    result += "    movq     %rsp, %rbp\n";
    for (const std::shared_ptr<Inst>& inst : functionNode->instructions)
        assembleInstruction(result, inst);
}

void assembleInstruction(std::string& result, const std::shared_ptr<Inst>& instruction)
{
    switch (instruction->kind) {
        case Inst::Kind::AllocateStack: {
            const auto instAllocStack = dynamic_cast<AllocStackInst*>(instruction.get());
            result += "    subq     %" + std::to_string(instAllocStack->alloc) + ", %rsp\n";
            return;
        }
        case Inst::Kind::Move: {
            const auto moveInst = dynamic_cast<MoveInst*>(instruction.get());
            result += "    movl     %" + assembleOperand(moveInst->source) + ", "
                                       + assembleOperand(moveInst->destination) + '\n';
            return;
        }
        case Inst::Kind::Ret: {
            result += "    movq     %rbp, %rsp\n";
            result += "    popq     %rbp\n";
            result += "    ret\n";
            return;
        }
        case Inst::Kind::Unary: {
            const auto unaryInst = dynamic_cast<UnaryInst*>(instruction.get());
            result += "    " + assembleUnaryOperator(unaryInst->oper) + "     "
                             + assembleOperand(unaryInst->destination) + "\n";
            return;
        }
        case Inst::Kind::Invalid: {
            result += "invalid\n";
            return;
        }
        default:
            result += "not set";
            return;
    }
}

std::string assembleOperand(const std::shared_ptr<Operand>& operand)
{
    switch (operand->kind) {
        case Operand::Kind::Register: {
            const auto registerOperand = dynamic_cast<RegisterOperand*>(operand.get());
            return assembleRegister(registerOperand);
        }
        case Operand::Kind::Pseudo:
            return "invalid pseudo";
        case Operand::Kind::Imm: {
            const auto immOperand = dynamic_cast<ImmOperand*>(operand.get());
            return "$" + std::to_string(immOperand->value);
        }
        case Operand::Kind::Stack: {
            const auto stackOperand = dynamic_cast<StackOperand*>(operand.get());
            return std::to_string(stackOperand->value) + "(%rbp)";
        }
        case Operand::Kind::Invalid:
            return "invalid";
        default:
            return "not set";
    }
}

std::string assembleRegister(const RegisterOperand* reg)
{
    switch (reg->kind) {
        case RegisterOperand::Kind::R10:
            return "%r10d";
        case RegisterOperand::Kind::AX:
            return "%eax";
        case RegisterOperand::Kind::Invalid:
            return "invalid";
        default:
            return "not set";
    }
}

std::string assembleUnaryOperator(const UnaryInst::Operator oper)
{
    switch (oper) {
        case UnaryInst::Operator::Neg:
            return "negl";
        case UnaryInst::Operator::Not:
            return "notl";
        case UnaryInst::Operator::Invalid:
            return "invalid";
        default:
            return "not set";
    }
}

} // CodeGen
