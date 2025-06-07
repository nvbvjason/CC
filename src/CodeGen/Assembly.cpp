#include "Assembly.hpp"

#include <string>
#include <iomanip>
#include <unordered_map>

namespace CodeGen {

std::string asmProgram(const Program& program)
{
    std::string result;
    for (const std::unique_ptr<TopLevel>& topLevel : program.topLevels) {
        if (topLevel->type == TopLevel::Type::StaticVariable) {
            const auto var = static_cast<const StaticVariable*>(topLevel.get());
            asmGlobalVar(result, *var);
            continue;
        }
        const auto function = dynamic_cast<Function*>(topLevel.get());
        asmFunction(result, *function);
    }
    result += ".section .note.GNU-stack,\"\",@progbits\n";
    return result;
}

void asmGlobalVar(std::string& result, const StaticVariable& variable)
{
    if (variable.global)
        result += asmFormatInstruction(".globl", variable.name);
    if (variable.init == 0)
        result += asmFormatInstruction(".bss");
    else
        result += asmFormatInstruction(".data");
    result += asmFormatInstruction(".align","4");
    result += asmFormatLabel(variable.name);
    if (variable.init == 0)
        result += asmFormatInstruction(".zero", "4");
    else
        result += asmFormatInstruction(".long", std::to_string(variable.init));
    result += '\n';
}

void asmFunction(std::string& result, const Function& functionNode)
{
    if (functionNode.isGlobal)
        result += asmFormatInstruction(".globl", functionNode.name);
    result += asmFormatInstruction(".text");
    result += asmFormatLabel(functionNode.name);
    result += asmFormatInstruction("pushq", "%rbp");
    result += asmFormatInstruction("movq","%rsp, %rbp");
    for (const std::unique_ptr<Inst>& inst : functionNode.instructions)
        asmInstruction(result, inst);
    result += '\n';
}

void asmStaticVariable(std::string& result, const StaticVariable& staticVariableNode)
{
    if (staticVariableNode.global)
        result += asmFormatInstruction(".globl", staticVariableNode.name);
    if (staticVariableNode.init == 0)
        result += asmFormatInstruction(".data");
    else
        result += asmFormatInstruction(".bss");
    result += asmFormatInstruction(alignDirective());
    result += asmFormatLabel(staticVariableNode.name);
    if (staticVariableNode.init == 0)
        result += asmFormatInstruction(".zero 4");
    else
        result += asmFormatInstruction(".long " + std::to_string(staticVariableNode.init));
}

void asmInstruction(std::string& result, const std::unique_ptr<Inst>& instruction)
{
    switch (instruction->kind) {
        case Inst::Kind::Move: {
            const auto moveInst = dynamic_cast<MoveInst*>(instruction.get());
            std::string operand = asmOperand(moveInst->src) + ", " + asmOperand(moveInst->dst);
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
        case Inst::Kind::Cmp: {
            const auto cmpInst = dynamic_cast<CmpInst*>(instruction.get());
            const std::string operands = asmOperand(cmpInst->lhs) + ", " + asmOperand(cmpInst->rhs);
            result += asmFormatInstruction("cmpl", operands);
            return;
        }
        case Inst::Kind::Jmp: {
            const auto jmpInst = dynamic_cast<JmpInst*>(instruction.get());
            result += asmFormatInstruction("jmp", createLabel(jmpInst->target.value));
            return;
        }
        case Inst::Kind::JmpCC: {
            const auto jmpCCInst = dynamic_cast<JmpCCInst*>(instruction.get());
            result += asmFormatInstruction("j" + condCode(jmpCCInst->condition), createLabel(jmpCCInst->target.value));
            return;
        }
        case Inst::Kind::SetCC: {
            const auto setCCInst = dynamic_cast<SetCCInst*>(instruction.get());
            result += asmFormatInstruction("set" + condCode(setCCInst->condition), asmOperand(setCCInst->operand));
            return;
        }
        case Inst::Kind::Label: {
            const auto labelInst = dynamic_cast<LabelInst*>(instruction.get());
            result += asmFormatLabel(createLabel(labelInst->target.value));
            return;
        }
        case Inst::Kind::Push: {
            const auto pushInst = dynamic_cast<PushInst*>(instruction.get());
            result += asmFormatInstruction("pushq", asmOperand(pushInst->operand));
            return;
        }
        case Inst::Kind::Call: {
            const auto callInst = dynamic_cast<CallInst*>(instruction.get());
            result += asmFormatInstruction("call", callInst->funName.value);
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
            if (immOperand->type == AssemblyType::LongWord)
                return "$" + std::to_string(std::get<i32>(immOperand->value));
            return "$" + std::to_string(std::get<i64>(immOperand->value));
        }
        case Operand::Kind::Stack: {
            const auto stackOperand = dynamic_cast<StackOperand*>(operand.get());
            return std::to_string(stackOperand->value) + "(%rbp)";
        }
        case Operand::Kind::Data: {
            const auto dataOperand = dynamic_cast<DataOperand*>(operand.get());
            return dataOperand->identifier + "(%rip)";
        }
        default:
            return "not set asmOperand";
    }
}

std::string asmRegister(const RegisterOperand* reg)
{
    using Type = RegisterOperand::Type;
    static const std::unordered_map<Type, std::array<const char*, 4>> registerMap = {
        {Type::AX,  {"%al",   "%ax",   "%eax",  "%rax"}},
        {Type::CX,  {"%cl",   "%cx",   "%ecx",  "%rcx"}},
        {Type::DX,  {"%dl",   "%dx",   "%edx",  "%rdx"}},
        {Type::DI,  {"%dil",  "%di",   "%edi",  "%rdi"}},
        {Type::SI,  {"%sil",  "%si",   "%esi",  "%rsi"}},
        {Type::R8,  {"%r8b",  "%r8w",  "%r8d",  "%r8"}},
        {Type::R9,  {"%r9b",  "%r9w",  "%r9d",  "%r9"}},
        {Type::R10, {"%r10b", "%r10w", "%r10d", "%r10"}},
        {Type::R11, {"%r11b", "%r11w", "%r11d", "%r11"}}
    };

    // Find the register in our map
    auto it = registerMap.find(reg->type);
    if (it == registerMap.end()) {
        return "invalid_register";
    }

    // Get the appropriate name based on size
    const auto& names = it->second;
    switch (reg->size) {
        case 1: return names[0];  // 8-bit
        case 2: return names[1];  // 16-bit
        case 4: return names[2];  // 32-bit
        case 8: return names[3];  // 64-bit
        default: return "invalid_size";
    }
}

std::string asmUnaryOperator(const UnaryInst::Operator oper)
{
    using Operator = UnaryInst::Operator;
    switch (oper) {
        case Operator::Neg:      return "negl";
        case Operator::Not:      return "notl";
        default:                 return "not set asmUnaryOperator";
    }
}

std::string asmBinaryOperator(BinaryInst::Operator oper)
{
    using Operator = BinaryInst::Operator;
    switch (oper) {
        case Operator::Mul:             return "imull";
        case Operator::Add:             return "addl";
        case Operator::Sub:             return "subl";

        case Operator::BitwiseAnd:      return "andl";
        case Operator::BitwiseOr:       return "orl";
        case Operator::BitwiseXor:      return "xorl";

        case Operator::LeftShift:       return "shll";
        case Operator::RightShift:      return "sarl";
        default:
            return "not set asmBinaryOperator";
    }
}

std::string alignDirective()
{
    return ".align 4";
}

std::string asmFormatLabel(const std::string& name)
{
    return name + ":\n";
}

std::string createLabel(const std::string& name)
{
    return ".L" + name;
}

std::string condCode(const BinaryInst::CondCode condCode)
{
    switch (condCode) {
        case BinaryInst::CondCode::E:
            return "e";
        case BinaryInst::CondCode::NE:
            return "ne";
        case BinaryInst::CondCode::L:
            return "l";
        case BinaryInst::CondCode::LE:
            return "le";
        case BinaryInst::CondCode::G:
            return "g";
        case BinaryInst::CondCode::GE:
            return "ge";
        default:
            return "not set condCode";
    }
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
