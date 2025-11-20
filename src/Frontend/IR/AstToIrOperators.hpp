#pragma once

#include "ASTExpr.hpp"
#include "ASTIr.hpp"
#include "DynCast.hpp"

namespace Ir {
inline UnaryInst::Operation convertUnaryOperation(const Parsing::UnaryExpr::Operator unaryOperation)
{
    using Operator = Parsing::UnaryExpr::Operator;
    using OperationIr = UnaryInst::Operation;
    switch (unaryOperation) {
        case Operator::Complement:  return OperationIr::Complement;
        case Operator::Negate:      return OperationIr::Negate;
        case Operator::Not:         return OperationIr::Not;
        default:
            throw std::invalid_argument("Invalid unary operation");
    }
}

inline BinaryInst::Operation convertBinaryOperation(const Parsing::BinaryExpr::Operator binaryOperation)
{
    using Parse = Parsing::BinaryExpr::Operator;
    using Ir = BinaryInst::Operation;
    switch (binaryOperation) {
        case Parse::Add:            return Ir::Add;
        case Parse::Subtract:       return Ir::Subtract;
        case Parse::Multiply:       return Ir::Multiply;
        case Parse::Divide:         return Ir::Divide;
        case Parse::Modulo:         return Ir::Remainder;

        case Parse::BitwiseAnd:     return Ir::BitwiseAnd;
        case Parse::BitwiseOr:      return Ir::BitwiseOr;
        case Parse::BitwiseXor:     return Ir::BitwiseXor;

        case Parse::LeftShift:      return Ir::LeftShift;
        case Parse::RightShift:     return Ir::RightShift;

        case Parse::And:            return Ir::And;
        case Parse::Or:             return Ir::Or;
        case Parse::Equal:          return Ir::Equal;
        case Parse::NotEqual:       return Ir::NotEqual;
        case Parse::GreaterThan:    return Ir::GreaterThan;
        case Parse::GreaterOrEqual: return Ir::GreaterOrEqual;
        case Parse::LessThan:       return Ir::LessThan;
        case Parse::LessOrEqual:    return Ir::LessOrEqual;

        default:
            throw std::invalid_argument("Invalid binary operation convertBinaryOperation generateIr");
    }
}

inline bool isPostfixOp(const Parsing::UnaryExpr::Operator oper)
{
    return oper == Parsing::UnaryExpr::Operator::PostFixDecrement
        || oper == Parsing::UnaryExpr::Operator::PostFixIncrement;
}

inline bool isPrefixOp(const Parsing::UnaryExpr::Operator oper)
{
    return oper == Parsing::UnaryExpr::Operator::PrefixDecrement
        || oper == Parsing::UnaryExpr::Operator::PrefixIncrement;
}

inline BinaryInst::Operation getPostPrefixOperation(const Parsing::UnaryExpr::Operator oper)
{
    using Operator = Parsing::UnaryExpr::Operator;
    switch (oper) {
        case Operator::PostFixDecrement:
        case Operator::PrefixDecrement:
            return BinaryInst::Operation::Subtract;
        case Operator::PostFixIncrement:
        case Operator::PrefixIncrement:
            return BinaryInst::Operation::Add;
        default:
            throw std::invalid_argument("Invalid postfix operation");
    }
}

inline BinaryInst::Operation convertBinaryOperation(const Parsing::AssignmentExpr::Operator assignOperation)
{
    using Parse = Parsing::AssignmentExpr::Operator;
    using Ir = BinaryInst::Operation;
    switch (assignOperation) {
        case Parse::PlusAssign:         return Ir::Add;
        case Parse::MinusAssign:        return Ir::Subtract;
        case Parse::MultiplyAssign:     return Ir::Multiply;
        case Parse::DivideAssign:       return Ir::Divide;
        case Parse::ModuloAssign:       return Ir::Remainder;
        case Parse::BitwiseAndAssign:   return Ir::BitwiseAnd;
        case Parse::BitwiseOrAssign:    return Ir::BitwiseOr;
        case Parse::BitwiseXorAssign:   return Ir::BitwiseXor;
        case Parse::LeftShiftAssign:    return Ir::LeftShift;
        case Parse::RightShiftAssign:   return Ir::RightShift;
        default:
            std::abort();
    }
}

inline bool isBitShift(const Parsing::AssignmentExpr::Operator oper)
{
    using Oper = Parsing::AssignmentExpr::Operator;
    return oper == Oper::LeftShiftAssign || oper == Oper::RightShiftAssign;
}

} // Ir