#include "Assembly.hpp"

#include <string>
#include <iomanip>

namespace CodeGen {

std::string asmProgram(const Program& program)
{
    std::string result;
    for (const std::unique_ptr<Function>& function : program.functions)
        asmFunction(result, function);
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
            result += asmFormatInstruction("call", createLabel(callInst->funName.value));
            return;
        }
        case Inst::Kind::DeallocateStack: {
            const auto deallocateStack = dynamic_cast<DeallocStackInst*>(instruction.get());
            result += asmFormatInstruction("addq", "$" + std::to_string(deallocateStack->dealloc) + ", %rsp");
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
    using Type = RegisterOperand::Type;
    using RegisterNames = std::array<const char*, 4>;
    static const std::array<std::pair<Type, RegisterNames>, 9> registerMap = {
        {
            //                   1-byte   2-byte   4-byte   8-byte
            {Type::AX,  {"%al",   "%ah",   "%eax",  "%rax"}},
            {Type::CX,  {"%cl",   "%ch",   "%ecx",  "%rcx"}},
            {Type::DX,  {"%dl",   "%dx",   "%edx",  "%rdx"}},
            {Type::DI,  {"%dil",  "%di",   "%edi",  "%rdi"}},
            {Type::SI,  {"%sil",  "%si",   "%esi",  "%rsi"}},
            {Type::R8,  {"%r8b",  "%r8w",  "%r8d",  "%r8"}},
            {Type::R9,  {"%r9b",  "%r9w",  "%r9d",  "%r9"}},
            {Type::R10, {"%r10b", "%r10w", "%r10d", "%r10"}},
            {Type::R11, {"%r11b", "%r11w", "%r11d", "%r11"}}
        }};

    static_assert(sizeof(RegisterNames) == sizeof(const char*[4]),
        "RegisterNames size mismatch");

    const auto it = std::lower_bound(
        registerMap.begin(), registerMap.end(), reg->type,
        [](const auto& pair, Type type) { return pair.first < type; }
    );

    if (it == registerMap.end() || it->first != reg->type)
        return "invalid_register";

    const auto& names = it->second;
    const u8 size = reg->size;
    if (size != 1 && size != 2 && size != 4 && size != 8)
        return "invalid_size";

    size_t index;
    switch (size) {
        case 1:  index = 0; break;
        case 2:  index = 1; break;
        case 4:  index = 2; break;
        case 8:  index = 3; break;
        default: return "invalid_size";
    }

    return names[std::bit_width(index)];
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
            return "sarl";
        default:
            return "not set asmBinaryOperator";
    }
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
