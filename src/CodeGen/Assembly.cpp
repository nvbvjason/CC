#include "Assembly.hpp"

#include <string>
#include <iomanip>

namespace CodeGen {

std::string asmProgram(const Program& program)
{
    std::string result;
    asmFunction(result, program.function);
    result += formatAsm(".section", ".note.GNU-stack,\"\",@progbits");
    return result;
}

void asmFunction(std::string& result, const std::shared_ptr<Function>& functionNode)
{
    result += formatAsm(".globl", functionNode->name);
    result += formatAsm(functionNode->name);
    result += formatAsm("pushq", "%rbp");
    result += formatAsm("movq","%rsp, %rbp");
    for (const std::shared_ptr<Inst>& inst : functionNode->instructions)
        asmInstruction(result, inst);
}

void asmInstruction(std::string& result, const std::shared_ptr<Inst>& instruction)
{
    switch (instruction->kind) {
        case Inst::Kind::AllocateStack: {
            const auto instAllocStack = dynamic_cast<AllocStackInst*>(instruction.get());
            result += formatAsm("subq", "$" + std::to_string(instAllocStack->alloc) + ", %rsp");
            return;
        }
        case Inst::Kind::Move: {
            const auto moveInst = dynamic_cast<MoveInst*>(instruction.get());
            const std::string operand =  "%" + asmOperand(moveInst->source) + ", "
                                             + asmOperand(moveInst->destination);
            result += formatAsm("movl", operand);;
            return;
        }
        case Inst::Kind::Ret: {
            result += formatAsm("movq", "%rbp, %rsp");
            result += formatAsm("popq", "%rbp");
            result += formatAsm("ret");
            return;
        }
        case Inst::Kind::Unary: {
            const auto unaryInst = dynamic_cast<UnaryInst*>(instruction.get());
            result += formatAsm(asmUnaryOperator(unaryInst->oper), asmOperand(unaryInst->destination));
            return;
        }
        case Inst::Kind::Invalid: {
            result += formatAsm("invalid");
            return;
        }
        default:
            result += formatAsm("not set");
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
        case Operand::Kind::Invalid:
            return "invalid";
        default:
            return "not set";
    }
}

std::string asmRegister(const RegisterOperand* reg)
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

std::string asmUnaryOperator(const UnaryInst::Operator oper)
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

std::string formatAsm(const std::string& mnemonic,
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
