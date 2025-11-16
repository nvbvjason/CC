#include "IrPrinter.hpp"
#include "DynCast.hpp"

#include <sstream>

namespace Ir {

std::string IrPrinter::print(const Program& program)
{
    addLine("Program:");
    for (const auto& topLevel : program.topLevels) {
        switch (topLevel->kind) {
            case TopLevel::Kind::Function: {
                const auto function = dynamic_cast<Function*>(topLevel.get());
                print(*function);
                break;
            }
            case TopLevel::Kind::StaticVariable: {
                const auto variable = dynamic_cast<StaticVariable*>(topLevel.get());
                print(*variable);
                break;
            }
            case TopLevel::Kind::StaticArray: {
                const auto array = dynamic_cast<StaticArray*>(topLevel.get());
                print(*array);
                break;
            }
            case TopLevel::Kind::StaticConstant: {
                const auto constant = dynamic_cast<StaticConstant*>(topLevel.get());
                print(*constant);
                break;
            }
            default:
                std::abort();
        }
        addLine("");
    }
    return m_oss.str();
}

void IrPrinter::print(const StaticConstant& staticConstant)
{
    IndentGuard guard(m_indentLevel);
    addLine("StaticConstant: " + staticConstant.identifier.value);
    addLine("Value: " + staticConstant.value);
    if (staticConstant.global)
        addLine("Is Global");
    if (staticConstant.nullTerminated)
        addLine("Is NullTerminated");
}

void IrPrinter::print(const StaticArray& staticArray)
{
    IndentGuard guard(m_indentLevel);
    addLine("StaticArray: " + staticArray.name);
    addLine("Type: " + to_string(staticArray.type));
    IndentGuard guardInits(m_indentLevel);
    if (staticArray.global)
        addLine("is Global");
    for (const auto& init : staticArray.initializers) {
        switch (init->kind) {
            case Initializer::Kind::Value: {
                const auto value = dynCast<ValueInitializer>(init.get());
                addLine(print(*value->value));
                break;
            }
            case Initializer::Kind::Zero: {
                const auto zero = dynCast<ZeroInitializer>(init.get());
                addLine("Zero. " + std::to_string(zero->size));
                break;
            }
        }
    }
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
            return print(*dynCast<const ValueVar>(&value));
        case Value::Kind::Constant:
            return print(*dynCast<const ValueConst>(&value));
        default:
            std::abort();
    }
}

std::string IrPrinter::print(const Identifier& identifier)
{
    return identifier.value;
}

void IrPrinter::print(const ReturnInst& inst)
{
    addLine("Return: " + print(*inst.returnValue));
    addLine("");
}

