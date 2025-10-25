#pragma once

#include "AsmAST.hpp"
#include "ASTIr.hpp"

namespace CodeGen::Operators {

UnaryInst::Operator unaryOperator(Ir::UnaryInst::Operation type);
BinaryInst::Operator binaryOperator(Ir::BinaryInst::Operation type);
BinaryInst::Operator getShiftOperator(Ir::BinaryInst::Operation type, bool isSigned);
BinaryInst::CondCode condCode(Ir::BinaryInst::Operation oper, bool isSigned);
AsmType getAsmType(Type type);

inline UnaryInst::Operator unaryOperator(const Ir::UnaryInst::Operation type)
{
    using IrOper = Ir::UnaryInst::Operation;
    using AsmOper = UnaryInst::Operator;
    switch (type)
    {
        case IrOper::Complement:        return AsmOper::Not;
        case IrOper::Negate:            return AsmOper::Neg;
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

inline BinaryInst::Operator binaryOperator(const Ir::BinaryInst::Operation type)
{
    using IrOper = Ir::BinaryInst::Operation;
    using AsmOper = BinaryInst::Operator;
    switch (type) {
        case IrOper::Add:          return AsmOper::Add;
        case IrOper::Subtract:     return AsmOper::Sub;
        case IrOper::Multiply:     return AsmOper::Mul;

        case IrOper::BitwiseAnd:   return AsmOper::AndBitwise;
        case IrOper::BitwiseOr:    return AsmOper::OrBitwise;
        case IrOper::BitwiseXor:   return AsmOper::BitwiseXor;
        default:
            throw std::invalid_argument("Invalid BinaryOperation type: " +
                std::to_string(static_cast<int>(type)));
    }
}

inline BinaryInst::Operator getShiftOperator(const Ir::BinaryInst::Operation type, const bool isSigned)
{
    using IrOper = Ir::BinaryInst::Operation;
    using AsmOper = BinaryInst::Operator;
    if (isSigned) {
        switch (type) {
            case IrOper::LeftShift:    return AsmOper::LeftShiftSigned;
            case IrOper::RightShift:   return AsmOper::RightShiftSigned;
            default:
                throw std::invalid_argument("Invalid BinaryOperation type: " +
                    std::to_string(static_cast<int>(type)));
        }
    }
    switch (type) {
        case IrOper::LeftShift:    return AsmOper::LeftShiftUnsigned;
        case IrOper::RightShift:   return AsmOper::RightShiftUnsigned;
        default:
            throw std::invalid_argument("Invalid BinaryOperation type: " +
                std::to_string(static_cast<int>(type)));
    }
}

inline BinaryInst::CondCode condCode(const Ir::BinaryInst::Operation oper, const bool isSigned)
{
    using IrOper = Ir::BinaryInst::Operation;
    using BinCond = BinaryInst::CondCode;
    if (isSigned)
        switch (oper) {
            case IrOper::Equal:             return BinCond::E;
            case IrOper::NotEqual:          return BinCond::NE;
            case IrOper::LessThan:          return BinCond::L;
            case IrOper::LessOrEqual:       return BinCond::LE;
            case IrOper::GreaterThan:       return BinCond::G;
            case IrOper::GreaterOrEqual:    return BinCond::GE;
            default:
                throw std::invalid_argument("Invalid BinaryOperation type");
        }
    switch (oper) {
        case IrOper::Equal:             return BinCond::E;
        case IrOper::NotEqual:          return BinCond::NE;
        case IrOper::LessThan:          return BinCond::B;
        case IrOper::LessOrEqual:       return BinCond::BE;
        case IrOper::GreaterThan:       return BinCond::A;
        case IrOper::GreaterOrEqual:    return BinCond::AE;
        default:
                throw std::invalid_argument("Invalid BinaryOperation type");
    }
}

inline AsmType getAsmType(Type type)
{
    if (type == Type::I32 || type == Type::U32)
        return AsmType::LongWord;
    if (type == Type::I64 || type == Type::U64 || type == Type::Pointer)
        return AsmType::QuadWord;
    if (type == Type::Double)
        return AsmType::Double;
    std::abort();
}

} // CodeGen::Operators