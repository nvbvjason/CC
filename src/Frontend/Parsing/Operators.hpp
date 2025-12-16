#pragma once

#include "Token.hpp"
#include "ASTParser.hpp"

#include <cassert>

namespace Parsing::Operators {

using TokenType = Lexing::Token::Type;
[[nodiscard]] constexpr UnaryExpr::Operator unaryOperator(TokenType type);
[[nodiscard]] constexpr BinaryExpr::Operator binaryOperator(TokenType type);
[[nodiscard]] constexpr AssignmentExpr::Operator assignOperator(TokenType type);
[[nodiscard]] constexpr bool isBinaryOperator(TokenType type);
[[nodiscard]] constexpr bool isUnaryOperator(TokenType type);
[[nodiscard]] constexpr bool isAssignmentOperator(TokenType type);
[[nodiscard]] constexpr bool isStorageSpecifier(TokenType type);
[[nodiscard]] constexpr bool isSpecifier(TokenType type);
[[nodiscard]] constexpr bool isType(TokenType type);
[[nodiscard]] constexpr bool isStructuredType(TokenType type);
[[nodiscard]] constexpr BinaryExpr::Operator getBinaryOperator(AssignmentExpr::Operator oper);
[[nodiscard]] Declaration::StorageClass getStorageClass(Lexing::Token::Type tokenType);


// https://en.cppreference.com/w/c/language/operator_precedence
[[nodiscard]] constexpr i32 precedence(TokenType type);
[[nodiscard]] constexpr i32 getPrecedenceLevel(UnaryExpr::Operator oper);
[[nodiscard]] constexpr i32 getPrecedenceLevel(BinaryExpr::Operator oper);
[[nodiscard]] constexpr i32 getPrecedenceLevel(AssignmentExpr::Operator oper);

constexpr UnaryExpr::Operator unaryOperator(const TokenType type)
{
    using Operator = UnaryExpr::Operator;
    switch (type) {
        case TokenType::Plus:               return Operator::Plus;
        case TokenType::Minus:              return Operator::Negate;
        case TokenType::Tilde:              return Operator::Complement;
        case TokenType::ExclamationMark:    return Operator::Not;
        case TokenType::Increment:          return Operator::PrefixIncrement;
        case TokenType::Decrement:          return Operator::PrefixDecrement;
        default:
            assert(false && "Invalid unary operator unaryOperator");
            std::unreachable();
    }
}

constexpr BinaryExpr::Operator binaryOperator(const TokenType type)
{
    using Operator = BinaryExpr::Operator;
    switch (type) {
        // Arithmetic operators
        case TokenType::Plus:               return Operator::Add;
        case TokenType::Minus:              return Operator::Subtract;
        case TokenType::Asterisk:           return Operator::Multiply;
        case TokenType::ForwardSlash:       return Operator::Divide;
        case TokenType::Percent:            return Operator::Modulo;

        // Bitwise operators
        case TokenType::Ampersand:          return Operator::BitwiseAnd;
        case TokenType::Pipe:               return Operator::BitwiseOr;
        case TokenType::Circumflex:         return Operator::BitwiseXor;
        case TokenType::LeftShift:          return Operator::LeftShift;
        case TokenType::RightShift:         return Operator::RightShift;

        // Logical/comparison operators
        case TokenType::LogicalAnd:         return Operator::And;
        case TokenType::LogicalOr:          return Operator::Or;
        case TokenType::LogicalEqual:       return Operator::Equal;
        case TokenType::LogicalNotEqual:    return Operator::NotEqual;
        case TokenType::Greater:            return Operator::GreaterThan;
        case TokenType::GreaterOrEqual:     return Operator::GreaterOrEqual;
        case TokenType::Less:               return Operator::LessThan;
        case TokenType::LessOrEqual:        return Operator::LessOrEqual;

        default:
            assert(false && "Invalid binary operator: binaryOperator(Token::Type)");
            std::unreachable();
    }
}

constexpr AssignmentExpr::Operator assignOperator(const TokenType type)
{
    using Operator = AssignmentExpr::Operator;
    switch (type) {
        case TokenType::Equal:              return Operator::Assign;
        case TokenType::PlusAssign:         return Operator::PlusAssign;
        case TokenType::MinusAssign:        return Operator::MinusAssign;
        case TokenType::MultiplyAssign:     return Operator::MultiplyAssign;
        case TokenType::DivideAssign:       return Operator::DivideAssign;
        case TokenType::ModuloAssign:       return Operator::ModuloAssign;
        case TokenType::BitwiseAndAssign:   return Operator::BitwiseAndAssign;
        case TokenType::BitwiseOrAssign:    return Operator::BitwiseOrAssign;
        case TokenType::BitwiseXorAssign:   return Operator::BitwiseXorAssign;
        case TokenType::LeftShiftAssign:    return Operator::LeftShiftAssign;
        case TokenType::RightShiftAssign:   return Operator::RightShiftAssign;

        default:
            assert(false && "Invalid binary operator: assignOperator(Token::Type)");
            std::unreachable();
    }
}

constexpr bool isBinaryOperator(const TokenType type)
{
    switch (type) {
        case TokenType::Plus:
        case TokenType::Minus:
        case TokenType::ForwardSlash:
        case TokenType::Percent:
        case TokenType::Asterisk:
        case TokenType::LeftShift:
        case TokenType::RightShift:
        case TokenType::Ampersand:
        case TokenType::Pipe:
        case TokenType::Circumflex:

        case TokenType::LogicalAnd:
        case TokenType::LogicalOr:
        case TokenType::LogicalEqual:
        case TokenType::LogicalNotEqual:
        case TokenType::Greater:
        case TokenType::Less:
        case TokenType::LessOrEqual:
        case TokenType::GreaterOrEqual:
            return true;
        default:
            return false;
    }
}

constexpr bool isUnaryOperator(const TokenType type)
{
    switch (type) {
        case TokenType::Minus:
        case TokenType::Plus:
        case TokenType::Tilde:
        case TokenType::ExclamationMark:
        case TokenType::Increment:
        case TokenType::Decrement:
        case TokenType::Ampersand:
        case TokenType::Asterisk:
        case TokenType::SizeOf:
            return true;
        default:
            return false;
    }
}

constexpr bool isAssignmentOperator(TokenType type)
{
    switch (type) {
        case TokenType::Equal:
        case TokenType::PlusAssign:
        case TokenType::MinusAssign:
        case TokenType::DivideAssign:
        case TokenType::MultiplyAssign:
        case TokenType::ModuloAssign:
        case TokenType::BitwiseAndAssign:
        case TokenType::BitwiseOrAssign:
        case TokenType::BitwiseXorAssign:
        case TokenType::LeftShiftAssign:
        case TokenType::RightShiftAssign:
            return true;
        default:
            return false;
    }
}

constexpr i32 precedence(const TokenType type)
{
    constexpr i32 precedenceMult = 1 << 10;
    constexpr i32 precedenceLevels = 16;
    if (type == TokenType::QuestionMark || type == TokenType::Colon)
        return (precedenceLevels - 13) * precedenceMult;
    if (isBinaryOperator(type)) {
        BinaryExpr::Operator oper = binaryOperator(type);
        return (precedenceLevels - getPrecedenceLevel(oper)) * precedenceMult;
    }
    if (isUnaryOperator(type)) {
        UnaryExpr::Operator oper = Operators::unaryOperator(type);
        return (precedenceLevels - getPrecedenceLevel(oper)) * precedenceMult;
    }
    if (isAssignmentOperator(type)) {
        AssignmentExpr::Operator oper = Operators::assignOperator(type);
        return (precedenceLevels - getPrecedenceLevel(oper)) * precedenceMult;
    }
    return 0;
}

constexpr i32 getPrecedenceLevel(const UnaryExpr::Operator oper)
{
    using Operator = UnaryExpr::Operator;
    switch (oper) {
        case Operator::PostFixIncrement:
        case Operator::PostFixDecrement:
            return 1;
        case Operator::PrefixIncrement:
        case Operator::PrefixDecrement:
        case Operator::Complement:
        case Operator::Negate:
        case Operator::Not:
            return 2;
        default:
            return 0;
    }
}

constexpr i32 getPrecedenceLevel(const BinaryExpr::Operator oper)
{
    using Operator = BinaryExpr::Operator;
    switch (oper) {
        case Operator::Multiply:
        case Operator::Divide:
        case Operator::Modulo:
            return 3;
        case Operator::Add:
        case Operator::Subtract:
            return 4;
        case Operator::LeftShift:
        case Operator::RightShift:
            return 5;
        case Operator::LessThan:
        case Operator::LessOrEqual:
        case Operator::GreaterThan:
        case Operator::GreaterOrEqual:
            return 6;
        case Operator::Equal:
        case Operator::NotEqual:
            return 7;
        case Operator::BitwiseAnd:
            return 8;
        case Operator::BitwiseXor:
            return 9;
        case Operator::BitwiseOr:
            return 10;
        case Operator::And:
            return 11;
        case Operator::Or:
            return 12;
        default:
            assert(false && "Invalid binary operator getPrecedenceLevel");
            std::unreachable();
    }
}

constexpr i32 getPrecedenceLevel(const AssignmentExpr::Operator oper)
{
    return 14;
}

constexpr bool isSpecifier(const TokenType type)
{
    return isType(type) || isStorageSpecifier(type);
}

constexpr bool isStorageSpecifier(const TokenType type)
{
    switch (type) {
        case TokenType::Static:
        case TokenType::Extern:
            return true;
        default:
            return false;
    }
}

constexpr bool isType(const TokenType type)
{
    switch (type) {
        case TokenType::CharKeyword:
        case TokenType::IntKeyword:
        case TokenType::LongKeyword:
        case TokenType::Signed:
        case TokenType::Unsigned:
        case TokenType::DoubleKeyword:
        case TokenType::StructKeyword:
        case TokenType::UnionKeyword:
        case TokenType::VoidKeyword:
            return true;
        default:
            return false;
    }
}

constexpr bool isStructuredType(const TokenType type)
{
    return type == TokenType::StructKeyword || type == TokenType::UnionKeyword;
}

constexpr BinaryExpr::Operator getBinaryOperator(const AssignmentExpr::Operator oper)
{
    using Assign = AssignmentExpr::Operator;
    using Binary = BinaryExpr::Operator;
    switch (oper) {
        case Assign::PlusAssign:        return Binary::Add;
        case Assign::MinusAssign:       return Binary::Subtract;
        case Assign::MultiplyAssign:    return Binary::Multiply;
        case Assign::DivideAssign:      return Binary::Divide;
        case Assign::ModuloAssign:      return Binary::Modulo;
        case Assign::BitwiseAndAssign:  return Binary::BitwiseAnd;
        case Assign::BitwiseOrAssign:   return Binary::BitwiseOr;
        case Assign::BitwiseXorAssign:  return Binary::BitwiseXor;
        case Assign::LeftShiftAssign:   return Binary::LeftShift;
        case Assign::RightShiftAssign:  return Binary::RightShift;
        default:
            assert(false && "Invalid binary operator getBinaryOperator");
            std::unreachable();
    }
}

inline Declaration::StorageClass getStorageClass(const Lexing::Token::Type tokenType)
{
    using StorageClass = Declaration::StorageClass;
    switch (tokenType) {
        case TokenType::Static:     return StorageClass::Static;
        case TokenType::Extern:     return StorageClass::Extern;
        case TokenType::NotAToken:  return StorageClass::None;
        default:
            assert("getVarStorageClass invalid TokenType");
            std::abort();
    }
    assert("getVarStorageClass invalid TokenType");
}

inline bool isLiteral(const TokenType type)
{
    switch (type) {
        case TokenType::CharLiteral:
        case TokenType::IntegerLiteral:
        case TokenType::UnsignedIntegerLiteral:
        case TokenType::LongLiteral:
        case TokenType::UnsignedLongLiteral:
        case TokenType::DoubleLiteral:
            return true;
        default:
            return false;
    }
    std::unreachable();
}
} // namespace Parsing::Operators