void IrPrinter::print(const SignExtendInst& inst)
{
    addLine("SignExtend: " + print(*inst.src) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const ZeroExtendInst& inst)
{
    addLine("ZeroExtend: " + print(*inst.src) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const TruncateInst& inst)
{
    addLine("Truncate: " + print(*inst.src) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const DoubleToIntInst& inst)
{
    addLine("DoubleToInt: " + print(*inst.src) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const DoubleToUIntInst& inst)
{
    addLine("DoubleToUInt: " + print(*inst.src) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const IntToDoubleInst& inst)
{
    addLine("IntToDouble: " + print(*inst.src) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const UIntToDoubleInst& inst)
{
    addLine("UIntToDouble: " + print(*inst.src) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const UnaryInst& inst)
{
    addLine(to_string(inst.operation) + " " +
            print(*inst.src) + " -> " +
            print(*inst.dst) + ", " +
            to_string(inst.type));
}

void IrPrinter::print(const BinaryInst& inst)
{
    addLine(print(*inst.lhs) + " " +
            to_string(inst.operation) + " " +
            print(*inst.rhs) + " -> " +
            print(*inst.dst) + ", " +
            to_string(inst.type));
}

void IrPrinter::print(const CopyInst& inst)
{
    addLine("Copy: " + print(*inst.src) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const GetAddressInst& inst)
{
    addLine("GetAddress: " + print(*inst.src) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const LoadInst& inst)
{
    addLine("Load: " + print(*inst.ptr) + " -> " + print(*inst.dst) + ", " + to_string(inst.type));
}

void IrPrinter::print(const StoreInst& inst)
{
    addLine("Store: " + print(*inst.src) + " -> " + print(*inst.ptr) + ", " + to_string(inst.type));
}

void IrPrinter::print(const AddPtrInst& inst)
{
    addLine("AddPtrInst: " +
            print(*inst.ptr) + " + " +
            print(*inst.index) + " * " +
            std::to_string(inst.scale) + " -> " +
            print(*inst.dst) + " " +
            to_string(inst.type));
}

void IrPrinter::print(const CopyToOffsetInst& inst)
{
    addLine("CopyToOffsetInst: " +
            print(*inst.src) + " -> " +
            print(inst.iden) + " offset " +
            std::to_string(inst.offset) + ", " +
            to_string(inst.type));
}

void IrPrinter::print(const JumpInst& inst)
{
    addLine("Jump: " + print(inst.target));
}

void IrPrinter::print(const JumpIfZeroInst& inst)
{
    addLine("JumpIfZero: " + print(*inst.condition) + ", " + print(inst.target) + ", " + to_string(inst.type));
}

void IrPrinter::print(const JumpIfNotZeroInst& inst)
{
    addLine("JumpIfNotZero: " + print(*inst.condition) + ", " + print(inst.target) + ", " + to_string(inst.type));
}

void IrPrinter::print(const LabelInst& inst)
{
    addLine("Label: " +print(inst.target));
}

void IrPrinter::print(const FunCallInst &inst)
{
    addLine("FunCall: " + inst.funName.value);
    IndentGuard guard2(m_indentLevel);
    std::string args;
    for (const auto& arg : inst.args)
        args += print(*arg) + ", ";
    addLine("args: " + args);
}

void IrPrinter::print(const AllocateInst& inst)
{
    addLine("Allocate:" + print(inst.iden) + ", " + std::to_string(inst.size));
}

std::string IrPrinter::print(const ValueVar& val)
{
    if (val.type == Type::I32)
        return "Var(" + print(val.value) + ") i32";
    return "Var(" + print(val.value) + ") i64";
}

std::string IrPrinter::print(const ValueConst& val)
{
    switch (val.type) {
        case Type::I8:           return "Const(" + std::to_string(std::get<i8>(val.value)) + ") I8";
        case Type::U8:           return "Const(" + std::to_string(std::get<u8>(val.value)) + ") U8";
        case Type::Char:         return "Const(" + std::to_string(std::get<char>(val.value)) + ") Char";
        case Type::I32:          return "Const(" + std::to_string(std::get<i32>(val.value)) + ") i32";
        case Type::I64:          return "Const(" + std::to_string(std::get<i64>(val.value)) + ") i64";
        case Type::U32:          return "Const(" + std::to_string(std::get<u32>(val.value)) + ") u32";
        case Type::U64:          return "Const(" + std::to_string(std::get<u64>(val.value)) + ") u64";
        case Type::Double:       return "Const(" + std::to_string(std::get<double>(val.value)) + ") double";
        default:
            std::abort();
    }
}

std::string to_string(const UnaryInst::Operation op)
{
    using Operation = UnaryInst::Operation;
    switch (op) {
        case Operation::Complement: return "Complement";
        case Operation::Negate:     return "Negate";
        case Operation::Not:        return "Not";
    }
    return "UnknownUnaryOp";
}

std::string to_string(const BinaryInst::Operation op)
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

std::string to_string(const Type type)
{
    switch (type) {
        case Type::Char:     return "char";
        case Type::U8:       return "u8";
        case Type::I8:       return "i8";
        case Type::I32:      return "i32";
        case Type::I64:      return "i64";
        case Type::U32:      return "u32";
        case Type::U64:      return "u64";
        case Type::Double:   return "double";
        case Type::Pointer:  return "pointer";
        case Type::Array:    return "array";
        case Type::String:   return "string";
        default:
            std::unreachable();
    }
}

void IrPrinter::print(const Instruction& instruction) {
    using Kind = Instruction::Kind;
    switch (instruction.kind) {
        case Kind::Return:          print(*dynCast<const ReturnInst>(&instruction)); break;
        case Kind::SignExtend:      print(*dynCast<const SignExtendInst>(&instruction)); break;
        case Kind::ZeroExtend:      print(*dynCast<const ZeroExtendInst>(&instruction)); break;
        case Kind::Truncate:        print(*dynCast<const TruncateInst>(&instruction)); break;
        case Kind::DoubleToInt:     print(*dynCast<const DoubleToIntInst>(&instruction)); break;
        case Kind::DoubleToUInt:    print(*dynCast<const DoubleToUIntInst>(&instruction)); break;
        case Kind::IntToDouble:     print(*dynCast<const IntToDoubleInst>(&instruction)); break;
        case Kind::UIntToDouble:    print(*dynCast<const UIntToDoubleInst>(&instruction)); break;
        case Kind::Unary:           print(*dynCast<const UnaryInst>(&instruction)); break;
        case Kind::Binary:          print(*dynCast<const BinaryInst>(&instruction)); break;
        case Kind::Copy:            print(*dynCast<const CopyInst>(&instruction)); break;
        case Kind::GetAddress:      print(*dynCast<const GetAddressInst>(&instruction)); break;
        case Kind::Load:            print(*dynCast<const LoadInst>(&instruction)); break;
        case Kind::Store:           print(*dynCast<const StoreInst>(&instruction)); break;
        case Kind::AddPtr:          print(*dynCast<const AddPtrInst>(&instruction)); break;
        case Kind::CopyToOffset:    print(*dynCast<const CopyToOffsetInst>(&instruction)); break;
        case Kind::Jump:            print(*dynCast<const JumpInst>(&instruction)); break;
        case Kind::JumpIfZero:      print(*dynCast<const JumpIfZeroInst>(&instruction)); break;
        case Kind::JumpIfNotZero:   print(*dynCast<const JumpIfNotZeroInst>(&instruction)); break;
        case Kind::Label:           print(*dynCast<const LabelInst>(&instruction)); break;
        case Kind::FunCall:         print(*dynCast<const FunCallInst>(&instruction)); break;
        case Kind::Allocate:        print(*dynCast<const AllocateInst>(&instruction)); break;
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