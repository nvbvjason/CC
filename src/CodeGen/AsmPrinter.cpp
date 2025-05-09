#include "AsmPrinter.hpp"

#include <iomanip>

#include "Assembly.hpp"
#include "ASTIr.hpp"

namespace CodeGen {
std::string AsmPrinter::printProgram(const Program &program)
{
    m_indentLevel = 0;
    for (const auto& function: program.functions)
        add(*function);
    return m_oss.str();
}

void AsmPrinter::add(const Function& function)
{
    IndentGuard indent(m_indentLevel);
    addLine(function.name);
    for (const auto& inst : function.instructions)
        add(*inst);
}

void AsmPrinter::add(const Inst& inst)
{
    using Kind = Inst::Kind;
    IndentGuard indent(m_indentLevel);
    switch (inst.kind) {
        case Kind::Move:
            add(static_cast<const MoveInst&>(inst)); break;
        case Kind::Unary:
            add(static_cast<const UnaryInst&>(inst)); break;
        case Kind::Binary:
            add(static_cast<const BinaryInst&>(inst)); break;
        case Kind::Cmp:
            add(static_cast<const CmpInst&>(inst)); break;
        case Kind::Idiv:
            add(static_cast<const IdivInst&>(inst)); break;
        case Kind::Cdq:
            add(static_cast<const CdqInst&>(inst)); break;
        case Kind::Jmp:
            add(static_cast<const JmpInst&>(inst)); break;
        case Kind::JmpCC:
            add(static_cast<const JmpCCInst&>(inst)); break;
        case Kind::SetCC:
            add(static_cast<const SetCCInst&>(inst)); break;
        case Kind::Label:
            add(static_cast<const LabelInst&>(inst)); break;
        case Kind::AllocateStack:
            add(static_cast<const AllocStackInst&>(inst)); break;
        case Kind::DeallocateStack:
            add(static_cast<const DeallocStackInst&>(inst)); break;
        case Kind::Push:
            add(static_cast<const PushInst&>(inst)); break;
        case Kind::Call:
            add(static_cast<const CallInst&>(inst)); break;
        case Kind::Ret:
            add(static_cast<const ReturnInst&>(inst)); break;
        default:
            addLine("Unknown Instruction");
    }
}

void AsmPrinter::add(const MoveInst& moveInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("MoveInst: ", to_string(*moveInst.src) + " " + to_string(*moveInst.dst));
}

void AsmPrinter::add(const UnaryInst& unaryInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Unary: ",
            to_string(unaryInst.oper) + " " +
            to_string(*unaryInst.destination));
}

void AsmPrinter::add(const BinaryInst& binaryInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Binary: ",
            to_string(*binaryInst.lhs) + " " +
            to_string(binaryInst.oper) + " " +
            to_string(*binaryInst.rhs));
}

void AsmPrinter::add(const CmpInst& cmpInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Cmp: ",
            to_string(*cmpInst.lhs) + " " +
            to_string(*cmpInst.rhs));
}

void AsmPrinter::add(const IdivInst& idivInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Idiv: ",
            to_string(*idivInst.operand));
}

void AsmPrinter::add(const CdqInst& cpqInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Cdq");
}

void AsmPrinter::add(const JmpInst& jmpInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Jmp: ",
            to_string(jmpInst.target));
}

void AsmPrinter::add(const JmpCCInst& jmpCCInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("JmpCC: ",
            to_string(jmpCCInst.target) + " " +
            to_string(jmpCCInst.condition));
}

void AsmPrinter::add(const SetCCInst& setCCInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("SetCC: ",
            to_string(*setCCInst.operand) + " " +
            to_string(setCCInst.condition));
}

void AsmPrinter::add(const LabelInst& labelInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Label: ",
            to_string(labelInst.target));
}

void AsmPrinter::add(const AllocStackInst& allocStackInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("AllocStack(" + std::to_string(allocStackInst.alloc) + ")");
}

