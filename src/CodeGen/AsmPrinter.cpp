#include "AsmPrinter.hpp"
#include "Assembly.hpp"
#include "ASTIr.hpp"

#include <iomanip>

#include "DynCast.hpp"

namespace CodeGen {
std::string AsmPrinter::printProgram(const Program &program)
{
    m_indentLevel = 0;
    for (const auto& topLevel: program.topLevels)
        add(*topLevel);
    return m_oss.str();
}

void AsmPrinter::add(const TopLevel& topLevel)
{
    using Type = TopLevel::Kind;
    switch (topLevel.kind) {
        case Type::Function:
            add(*dynCast<const Function>(&topLevel));
            break;
        case Type::StaticVariable:
            add(*dynCast<const StaticVariable>(&topLevel));
            break;
        case Type::StaticConstant:
            add(*dynCast<const ConstVariable>(&topLevel));
            break;
        default:
            addLine("Unknown Instruction");
    }
}

void AsmPrinter::add(const StaticVariable& staticVariable)
{
    std::string global;
    if (staticVariable.global)
        global = "is global";
    else
        global = "is not global";
    addLine(staticVariable.name + " " + std::to_string(staticVariable.init) + " " + global);
}

void AsmPrinter::add(const ConstVariable& constVariable)
{
    addLine(constVariable.name.value + " " + std::to_string(constVariable.staticInit) + " " +
            std::to_string(constVariable.alignment));
}

void AsmPrinter::add(const Function& function)
{
    IndentGuard indent(m_indentLevel);
    std::string global;
    if (function.isGlobal)
        global = "is global";
    else
        global = "is not global";
    addLine(function.name + " " + global);
    for (const auto& inst : function.instructions)
        add(*inst);
}

void AsmPrinter::add(const Inst& inst)
{
    using Kind = Inst::Kind;
    IndentGuard indent(m_indentLevel);
    switch (inst.kind) {
        case Kind::Move:
            add(*dynCast<const MoveInst>(&inst)); break;
        case Kind::MoveSX:
            add(*dynCast<const MoveSXInst>(&inst)); break;
        case Kind::MoveZeroExtend:
            add(*dynCast<const MoveZeroExtendInst>(&inst)); break;
        case Kind::Lea:
            add(*dynCast<const LeaInst>(&inst)); break;
        case Kind::Cvtsi2sd:
            add(*dynCast<const Cvtsi2sdInst>(&inst)); break;
        case Kind::Cvttsd2si:
            add(*dynCast<const Cvttsd2siInst>(&inst)); break;
        case Kind::Unary:
            add(*dynCast<const UnaryInst>(&inst)); break;
        case Kind::Binary:
            add(*dynCast<const BinaryInst>(&inst)); break;
        case Kind::Cmp:
            add(*dynCast<const CmpInst>(&inst)); break;
        case Kind::Idiv:
            add(*dynCast<const IdivInst>(&inst)); break;
        case Kind::Div:
            add(*dynCast<const DivInst>(&inst)); break;
        case Kind::Cdq:
            add(*dynCast<const CdqInst>(&inst)); break;
        case Kind::Jmp:
            add(*dynCast<const JmpInst>(&inst)); break;
        case Kind::JmpCC:
            add(*dynCast<const JmpCCInst>(&inst)); break;
        case Kind::SetCC:
            add(*dynCast<const SetCCInst>(&inst)); break;
        case Kind::Label:
            add(*dynCast<const LabelInst>(&inst)); break;
        case Kind::Push:
            add(*dynCast<const PushInst>(&inst)); break;
        case Kind::Call:
            add(*dynCast<const CallInst>(&inst)); break;
        case Kind::Ret:
            add(*dynCast<const ReturnInst>(&inst)); break;
        default:
            addLine("Unknown Instruction");
    }
}

void AsmPrinter::add(const MoveInst& move)
{
    addLine("MoveInst: ", to_string(*move.src) + " " + to_string(*move.dst));
}

void AsmPrinter::add(const MoveSXInst& moveSX)
{
    addLine("MoveSXInst: ",
        to_string(*moveSX.src) + " " +
        to_string(*moveSX.dst));
}

void AsmPrinter::add(const MoveZeroExtendInst& moveZeroExtend)
{
    addLine("moveZeroExtendInst: ",
            to_string(*moveZeroExtend.src) + " " +
            to_string(*moveZeroExtend.dst));
}

void AsmPrinter::add(const LeaInst& lea)
{
    addLine("LeaInst: ", to_string(*lea.src) + " " + to_string(*lea.dst));
}

void AsmPrinter::add(const UnaryInst& unary)
{
    addLine("Unary: ",
            to_string(unary.oper) + " " +
            to_string(*unary.destination));
}

void AsmPrinter::add(const BinaryInst& binary)
{
    addLine("Binary: ",
            to_string(*binary.lhs) + " " +
            to_string(binary.oper) + " " +
            to_string(*binary.rhs));
}

void AsmPrinter::add(const CmpInst& cmp)
{
    addLine("Cmp: ",
            to_string(*cmp.lhs) + " " +
            to_string(*cmp.rhs));
}

void AsmPrinter::add(const IdivInst& idiv)
{
    addLine("Idiv: ", to_string(*idiv.operand));
}

void AsmPrinter::add(const DivInst& div)
{
    addLine("Div: ", to_string(*div.operand));
}

void AsmPrinter::add(const CdqInst& cpq)
{
    addLine("Cdq");
}

void AsmPrinter::add(const JmpInst& jmp)
{
    addLine("Jmp: ", to_string(jmp.target));
}

void AsmPrinter::add(const JmpCCInst& jmpCC)
{
    addLine("JmpCC: ",
            to_string(jmpCC.target) + " " +
            to_string(jmpCC.condition));
}

void AsmPrinter::add(const SetCCInst& setCC)
{
    addLine("SetCC: ",
            to_string(*setCC.operand) + " " +
            to_string(setCC.condition));
}

void AsmPrinter::add(const LabelInst& label)
{
    addLine("Label: ",
            to_string(label.target));
}

void AsmPrinter::add(const PushInst& push)
{
    addLine("Push: ",  to_string(*push.operand));
}

void AsmPrinter::add(const CallInst& call)
{
    addLine("Call: ", to_string(call.funName));
}

void AsmPrinter::add(const ReturnInst& returnInst)
{
    addLine("Return: \n");
}

void AsmPrinter::add(const Cvtsi2sdInst& cvtsi2sd)
{
    addLine("Cvtsi2sd",
    to_string(*cvtsi2sd.src) +  " " +
            to_string(*cvtsi2sd.dst) + " " +
            to_string(cvtsi2sd.srcType));
}

void AsmPrinter::add(const Cvttsd2siInst& cvttsd2si)
{
    addLine("Cvttsd2si",
            to_string(*cvttsd2si.src) + " " +
            to_string(*cvttsd2si.dst) + " " +
            to_string(cvttsd2si.dstType));
}

std::string to_string(const Identifier& identifier)
{
    return identifier.value;
}

std::string to_string(const Operand& operand)
{
    using Kind = Operand::Kind;
    switch (operand.kind) {
        case Kind::Imm:         return to_string(*dynCast<const ImmOperand>(&operand));
        case Kind::Register:    return to_string(*dynCast<const RegisterOperand>(&operand));
        case Kind::Pseudo:      return to_string(*dynCast<const PseudoOperand>(&operand));
        case Kind::PseudoMem:   return to_string(*dynCast<const PseudoMemOperand>(&operand));
        case Kind::Memory:      return to_string(*dynCast<const MemoryOperand>(&operand));
        case Kind::Data:        return to_string(*dynCast<const DataOperand>(&operand));
        case Kind::Indexed:     return to_string(*dynCast<const IndexedOperand>(&operand));
        default:
            std::unreachable();
    }
}

std::string to_string(const ImmOperand& immOperand)
{
    return "ImmOperand(" + std::to_string(immOperand.value) + ", " + to_string(immOperand.type) + ")";
}

std::string to_string(const RegisterOperand& registerOperand)
{
    return "Register(" + to_string(registerOperand.regKind) + ", " + to_string(registerOperand.type) + ")";
}

std::string to_string(const PseudoOperand& pseudoOperand)
{
    return "Pseudo(" + pseudoOperand.identifier.value + ", " + to_string(pseudoOperand.type) + ")";
}

std::string to_string(const PseudoMemOperand& pseudoMemOperand)
{
    return "PseudoMem(" + pseudoMemOperand.identifier.value + ", " +
                          to_string(pseudoMemOperand.type) + ", " +
                          std::to_string(pseudoMemOperand.size)  + ")";
}

std::string to_string(const MemoryOperand& memoryOperand)
{
    return "Memory(" + std::to_string(memoryOperand.value) + ", " + to_string(memoryOperand.regKind) + ", "
            + to_string(memoryOperand.type) + ")";
}

std::string to_string(const DataOperand& dataOperand)
{
    return "Data(" + dataOperand.identifier.value + ", " + to_string(dataOperand.type) + ")";
}

std::string to_string(const IndexedOperand& indexedOperand)
{
    return "Indexed(" + to_string(indexedOperand.regKind) + ", " +
                        to_string(indexedOperand.indexRegKind) + ", " +
                        std::to_string(indexedOperand.scale) + ", " +
                        to_string(indexedOperand.type) + ")";
}

std::string to_string(const Operand::RegKind regType)
{
    using Type = Operand::RegKind;
    switch (regType) {
        case Type::AX:    return "AX";
        case Type::CX:    return "CX";
        case Type::DX:    return "DX";
        case Type::DI:    return "DI";
        case Type::SI:    return "SI";
        case Type::R8:    return "R8";
        case Type::R9:    return "R9";
        case Type::R10:   return "R10";
        case Type::R11:   return "R11";
        case Type::SP:    return "SP";
        case Type::BP:    return "BP";
        case Type::XMM0:  return "XMM0";
        case Type::XMM1:  return "XMM1";
        case Type::XMM2:  return "XMM2";
        case Type::XMM3:  return "XMM3";
        case Type::XMM4:  return "XMM4";
        case Type::XMM5:  return "XMM5";
        case Type::XMM6:  return "XMM6";
        case Type::XMM7:  return "XMM7";
        case Type::XMM14: return "XMM14";
        case Type::XMM15: return "XMM15";
        default:          return "UnknownRegister";
    }
}

std::string to_string(const UnaryInst::Operator& oper)
{
    using Oper = UnaryInst::Operator;
    switch (oper) {
        case Oper::Neg:     return "-";
        case Oper::Not:     return "!";
        default:            return "Unknown UnaryOp";
    }
}

std::string to_string(const BinaryInst::Operator& oper)
{
    using Oper = BinaryInst::Operator;
    switch (oper) {
        case Oper::Add:              return "+";
        case Oper::Sub:              return "-";
        case Oper::Mul:              return "*";
        case Oper::DivDouble:        return "/";
        case Oper::BitwiseAnd:       return "&";
        case Oper::BitwiseOr:        return "|";
        case Oper::BitwiseXor:       return "^";
        case Oper::LeftShiftSigned:  return "<<";
        case Oper::RightShiftSigned: return ">>";
        default:                     return "Unknown BinaryOp";
    }
}

std::string to_string(const Inst::CondCode& condCode)
{
    using CondCode = Inst::CondCode;
    switch (condCode) {
        case CondCode::E:       return "E";
        case CondCode::NE:      return "NE";
        case CondCode::L:       return "L";
        case CondCode::LE:      return "LE";
        case CondCode::G:       return "G";
        case CondCode::GE:      return "GE";
        case CondCode::A:       return "A";
        case CondCode::AE:      return "AE";
        case CondCode::B:       return "B";
        case CondCode::BE:      return "BE";
        case CondCode::PF:      return "PF";
        default:                return "Unknown CondCode";
    }
}

std::string to_string(const AsmType type)
{
    switch (type) {
        case AsmType::Byte:       return "Byte";
        case AsmType::Word:       return "Word";
        case AsmType::LongWord:   return "LongWord";
        case AsmType::QuadWord:   return "QuadWord";
        case AsmType::Double:     return "Double";
        default:                  return "Unknown AssemblyType";
    }
}

void AsmPrinter::addLine(const std::string& name,
                         const std::string& operands)
{
    constexpr i32 mnemonicWidth = 14;
    constexpr i32 operandsWidth = 50;
    std::ostringstream oss;
    oss << getIndent();
    oss << std::left << std::setw(mnemonicWidth) << name
        << std::setw(operandsWidth) << operands;
    oss << "\n";
    m_oss << oss.str();
}

std::string AsmPrinter::getIndent() const
{
    return std::string(m_indentLevel * c_indentMult, ' ');
}
} // CodeGen