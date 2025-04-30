#include "Assembly.hpp"

#include <string>
#include <iomanip>

namespace CodeGen {

std::string asmProgram(const Program& program)
{
    std::string result;
    asmFunction(result, program.function);
    result += ".section .note.GNU-stack,\"\",@progbits\n";
    return result;
}

void asmFunction(std::string& result, const std::unique_ptr<Function>& functionNode)
{
    result += ".globl    " + functionNode->name + '\n';
    result += asmFormatLabel(functionNode->name);
    result += asmFormatInstruction("pushq", "%rbp");
    result += asmFormatInstruction("movq","%rsp, %rbp");
    for (const std::unique_ptr<Inst>& inst : functionNode->instructions)
        asmInstruction(result, inst);
}

void asmInstruction(std::string& result, const std::unique_ptr<Inst>& instruction)
{
    switch (instruction->kind) {
        case Inst::Kind::AllocateStack: {
            const auto instAllocStack = dynamic_cast<AllocStackInst*>(instruction.get());
            result += asmFormatInstruction("subq", "$" + std::to_string(instAllocStack->alloc) + ", %rsp");
            return;
        }
        case Inst::Kind::Move: {
            const auto moveInst = dynamic_cast<MoveInst*>(instruction.get());
            const std::string operand = asmOperand(moveInst->src) + ", " + asmOperand(moveInst->dst);
            result += asmFormatInstruction("movl", operand);;
            return;
        }
        case Inst::Kind::Unary: {
            const auto unaryInst = dynamic_cast<UnaryInst*>(instruction.get());
            result += asmFormatInstruction(asmUnaryOperator(unaryInst->oper), asmOperand(unaryInst->destination));
            return;
        }
        case Inst::Kind::Binary: {
            const auto binaryInst = dynamic_cast<BinaryInst*>(instruction.get());
            std::string operands = asmOperand(binaryInst->lhs) + ", " + asmOperand(binaryInst->rhs);
            result += asmFormatInstruction(asmBinaryOperator(binaryInst->oper), operands);
            return;
        }
        case Inst::Kind::Cdq: {
            result += asmFormatInstruction("cdq");
            return;
        }
        case Inst::Kind::Idiv: {
            const auto idivInst = dynamic_cast<IdivInst*>(instruction.get());
            result += asmFormatInstruction("idivl", asmOperand(idivInst->operand));
            return;
        }
        case Inst::Kind::Ret: {
            result += asmFormatInstruction("movq", "%rbp, %rsp");
            result += asmFormatInstruction("popq", "%rbp");
            result += asmFormatInstruction("ret");
            return;
        }
        default:
            result += asmFormatInstruction("not set asmInstruction");
            return;
    }
}

std::string asmOperand(const std::shared_ptr<Operand>& operand)
{
    switch (operand->kind) {
        case Operand::Kind::Register: {
            const auto registerOperand = dynamic_cast<RegisterOperand*>(operand.get());
            return asmRegister(registerOperand);
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
        default:
            return "not set asmOperand";
    }
}

std::string asmRegister(const RegisterOperand* reg)
{
    switch (reg->type) {
        case RegisterOperand::Type::R10:
            return "%r10d";
        case RegisterOperand::Type::AX:
            return "%eax";
        case RegisterOperand::Type::R11:
            return "%r11d";
        case RegisterOperand::Type::DX:
            return "%edx";
        case RegisterOperand::Type::CX:
            return "%ecx";
        case RegisterOperand::Type::CL:
            return "%cl";
        default:
            return "not set asmRegister";
    }
}

std::string asmUnaryOperator(const UnaryInst::Operator oper)
{
    switch (oper) {
        case UnaryInst::Operator::Neg:
            return "negl";
        case UnaryInst::Operator::Not:
            return "notl";
        default:
            return "not set asmUnaryOperator";
    }
}

std::string asmBinaryOperator(BinaryInst::Operator oper)
{
    switch (oper) {
        case BinaryInst::Operator::Mul:
            return "imull";
        case BinaryInst::Operator::Add:
            return "addl";
        case BinaryInst::Operator::Sub:
            return "subl";

        case BinaryInst::Operator::BitwiseAnd:
            return "andl";
        case BinaryInst::Operator::BitwiseOr:
            return "orl";
        case BinaryInst::Operator::BitwiseXor:
            return "xorl";

        case BinaryInst::Operator::LeftShift:
            return "shll";
        case BinaryInst::Operator::RightShift:
            return "shrl";
        default:
            return "not set asmBinaryOperator";
    }
}

std::string asmFormatLabel(const std::string& name)
{
    return name + ":\n";
}

std::string asmFormatInstruction(const std::string& mnemonic,
                      const std::string& operands,
                      const std::string& comment)
{
    constexpr int mnemonicWidth = 8;
    constexpr int operandsWidth = 16;

    std::ostringstream oss;
    oss << "    " << std::left << std::setw(mnemonicWidth) << mnemonic
        << std::setw(operandsWidth) << operands;
    if (!comment.empty())
        oss << "# " << comment;
    oss << "\n";
    return oss.str();
}

} // CodeGen
