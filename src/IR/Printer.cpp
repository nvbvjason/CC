#include "Printer.hpp"
#include <sstream>

namespace Ir {

std::string Printer::print(const Program& program)
{
    m_oss << "Program:\n";
    indentLevel++;
    if (program.function)
        print(*program.function);
    else {
        printIndent();
        m_oss << "<empty>\n";
    }
    indentLevel--;
    return m_oss.str();
}

void Printer::print(const Function& function)
{
    printIndent();
    m_oss << "Function " << function.name << ":\n";
    indentLevel++;
    for (const auto& inst : function.insts)
        print(*inst);
    indentLevel--;
}


void Printer::print(const Value& value)
{
    switch (value.type) {
        case Value::Type::Variable:
            visit(static_cast<const ValueVar&>(value)); break;
        case Value::Type::Constant:
            visit(static_cast<const ValueConst&>(value)); break;
    }
}

void Printer::print(const Identifier& identifier)
{
    m_oss << identifier.value;
}

// Instruction visitors
void Printer::visit(const ReturnInst& inst)
{
    m_oss << "Return ";
    print(*inst.returnValue);
    m_oss << "\n";
}

void Printer::visit(const UnaryInst& inst)
{
    m_oss << to_string(inst.operation) << " ";
    print(*inst.source);
    m_oss << " -> ";
    print(*inst.destination);
    m_oss << "\n";
}

void Printer::visit(const BinaryInst& inst)
{
    m_oss << to_string(inst.operation) << " ";
    print(*inst.source1);
    m_oss << ", ";
    print(*inst.source2);
    m_oss << " -> ";
    print(*inst.destination);
    m_oss << "\n";
}

void Printer::visit(const CopyInst& inst)
{
    m_oss << "Copy ";
    print(*inst.source);
    m_oss << " -> ";
    print(*inst.destination);
    m_oss << "\n";
}

void Printer::visit(const JumpInst& inst)
{
    m_oss << "Jump ";
    print(inst.target);
    m_oss << "\n";
}

void Printer::visit(const JumpIfZeroInst& inst)
{
    m_oss << "JumpIfZero ";
    print(*inst.condition);
    m_oss << ", ";
    print(inst.target);
    m_oss << "\n";
}

void Printer::visit(const JumpIfNotZeroInst& inst)
{
    m_oss << "JumpIfNotZero ";
    print(*inst.condition);
    m_oss << ", ";
    print(inst.target);
    m_oss << "\n";
}

void Printer::visit(const LabelInst& inst)
{
    m_oss << "Label ";
    print(inst.target);
    m_oss << ":\n";
}

// Value visitors
void Printer::visit(const ValueVar& val)
{
    m_oss << "Var(";
    print(val.value);
    m_oss << ")";
}

void Printer::visit(const ValueConst& val)
{
    m_oss << "Const(" << val.value << ")";
}

std::string to_string(UnaryInst::Operation op)
{
    switch (op) {
        case UnaryInst::Operation::Complement: return "Complement";
        case UnaryInst::Operation::Negate: return "Negate";
        case UnaryInst::Operation::Not: return "Not";
    }
    return "UnknownUnaryOp";
}

std::string to_string(BinaryInst::Operation op)
{
    switch (op) {
        case BinaryInst::Operation::Add:            return "Add";
        case BinaryInst::Operation::Subtract:       return "Subtract";
        case BinaryInst::Operation::Multiply:       return "Multiply";
        case BinaryInst::Operation::Divide:         return "Divide";
        case BinaryInst::Operation::Remainder:      return "Remainder";
        case BinaryInst::Operation::BitwiseAnd:     return "BitwiseAnd";
        case BinaryInst::Operation::BitwiseOr:      return "BitwiseOr";
        case BinaryInst::Operation::BitwiseXor:     return "BitwiseXor";
        case BinaryInst::Operation::LeftShift:      return "LeftShift";
        case BinaryInst::Operation::RightShift:     return "RightShift";
        case BinaryInst::Operation::And:            return "And";
        case BinaryInst::Operation::Or:             return "Or";
        case BinaryInst::Operation::Equal:          return "Equal";
        case BinaryInst::Operation::NotEqual:       return "NotEqual";
        case BinaryInst::Operation::LessThan:       return "LessThan";
        case BinaryInst::Operation::LessOrEqual:    return "LessOrEqual";
        case BinaryInst::Operation::GreaterThan:    return "GreaterThan";
        case BinaryInst::Operation::GreaterOrEqual: return "GreaterOrEqual";
    }
    return "UnknownBinaryOp";
}

void Printer::print(const Instruction& instruction) {
    printIndent();
    switch (instruction.type) {
        case Instruction::Type::Return:
            visit(static_cast<const ReturnInst&>(instruction)); break;
        case Instruction::Type::Unary:
            visit(static_cast<const UnaryInst&>(instruction)); break;
        case Instruction::Type::Binary:
            visit(static_cast<const BinaryInst&>(instruction)); break;
        case Instruction::Type::Copy:
            visit(static_cast<const CopyInst&>(instruction)); break;
        case Instruction::Type::Jump:
            visit(static_cast<const JumpInst&>(instruction)); break;
        case Instruction::Type::JumpIfZero:
            visit(static_cast<const JumpIfZeroInst&>(instruction)); break;
        case Instruction::Type::JumpIfNotZero:
            visit(static_cast<const JumpIfNotZeroInst&>(instruction)); break;
        case Instruction::Type::Label:
            visit(static_cast<const LabelInst&>(instruction)); break;
        default:
            m_oss << "Unknown Instruction\n";
            break;
    }
}

void Printer::printIndent()
{
    for (size_t i = 0; i < indentLevel * 2; ++i)
        m_oss << ' ';
}

} // namespace Ir