void AsmPrinter::add(const DeallocStackInst& deallocStackInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("DeallocStack(" + std::to_string(deallocStackInst.dealloc) + ")");
}

void AsmPrinter::add(const PushInst& pushInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Push: " + to_string(*pushInst.operand));
}

void AsmPrinter::add(const CallInst& callInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Call: " + to_string(callInst.funName));
}

void AsmPrinter::add(const ReturnInst& returnInst)
{
    IndentGuard indent(m_indentLevel);
    addLine("Return: ");
}

std::string add(const Identifier& identifier)
{
    return identifier.value;
}

std::string add(const Operand& operand)
{
    using Kind = Operand::Kind;
    switch (operand.kind) {
        case Kind::Imm:
            return add(static_cast<const ImmOperand&>(operand));
        case Kind::Register:
            return add(static_cast<const RegisterOperand&>(operand));
        case Kind::Pseudo:
            return add(static_cast<const PseudoOperand&>(operand));
        case Kind::Stack:
            return add(static_cast<const StackOperand&>(operand));
        default:
            std::unreachable();
    }
}

std::string add(const ImmOperand& immOperand)
{
    return "Imm(" + std::to_string(immOperand.value) + ")";
}

std::string add(const RegisterOperand& registerOperand)
{
    return "Register(" + std::to_string(registerOperand.size) + ")";
}

std::string add(const PseudoOperand& pseudoOperand)
{
    return "Pseudo(" + pseudoOperand.identifier + ")";
}

std::string add(const StackOperand& stackOperand)
{
    return "Stack(" + std::to_string(stackOperand.value) + ")";
}

std::string add(const RegisterOperand::Type& type)
{
    using Type = RegisterOperand::Type;
    switch (type) {
        case Type::AX:          return "AX";
        case Type::CX:          return "CX";
        case Type::DX:          return "DX";
        case Type::DI:          return "DI";
        case Type::SI:          return "SI";
        case Type::R8:          return "R8";
        case Type::R9:          return "R9";
        case Type::R10:         return "R10";
        case Type::R11:         return "R11";
        default:                return "UnknownRegister";
    }
}

std::string add(const UnaryInst::Operator& oper)
{
    using Oper = UnaryInst::Operator;
    switch (oper) {
        case Oper::Neg:     return "!";
        case Oper::Not:     return "~";
        default:            return "UnknownUnaryOp";
    }
}

std::string add(const BinaryInst::Operator& oper)
{
    using Oper = BinaryInst::Operator;
    switch (oper) {
        case Oper::Add:             return "+";
        case Oper::Sub:             return "-";
        case Oper::Mul:             return "*";
        case Oper::BitwiseAnd:      return "&";
        case Oper::BitwiseOr:       return "|";
        case Oper::BitwiseXor:      return "^";
        case Oper::LeftShift:       return "<<";
        case Oper::RightShift:      return ">>";
        default:                    return "UnknownBinaryOp";
    }
}

std::string add(const Inst::CondCode& condCode)
{
    using CondCode = Inst::CondCode;
    switch (condCode) {
        case CondCode::E:       return "E";
        case CondCode::NE:      return "NE";
        case CondCode::L:       return "L";
        case CondCode::LE:      return "LE";
        case CondCode::G:       return "G";
        case CondCode::GE:      return "GE";
        default:                return "UnknownCondCode";
    }

}

void AsmPrinter::addLine(const std::string& name,
                         const std::string& operands)
{
    constexpr int mnemonicWidth = 8;
    constexpr int operandsWidth = 50;

    m_oss << "    " << std::left << std::setw(mnemonicWidth) << name
          << std::setw(operandsWidth) << operands;
    m_oss << "\n";
}

std::string AsmPrinter::getIndent() const
{
    return std::string(m_indentLevel * c_indentMult, ' ');
}
} // CodeGen
