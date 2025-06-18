#include "Assembly.hpp"

#include <array>
#include <string>
#include <iomanip>
#include <unordered_map>

namespace CodeGen {

std::string asmProgram(const Program& program)
{
    std::string result;
    for (const std::unique_ptr<TopLevel>& topLevel : program.topLevels) {
        if (topLevel->type == TopLevel::Kind::StaticVariable) {
            const auto var = static_cast<const StaticVariable*>(topLevel.get());
            asmStaticVariable(result, *var);
            continue;
        }
        if (topLevel->type == TopLevel::Kind::StaticConstant) {
            const auto var = static_cast<const ConstVariable*>(topLevel.get());
            asmStaticConstant(result, *var);
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
    if (variable.type == AsmType::LongWord)
        return asmStaticVariableLong(result, variable);
    if (variable.type == AsmType::QuadWord)
        return asmStaticVariableQuad(result, variable);
    if (variable.type == AsmType::Double)
        return asmStaticVariableDouble(result, variable);
}

void asmStaticVariableLong(std::string& result, const StaticVariable& variable)
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
        result += asmFormatInstruction(".zero 4");
    if (variable.init != 0)
        result += asmFormatInstruction(".long " + std::to_string(variable.init));
    result += '\n';
}

void asmStaticVariableQuad(std::string& result, const StaticVariable& variable)
{
    if (variable.global)
        result += asmFormatInstruction(".globl", variable.name);
    if (variable.init == 0)
        result += asmFormatInstruction(".bss");
    else
        result += asmFormatInstruction(".data");
    result += asmFormatInstruction(".align","8");
    result += asmFormatLabel(variable.name);
    if (variable.init == 0)
        result += asmFormatInstruction(".zero 8");
    if (variable.init != 0)
        result += asmFormatInstruction(".quad " + std::to_string(variable.init));
    result += '\n';
}

void asmStaticVariableDouble(std::string& result, const StaticVariable& variable)
{
    if (variable.global)
        result += asmFormatInstruction(".globl", variable.name);
    result += asmFormatInstruction(".data");
    result += asmFormatInstruction(".align","8");
    result += asmFormatLabel(variable.name);
    result += asmFormatInstruction(".quad " + std::to_string(variable.init));
    result += '\n';
}

void asmStaticConstant(std::string& result, const ConstVariable& variable)
{
    result += asmFormatInstruction(".section .rodata");
    result += asmFormatInstruction(".align",std::to_string(variable.alignment));
    if (variable.local)
        result += asmFormatLabel(createLabel(variable.name.value));
    else
        result += asmFormatLabel(variable.name.value);
    result += asmFormatInstruction(".quad "+ std::to_string( std::bit_cast<u64>(variable.staticInit))
                + " # " + std::to_string(variable.staticInit));
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
            const std::string operand = asmOperand(moveInst->src) + ", " + asmOperand(moveInst->dst);
            result += asmFormatInstruction(addType("mov", moveInst->type), operand);
            return;
        }
        case Inst::Kind::MoveSX: {
            const auto moveSXInst = dynamic_cast<MoveSXInst*>(instruction.get());
            const std::string operands = asmOperand(moveSXInst->src) + ", " + asmOperand(moveSXInst->dst);
            result += asmFormatInstruction("movslq", operands);
            return;
        }
        case Inst::Kind::MovZeroExtend: {
            const auto moveZeroExtend = dynamic_cast<MoveZeroExtendInst*>(instruction.get());
            const std::string operands = asmOperand(moveZeroExtend->src) + ", " + asmOperand(moveZeroExtend->dst);
            result += asmFormatInstruction(addType("mov", moveZeroExtend->type), operands);
            return;
        }
        case Inst::Kind::Cvtsi2sd: {
            const auto cvtsi2sd = dynamic_cast<Cvtsi2sdInst*>(instruction.get());
            const AsmType type = cvtsi2sd->srcType;
            const std::string operands = asmOperand(cvtsi2sd->src) + ", " + asmOperand(cvtsi2sd->dst);
            result += asmFormatInstruction(addType("cvtsi2sd", type), operands);
            return;
        }
        case Inst::Kind::Cvttsd2si: {
            const auto cvtsd2siInst = dynamic_cast<Cvttsd2siInst*>(instruction.get());
            const AsmType type = cvtsd2siInst->dstType;
            const std::string operands = asmOperand(cvtsd2siInst->src) + ", " + asmOperand(cvtsd2siInst->dst);
            result += asmFormatInstruction(addType("cvttsd2si", type), operands);
            return;
        }
        case Inst::Kind::Unary: {
            const auto unaryInst = dynamic_cast<UnaryInst*>(instruction.get());
            result += asmFormatInstruction(
                asmUnaryOperator(unaryInst->oper, unaryInst->type),
                asmOperand(unaryInst->destination));
            return;
        }
        case Inst::Kind::Binary: {
            const auto binaryInst = dynamic_cast<BinaryInst*>(instruction.get());
            const std::string operands = asmOperand(binaryInst->lhs) + ", " + asmOperand(binaryInst->rhs);
            result += asmFormatInstruction(asmBinaryOperator(binaryInst->oper, binaryInst->type), operands);
            return;
        }
        case Inst::Kind::Cdq: {
            const auto cdqInst = dynamic_cast<CdqInst*>(instruction.get());
            if (cdqInst->type == AsmType::LongWord)
                result += asmFormatInstruction("cdq");
            if (cdqInst->type == AsmType::QuadWord)
                result += asmFormatInstruction("cqo");
            return;
        }
        case Inst::Kind::Idiv: {
            const auto idivInst = dynamic_cast<IdivInst*>(instruction.get());
            result += asmFormatInstruction(addType(
                "idiv", idivInst->type), asmOperand(idivInst->operand));
            return;
        }
        case Inst::Kind::Div: {
            const auto divInst = dynamic_cast<DivInst*>(instruction.get());
            if (divInst->type == AsmType::LongWord)
                result += asmFormatInstruction("divl", asmOperand(divInst->operand));
            if (divInst->type == AsmType::QuadWord)
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
            if (cmpInst->lhs->type == AsmType::Double)
                result += asmFormatInstruction("comisd", operands);
            else
                result += asmFormatInstruction(addType("cmp", cmpInst->lhs->type), operands);
            return;
        }
        case Inst::Kind::Jmp: {
            const auto jmpInst = dynamic_cast<JmpInst*>(instruction.get());
            result += asmFormatInstruction("jmp", createLabel(jmpInst->target.value));
            return;
        }
        case Inst::Kind::JmpCC: {
            const auto jmpCCInst = dynamic_cast<JmpCCInst*>(instruction.get());
            result += asmFormatInstruction(
                "j" + condCode(jmpCCInst->condition), createLabel(jmpCCInst->target.value));
            return;
        }
        case Inst::Kind::SetCC: {
            const auto setCCInst = dynamic_cast<SetCCInst*>(instruction.get());
            result += asmFormatInstruction(
                "set" + condCode(setCCInst->condition), asmOperand(setCCInst->operand));
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
        case Operand::Kind::Data: {
            const auto dataOperand = dynamic_cast<DataOperand*>(operand.get());
            if (dataOperand->local && dataOperand->type == AsmType::Double)
                return createLabel(dataOperand->identifier.value) + "(%rip)";
            return dataOperand->identifier.value + "(%rip)";
        }
        default:
            return "not set asmOperand";
    }
}

std::string asmRegister(const RegisterOperand* reg)
{
    using Type = RegisterOperand::Kind;
    switch (reg->kind) {
        case Type::XMM0: return "%xmm0";
        case Type::XMM1: return "%xmm1";
        case Type::XMM2: return "%xmm2";
        case Type::XMM3: return "%xmm3";
        case Type::XMM4: return "%xmm4";
        case Type::XMM5: return "%xmm5";
        case Type::XMM6: return "%xmm6";
        case Type::XMM7: return "%xmm7";
        case Type::XMM14: return "%xmm14";
        case Type::XMM15: return "%xmm15";
            default:
            break;
    }
    static const std::unordered_map<Type, std::array<std::string, 4>> registerMap = {
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

    const auto it = registerMap.find(reg->kind);
    if (it == registerMap.end())
        return "invalid_register";

    const auto& names = it->second;
    switch (reg->type) {
        case AsmType::Byte:     return names[0];
        case AsmType::Word:     return names[1];
        case AsmType::LongWord: return names[2];
        case AsmType::QuadWord: return names[3];
        default: return "invalid_size";
    }
}

std::string asmUnaryOperator(const UnaryInst::Operator oper, const AsmType type)
{
    using Operator = UnaryInst::Operator;
    switch (oper) {
        case Operator::Neg:      return addType("neg", type);
        case Operator::Not:      return addType("not", type);
        case Operator::Shr:      return addType("shr", type);
        default:                 return "not set asmUnaryOperator";
    }
}

std::string asmBinaryOperator(const BinaryInst::Operator oper, const AsmType type)
{
    using Operator = BinaryInst::Operator;
    if (oper == Operator::BitwiseXor && type == AsmType::Double)
        return "xorpd";
    if (oper == Operator::Mul && type == AsmType::Double)
        return "mulsd";
    if (oper == Operator::DivDouble && type == AsmType::Double)
        return "divsd";
    switch (oper) {
        case Operator::Mul:                 return addType("imul", type);
        case Operator::Add:                 return addType("add", type);
        case Operator::Sub:                 return addType("sub", type);

        case Operator::AndBitwise:          return addType("and", type);
        case Operator::OrBitwise:           return addType("or", type);
        case Operator::BitwiseXor:          return addType("xor" ,type);

        case Operator::LeftShiftSigned:     return addType("shl", type);
        case Operator::LeftShiftUnsigned:   return addType("sal", type);
        case Operator::RightShiftSigned:    return addType("sar", type);
        case Operator::RightShiftUnsigned:  return addType("shr", type);
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
        case CondCode::PF:  return "p";
        default:
            return "not set condCode";
    }
}

std::string asmFormatInstruction(const std::string& mnemonic,
                      const std::string& operands,
                      const std::string& comment)
{
    constexpr int mnemonicWidth = 12;
    constexpr int operandsWidth = 16;

    std::ostringstream oss;
    oss << "    " << std::left << std::setw(mnemonicWidth) << mnemonic
        << std::setw(operandsWidth) << operands;
    if (!comment.empty())
        oss << "# " << comment;
    oss << "\n";
    return oss.str();
}

std::string addType(const std::string& instruction, const AsmType type)
{
    switch (type) {
        case AsmType::LongWord:
            return instruction + "l";
        case AsmType::QuadWord:
            return instruction + "q";
        case AsmType::Double:
            return instruction + "sd";
        default:
            return instruction + "not set addType";
    }
}

} // CodeGen