#include "IrPrinter.hpp"

#include <sstream>

namespace Ir {

std::string IrPrinter::print(const Program& program)
{
    addLine("Program:");
    for (const auto& topLevel : program.topLevels) {
        if (topLevel->type == TopLevel::Kind::Function) {
            const auto function = dynamic_cast<Function*>(topLevel.get());
            print(*function);
        }
        if (topLevel->type == TopLevel::Kind::StaticVariable) {
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
    IndentGuard innerGuard(m_indentLevel);
    if (variable.global)
        addLine("is Global");
    else
        addLine("is not Global");
    addLine(print(*variable.value));
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
    switch (value.kind) {
        case Value::Kind::Variable:
            return print(static_cast<const ValueVar&>(value));
        case Value::Kind::Constant:
            return print(static_cast<const ValueConst&>(value));
    }
    std::unreachable();
}

std::string IrPrinter::print(const Identifier& identifier)
{
    return identifier.value;
}

void IrPrinter::print(const ReturnInst& inst)
{
    addLine("Return " + print(*inst.returnValue));
}

void IrPrinter::print(const SignExtendInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("SignExtend " + print(*inst.src) + " -> " + print(*inst.dst));
}

void IrPrinter::print(const ZeroExtendInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("ZeroExtend " + print(*inst.src) + " -> " + print(*inst.dst));
}

void IrPrinter::print(const TruncateInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("Truncate " + print(*inst.src) + " -> " + print(*inst.dst));
}

void IrPrinter::print(const DoubleToIntInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("DoubleToInt " + print(*inst.src) + " -> " + print(*inst.dst));
}

void IrPrinter::print(const DoubleToUIntInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("DoubleToUInt " + print(*inst.src) + " -> " + print(*inst.dst));
}

void IrPrinter::print(const IntToDoubleInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("IntToDouble " + print(*inst.src) + " -> " + print(*inst.dst));
}

void IrPrinter::print(const UIntToDoubleInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("UIntToDouble " + print(*inst.src) + " -> " + print(*inst.dst));
}

void IrPrinter::print(const UnaryInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine(to_string(inst.operation) + " " +
            print(*inst.source) + " -> " +
            print(*inst.destination));
}

void IrPrinter::print(const BinaryInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine(print(*inst.lhs) + " " +
            to_string(inst.operation) + " " +
            print(*inst.rhs) + " -> " +
            print(*inst.destination));
}

void IrPrinter::print(const CopyInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("Copy " + print(*inst.source) + " -> " + print(*inst.destination));
}

void IrPrinter::print(const JumpInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("Jump " + print(inst.target));
}

void IrPrinter::print(const JumpIfZeroInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("JumpIfZero " + print(*inst.condition) + ", " + print(inst.target));
}

void IrPrinter::print(const JumpIfNotZeroInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("JumpIfNotZero " + print(*inst.condition) + ", " + print(inst.target));
}

void IrPrinter::print(const LabelInst& inst)
{
    IndentGuard guard(m_indentLevel);
    addLine("Label " +print(inst.target));
}

void IrPrinter::print(const FunCallInst &inst)
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
    if (val.type == Type::I32)
        return "Var(" + print(val.value) + ") i32";
    return "Var(" + print(val.value) + ") i64";
}

std::string IrPrinter::print(const ValueConst& val)
{
    if (val.type == Type::I32)
        return "Const(" + std::to_string(std::get<i32>(val.value)) + ") i32";
    if (val.type == Type::I64)
        return "Const(" + std::to_string(std::get<i64>(val.value)) + ") i64";
    if (val.type == Type::U32)
        return "Const(" + std::to_string(std::get<u32>(val.value)) + ") u32";
    if (val.type == Type::U64)
        return "Const(" + std::to_string(std::get<u64>(val.value)) + ") u64";
    std::unreachable();
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
    switch (instruction.kind) {
        case Instruction::Kind::Return:
            print(static_cast<const ReturnInst&>(instruction)); break;
        case Instruction::Kind::SignExtend:
            print(static_cast<const SignExtendInst&>(instruction)); break;
        case Instruction::Kind::Truncate:
            print(static_cast<const TruncateInst&>(instruction)); break;
        case Instruction::Kind::Unary:
            print(static_cast<const UnaryInst&>(instruction)); break;
        case Instruction::Kind::Binary:
            print(static_cast<const BinaryInst&>(instruction)); break;
        case Instruction::Kind::Copy:
            print(static_cast<const CopyInst&>(instruction)); break;
        case Instruction::Kind::Jump:
            print(static_cast<const JumpInst&>(instruction)); break;
        case Instruction::Kind::JumpIfZero:
            print(static_cast<const JumpIfZeroInst&>(instruction)); break;
        case Instruction::Kind::JumpIfNotZero:
            print(static_cast<const JumpIfNotZeroInst&>(instruction)); break;
        case Instruction::Kind::Label:
            print(static_cast<const LabelInst&>(instruction)); break;
        case Instruction::Kind::FunCall:
            print(static_cast<const FunCallInst&>(instruction)); break;
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
