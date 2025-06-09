#include "Assembly.hpp"

#include <string>
#include <iomanip>
#include <unordered_map>

namespace CodeGen {

std::string makeTemporaryPseudoName()
{
    static i32 i = 0;
    return std::to_string(i++) + "..";
}

std::string asmProgram(const Program& program)
{
    std::string result;
    for (const std::unique_ptr<TopLevel>& topLevel : program.topLevels) {
        if (topLevel->type == TopLevel::Type::StaticVariable) {
            const auto var = static_cast<const StaticVariable*>(topLevel.get());
            asmStaticVariable(result, *var);
            continue;
        }
        const auto function = dynamic_cast<Function*>(topLevel.get());
        asmFunction(result, *function);
    }
    result += ".section .note.GNU-stack,\"\",@progbits\n";
    return result;
}

void asmStaticVariable(std::string& result, const StaticVariable& variable)
{
    if (variable.global)
        result += asmFormatInstruction(".globl", variable.name);
    if (variable.init == 0)
        result += asmFormatInstruction(".bss");
    else
        result += asmFormatInstruction(".data");
    if (variable.type == AssemblyType::LongWord)
        result += asmFormatInstruction(".align","4");
    if (variable.type == AssemblyType::QuadWord)
        result += asmFormatInstruction(".align","8");
    result += asmFormatLabel(variable.name);
    if (variable.init == 0 && variable.type == AssemblyType::LongWord)
        result += asmFormatInstruction(".zero 4");
    if (variable.init != 0 && variable.type == AssemblyType::LongWord)
        result += asmFormatInstruction(".long " + std::to_string(variable.init));
    if (variable.init == 0 && variable.type == AssemblyType::QuadWord)
        result += asmFormatInstruction(".zero 8");
    if (variable.init != 0 && variable.type == AssemblyType::QuadWord)
        result += asmFormatInstruction(".quad " + std::to_string(variable.init));
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

void asmInstruction(std::string& result, const std::unique_ptr<Inst>& instruction)
{
    switch (instruction->kind) {
        case Inst::Kind::Move: {
            const auto moveInst = dynamic_cast<MoveInst*>(instruction.get());
            std::string operand = asmOperand(moveInst->src) + ", " + asmOperand(moveInst->dst);
            if (moveInst->type == AssemblyType::LongWord)
                result += asmFormatInstruction("movl", operand);
            if (moveInst->type == AssemblyType::QuadWord)
                result += asmFormatInstruction("movq", operand);;
            return;
        }
        case Inst::Kind::MoveSX: {
            const auto moveSXInst = dynamic_cast<MoveSXInst*>(instruction.get());
            std::string operands = asmOperand(moveSXInst->src) + ", " + asmOperand(moveSXInst->dst);
            result += asmFormatInstruction("movslq", operands);
            return;
        }
        case Inst::Kind::Unary: {
            const auto unaryInst = dynamic_cast<UnaryInst*>(instruction.get());
            result += asmFormatInstruction(asmUnaryOperator(unaryInst->oper, unaryInst->type), asmOperand(unaryInst->destination));
            return;
        }
        case Inst::Kind::Binary: {
            const auto binaryInst = dynamic_cast<BinaryInst*>(instruction.get());
            std::string operands = asmOperand(binaryInst->lhs) + ", " + asmOperand(binaryInst->rhs);
            result += asmFormatInstruction(asmBinaryOperator(binaryInst->oper, binaryInst->type), operands);
            return;
        }
        case Inst::Kind::Cdq: {
            const auto cdqInst = dynamic_cast<CdqInst*>(instruction.get());
            if (cdqInst->type == AssemblyType::LongWord)
                result += asmFormatInstruction("cdq");
            if (cdqInst->type == AssemblyType::QuadWord)
                result += asmFormatInstruction("cqo");
            return;
        }
        case Inst::Kind::Idiv: {
            const auto idivInst = dynamic_cast<IdivInst*>(instruction.get());
            if (idivInst->type == AssemblyType::LongWord)
                result += asmFormatInstruction("idivl", asmOperand(idivInst->operand));
            if (idivInst->type == AssemblyType::QuadWord)
                result += asmFormatInstruction("idivq", asmOperand(idivInst->operand));
            return;
        }
        case Inst::Kind::Div: {
            const auto divInst = dynamic_cast<DivInst*>(instruction.get());
            if (divInst->type == AssemblyType::LongWord)
                result += asmFormatInstruction("divl", asmOperand(divInst->operand));
            if (divInst->type == AssemblyType::QuadWord)
                result += asmFormatInstruction("divq", asmOperand(divInst->operand));
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
            if (cmpInst->lhs->type == AssemblyType::LongWord)
                result += asmFormatInstruction("cmpl", operands);
            if (cmpInst->lhs->type == AssemblyType::QuadWord)
                result += asmFormatInstruction("cmpq", operands);
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
            if (immOperand->type == AssemblyType::LongWord && immOperand->isSigned)
                return "$" + std::to_string(std::get<i32>(immOperand->value));
            if (immOperand->type == AssemblyType::QuadWord && immOperand->isSigned)
                return "$" + std::to_string(std::get<i64>(immOperand->value));
            if (immOperand->type == AssemblyType::LongWord && !immOperand->isSigned)
                return "$" + std::to_string(std::get<u32>(immOperand->value));
            return "$" + std::to_string(std::get<u64>(immOperand->value));
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
    using Type = RegisterOperand::Kind;
    static const std::unordered_map<Type, std::array<const char*, 4>> registerMap = {
        {Type::AX,  {"%al",   "%ax",   "%eax",  "%rax"}},
        {Type::CX,  {"%cl",   "%cx",   "%ecx",  "%rcx"}},
        {Type::DX,  {"%dl",   "%dx",   "%edx",  "%rdx"}},
        {Type::DI,  {"%dil",  "%di",   "%edi",  "%rdi"}},
        {Type::SI,  {"%sil",  "%si",   "%esi",  "%rsi"}},
        {Type::R8,  {"%r8b",  "%r8w",  "%r8d",  "%r8"}},
        {Type::R9,  {"%r9b",  "%r9w",  "%r9d",  "%r9"}},
        {Type::R10, {"%r10b", "%r10w", "%r10d", "%r10"}},
        {Type::R11, {"%r11b", "%r11w", "%r11d", "%r11"}},
        {Type::SP,  {"%rsp",  "%rsp",  "%rsp",  "%rsp"}}
    };

    auto it = registerMap.find(reg->kind);
    if (it == registerMap.end())
        return "invalid_register";

    const auto& names = it->second;
    switch (reg->type) {
        case AssemblyType::Byte:     return names[0];
        case AssemblyType::Word:     return names[1];
        case AssemblyType::LongWord: return names[2];
        case AssemblyType::QuadWord: return names[3];
        default: return "invalid_size";
    }
}

std::string asmUnaryOperator(const UnaryInst::Operator oper, AssemblyType type)
{
    using Operator = UnaryInst::Operator;
    if (type == AssemblyType::LongWord) {
        switch (oper) {
            case Operator::Neg:      return "negl";
            case Operator::Not:      return "notl";
            default:                 return "not set asmUnaryOperator";
        }
    }
    if (type == AssemblyType::QuadWord) {
        switch (oper) {
            case Operator::Neg:      return "negq";
            case Operator::Not:      return "notq";
            default:                 return "not set asmUnaryOperator";
        }
    }
    std::abort();
}

std::string asmBinaryOperator(const BinaryInst::Operator oper, const AssemblyType type)
{
    using Operator = BinaryInst::Operator;
    if (type == AssemblyType::LongWord) {
        switch (oper) {
            case Operator::Mul:                 return "imull";
            case Operator::Add:                 return "addl";
            case Operator::Sub:                 return "subl";

            case Operator::BitwiseAnd:          return "andl";
            case Operator::BitwiseOr:           return "orl";
            case Operator::BitwiseXor:          return "xorl";

            case Operator::LeftShiftSigned:     return "shll";
            case Operator::LeftShiftUnsigned:   return "sall";
            case Operator::RightShiftSigned:    return "sarl";
            case Operator::RightShiftUnsigned:  return "shrl";
            default:
                return "not set asmBinaryOperator";
        }
    }
    if (type == AssemblyType::QuadWord) {
        switch (oper) {
            case Operator::Mul:                 return "imulq";
            case Operator::Add:                 return "addq";
            case Operator::Sub:                 return "subq";

            case Operator::BitwiseAnd:          return "andq";
            case Operator::BitwiseOr:           return "orq";
            case Operator::BitwiseXor:          return "xorq";

            case Operator::LeftShiftSigned:     return "shlq";
            case Operator::LeftShiftUnsigned:   return "salq";
            case Operator::RightShiftSigned:    return "sarq";
            case Operator::RightShiftUnsigned:  return "shrq";
            default:
                return "not set asmBinaryOperator";
        }
    }
    std::abort();
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
    using CondCode = BinaryInst::CondCode;
    switch (condCode) {
        case CondCode::E:   return "e";
        case CondCode::NE:  return "ne";
        case CondCode::L:   return "l";
        case CondCode::LE:  return "le";
        case CondCode::G:   return "g";
        case CondCode::GE:  return "ge";
        case CondCode::A:   return "a";
        case CondCode::AE:  return "ae";
        case CondCode::B:   return "b";
        case CondCode::BE:  return "be";
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
