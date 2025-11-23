#pragma once

#include "AsmAST.hpp"
#include "ASTIr.hpp"
#include "IrType.hpp"

namespace CodeGen {

UnaryInst::Operator unaryOperator(Ir::UnaryInst::Operation type);
BinaryInst::Operator binaryOperator(Ir::BinaryInst::Operation type);
BinaryInst::Operator getShiftOperator(Ir::BinaryInst::Operation type, bool isSigned);
BinaryInst::CondCode condCode(Ir::BinaryInst::Operation oper, bool isSigned);
AsmType getAsmType(Ir::IrType type);

inline UnaryInst::Operator unaryOperator(const Ir::UnaryInst::Operation type)
{
    using IrOper = Ir::UnaryInst::Operation;
    using AsmOper = UnaryInst::Operator;
    switch (type)
    {
        case IrOper::Complement:        return AsmOper::Not;
        case IrOper::Negate:            return AsmOper::Neg;
        default:
            std::abort();
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

        case IrOper::BitwiseAnd:   return AsmOper::BitwiseAnd;
        case IrOper::BitwiseOr:    return AsmOper::BitwiseOr;
        case IrOper::BitwiseXor:   return AsmOper::BitwiseXor;
        default:
            std::abort();
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
                std::abort();
        }
    }
    switch (type) {
        case IrOper::LeftShift:    return AsmOper::LeftShiftUnsigned;
        case IrOper::RightShift:   return AsmOper::RightShiftUnsigned;
        default:
            std::abort();
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
                std::abort();
        }
    switch (oper) {
        case IrOper::Equal:             return BinCond::E;
        case IrOper::NotEqual:          return BinCond::NE;
        case IrOper::LessThan:          return BinCond::B;
        case IrOper::LessOrEqual:       return BinCond::BE;
        case IrOper::GreaterThan:       return BinCond::A;
        case IrOper::GreaterOrEqual:    return BinCond::AE;
        default:
            std::abort();
    }
}

constexpr auto asmByte =     AsmType(AsmType::Kind::Byte, 1);
constexpr auto asmWord =     AsmType(AsmType::Kind::Word, 2);
constexpr auto asmLongWord = AsmType(AsmType::Kind::LongWord, 4);
constexpr auto asmQuadWord = AsmType(AsmType::Kind::QuadWord, 8);
constexpr auto asmDouble =   AsmType(AsmType::Kind::Double, 8);

inline AsmType getAsmType(const Ir::IrType type)
{
    if (type == Ir::i8Type || type == Ir::u8Type || type == Ir::charType)
        return asmByte;
    if (type == Ir::i32Type || type == Ir::u32Type)
        return asmLongWord;
    if (type == Ir::i64Type || type == Ir::u64Type || type == Ir::pointerType)
        return asmQuadWord;
    if (type == Ir::doubleType)
        return asmDouble;
    std::abort();
}

inline AsmType getAsmType(const Type type)
{
    switch (type) {
        case Type::I8:      return asmByte;
        case Type::U8:      return asmByte;
        case Type::Char:    return asmByte;
        case Type::I32:     return asmLongWord;
        case Type::U32:     return asmLongWord;
        case Type::I64:     return asmQuadWord;
        case Type::U64:     return asmQuadWord;
        case Type::Pointer: return asmQuadWord;
        case Type::Double:  return asmDouble;
        default:
            std::abort();
    }
}
} // CodeGen