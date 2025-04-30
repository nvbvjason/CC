#include "Printer.hpp"
#include <sstream>

namespace Ir {

void Printer::print(const Program& program)
{
    out << "Program:\n";
    indentLevel++;
    if (program.function) {
        print(*program.function);
    } else {
        printIndent();
        out << "<empty>\n";
    }
    indentLevel--;
}

void Printer::print(const Function& function)
{
    printIndent();
    out << "Function " << function.identifier << ":\n";
    indentLevel++;
    for (const auto& inst : function.instructions)
        print(*inst);
    indentLevel--;
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
    }
}

void Printer::print(const Value& value) const
{
    switch (value.type) {
        case Value::Type::Variable:
            visit(static_cast<const ValueVar&>(value)); break;
        case Value::Type::Constant:
            visit(static_cast<const ValueConst&>(value)); break;
    }
}

void Printer::print(const Identifier& identifier) const
{
    out << identifier.value;
}

// Instruction visitors
void Printer::visit(const ReturnInst& inst) const
{
    out << "Return ";
    print(*inst.returnValue);
    out << "\n";
}

void Printer::visit(const UnaryInst& inst) const
{
    out << to_string(inst.operation) << " ";
    print(*inst.source);
    out << " -> ";
    print(*inst.destination);
    out << "\n";
}

void Printer::visit(const BinaryInst& inst) const
{
    out << to_string(inst.operation) << " ";
    print(*inst.source1);
    out << ", ";
    print(*inst.source2);
    out << " -> ";
    print(*inst.destination);
    out << "\n";
}

void Printer::visit(const CopyInst& inst) const
{
    out << "Copy ";
    print(*inst.source);
    out << " -> ";
    print(*inst.destination);
    out << "\n";
}

void Printer::visit(const JumpInst& inst) const
{
    out << "Jump ";
    print(inst.target);
    out << "\n";
}

void Printer::visit(const JumpIfZeroInst& inst) const
{
    out << "JumpIfZero ";
    print(*inst.condition);
    out << ", ";
    print(inst.target);
    out << "\n";
}

void Printer::visit(const JumpIfNotZeroInst& inst) const
{
    out << "JumpIfNotZero ";
    print(*inst.condition);
    out << ", ";
    print(inst.target);
    out << "\n";
}

void Printer::visit(const LabelInst& inst) const
{
    out << "Label ";
    print(inst.target);
    out << ":\n";
}

// Value visitors
void Printer::visit(const ValueVar& val) const
{
    out << "Var(";
    print(val.value);
    out << ")";
}

void Printer::visit(const ValueConst& val) const
{
    out << "Const(" << val.value << ")";
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

} // namespace Ir