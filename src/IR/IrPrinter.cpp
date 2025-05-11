#include "IrPrinter.hpp"

#include <sstream>

namespace Ir {

std::string IrPrinter::print(const Program& program)
{
    addLine("Program:");
    for (const auto& topLevel : program.topLevels) {
        if (topLevel->type == TopLevel::Type::Function) {
            const auto function = dynamic_cast<Function*>(topLevel.get());
            print(*function);
        }
        if (topLevel->type == TopLevel::Type::StaticVariable) {
            const auto variable = dynamic_cast<StaticVariable*>(topLevel.get());
            print(*variable);
        }
    }
    return m_oss.str();
}

void IrPrinter::print(const StaticVariable& variable)
{
    IndentGuard guard(m_indentLevel);
    addLine("Variable: " + variable.name);
    if (variable.isGlobal)
        addLine("is Global");
    else
        addLine("is not Global");
    print(*variable.value);
}

void IrPrinter::print(const Function& function)
{
    IndentGuard guard(m_indentLevel);
    addLine("Function " + function.name);
    if (function.isGlobal)
        addLine("is Global");
    else
        addLine("is not Global");
    IndentGuard guard2(m_indentLevel);
    std::string args;
    for (const auto& arg : function.args)
        args += print(arg) + ", ";
    addLine("args: " + args);
    for (const auto& inst : function.insts)
        print(*inst);
}

std::string IrPrinter::print(const Value& value)
{
    switch (value.type) {
        case Value::Type::Variable:
            return print(static_cast<const ValueVar&>(value));
        case Value::Type::Constant:
            return print(static_cast<const ValueConst&>(value));
    }
    std::unreachable();
}

std::string IrPrinter::print(const Identifier& identifier)
{
    return identifier.value;
}

void IrPrinter::visit(const ReturnInst& inst)
{
    addLine("Return " + print(*inst.returnValue));
}

void IrPrinter::visit(const UnaryInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine(to_string(inst.operation) + " " +
            print(*inst.source) + " -> " +
            print(*inst.destination));
}

void IrPrinter::visit(const BinaryInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine(print(*inst.source1) + " " +
            to_string(inst.operation) + " " +
            print(*inst.source2) + " -> " +
            print(*inst.destination));
}

void IrPrinter::visit(const CopyInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("Copy " + print(*inst.source) + " -> " + print(*inst.destination));
}

void IrPrinter::visit(const JumpInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("Jump " + print(inst.target));
}

void IrPrinter::visit(const JumpIfZeroInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("JumpIfZero " + print(*inst.condition) + ", " + print(inst.target));
}

void IrPrinter::visit(const JumpIfNotZeroInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("JumpIfNotZero " + print(*inst.condition) + ", " + print(inst.target));
}

void IrPrinter::visit(const LabelInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("Label " +print(inst.target));
}

void IrPrinter::visit(const FunCallInst &inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("FunCall: " + inst.funName.value);
    IndentGuard guard2(m_indentLevel);
    std::string args;
    for (const auto& arg : inst.args)
        args += print(*arg) + ", ";
    addLine("args: " + args);
}

std::string IrPrinter::print(const ValueVar& val)
{
    return "Var(" + print(val.value) + ")";
}

std::string IrPrinter::print(const ValueConst& val)
{
    return "Const(" + std::to_string(val.value) + ")";
}

std::string to_string(UnaryInst::Operation op)
{
    using Operation = UnaryInst::Operation;
    switch (op) {
        case Operation::Complement: return "Complement";
        case Operation::Negate:     return "Negate";
        case Operation::Not:        return "Not";
    }
    return "UnknownUnaryOp";
}

std::string to_string(BinaryInst::Operation op)
{
    using Operation = BinaryInst::Operation;
    switch (op) {
        case Operation::Add:            return "Add";
        case Operation::Subtract:       return "Subtract";
        case Operation::Multiply:       return "Multiply";
        case Operation::Divide:         return "Divide";
        case Operation::Remainder:      return "Remainder";
        case Operation::BitwiseAnd:     return "BitwiseAnd";
        case Operation::BitwiseOr:      return "BitwiseOr";
        case Operation::BitwiseXor:     return "BitwiseXor";
        case Operation::LeftShift:      return "LeftShift";
        case Operation::RightShift:     return "RightShift";
        case Operation::And:            return "And";
        case Operation::Or:             return "Or";
        case Operation::Equal:          return "Equal";
        case Operation::NotEqual:       return "NotEqual";
        case Operation::LessThan:       return "LessThan";
        case Operation::LessOrEqual:    return "LessOrEqual";
        case Operation::GreaterThan:    return "GreaterThan";
        case Operation::GreaterOrEqual: return "GreaterOrEqual";
    }
    return "UnknownBinaryOp";
}

void IrPrinter::print(const Instruction& instruction) {
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
        case Instruction::Type::FunCall:
            visit(static_cast<const FunCallInst&>(instruction)); break;
        default:
            m_oss << "Unknown Instruction\n";
            break;
    }
}

void IrPrinter::addLine(const std::string& line)
{
    m_oss << getIndent() << line << '\n';
}

std::string IrPrinter::getIndent() const
{
    return std::string(m_indentLevel * c_indentMult, ' ');
}
} // namespace Ir
