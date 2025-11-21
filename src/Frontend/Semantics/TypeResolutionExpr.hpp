#pragma once

#include "ASTBase.hpp"
#include "FuncEntry.hpp"
#include "VarTable.hpp"

namespace Semantics {

class TypeResolutionExpr {
    static constexpr auto s_boolType = Type::I32;

    const std::unordered_map<std::string, FuncEntry>& m_functions;

    std::vector<Error>& m_errors;
    const VarTable& varTable;
public:
    explicit TypeResolutionExpr(std::vector<Error>& errors,
                                const VarTable& varTable,
                                const std::unordered_map<std::string, FuncEntry>& m_functions)
        : m_errors(errors), varTable(varTable), m_functions(m_functions) {}

    TypeResolutionExpr() = delete;

    bool m_isConst = false;
    bool m_inArrayInit = false;

    std::unique_ptr<Parsing::Expr> convertArrayType(Parsing::Expr& expr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::Expr& expr);

    static std::unique_ptr<Parsing::Expr> convertConstExpr(Parsing::ConstExpr& constExpr);
    std::unique_ptr<Parsing::Expr> convertStringExpr(Parsing::StringExpr& stringExpr) const;
    std::unique_ptr<Parsing::Expr> convertVarExpr(Parsing::VarExpr& varExpr);
    std::unique_ptr<Parsing::Expr> convertCastExpr(Parsing::CastExpr& castExpr);
    std::unique_ptr<Parsing::Expr> convertUnaryExpr(Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<Parsing::Expr> convertBinaryExpr(Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<Parsing::Expr> handleAddSubtractPtrToIntegerTypes(Parsing::BinaryExpr& binaryExpr,
                                                                    Type leftType, Type rightType) const;
    std::unique_ptr<Parsing::Expr> handlePtrToPtrBinaryOpers(Parsing::BinaryExpr& binaryExpr) const;
    std::unique_ptr<Parsing::Expr> handleBinaryPtr(Parsing::BinaryExpr& binaryExpr,
                                                   Type leftType, Type rightType) const;
    std::unique_ptr<Parsing::Expr> converSimpleAssignExpr(Parsing::AssignmentExpr& assignmentExpr) const;
    std::unique_ptr<Parsing::Expr> convertAssignExpr(Parsing::AssignmentExpr& assignmentExpr);
    std::unique_ptr<Parsing::Expr> convertTernaryExpr(Parsing::TernaryExpr& ternaryExpr);
    std::unique_ptr<Parsing::Expr> convertFuncCallExpr(Parsing::FuncCallExpr& funCallExpr);

    std::unique_ptr<Parsing::Expr> convertDerefExpr(Parsing::DereferenceExpr& dereferenceExpr);
    std::unique_ptr<Parsing::Expr> convertAddrOfExpr(Parsing::AddrOffExpr& addrOffExpr);
    std::unique_ptr<Parsing::Expr> convertSubscriptExpr(Parsing::SubscriptExpr& subscriptExpr);
    std::unique_ptr<Parsing::Expr> convertSizeOfExprExpr(Parsing::SizeOfExprExpr& sizeOfExprExpr);
    std::unique_ptr<Parsing::Expr> convertSizeOfExprType(Parsing::SizeOfTypeExpr& sizeOfTypeExpr) const;

    std::unique_ptr<Parsing::Expr> convertDotExpr(Parsing::DotExpr& dotExpr);
    std::unique_ptr<Parsing::Expr> convertArrowExpr(Parsing::ArrowExpr& arrowExpr);
    std::unique_ptr<Parsing::Expr> validateAndConvertPtrsInTernaryExpr(
        Parsing::TernaryExpr& ternaryExpr, Type trueType, Type falseType) const;

    const Parsing::TypeBase* validateStructuredAccessors(
        const Parsing::TypeBase* structuredType,
        const std::string& identifier,
        i64 location) const;
    void validateAndConvertFuncCallArgs(
        Parsing::FuncCallExpr& funCallExpr,
        const std::vector<std::unique_ptr<Parsing::TypeBase>>& params) const;

    [[nodiscard]] bool isLegalAssignExpr(const Parsing::AssignmentExpr& assignmentExpr) const;

private:
    void addError(const std::string& error, const i64 location) const { m_errors.emplace_back(error, location); }
    [[nodiscard]] bool hasError() const { return !m_errors.empty(); }
};

inline bool isBinaryBitwise(const Parsing::BinaryExpr::Operator binOper)
{
    using Operator = Parsing::BinaryExpr::Operator;
    return binOper == Operator::BitwiseAnd || binOper == Operator::BitwiseOr ||
           binOper == Operator::BitwiseXor || binOper == Operator::LeftShift ||
           binOper == Operator::RightShift;
}

inline bool isIllegalFloatingBinaryOperator(const Parsing::BinaryExpr::Operator oper)
{
    return isBinaryBitwise(oper) || oper == Parsing::BinaryExpr::Operator::Modulo;
}

inline bool isIllegalUnaryPointerOperator(const Parsing::UnaryExpr::Operator oper)
{
    using Operator = Parsing::UnaryExpr::Operator;
    return oper == Operator::Complement || oper == Operator::Negate;
}

inline bool isIllegalPtrBinaryOperation(const Parsing::BinaryExpr::Operator oper)
{
    using Oper = Parsing::BinaryExpr::Operator;
    return oper == Oper::Modulo || oper == Oper::Multiply ||
           oper == Oper::Divide ||
           oper == Oper::BitwiseOr || oper == Oper::BitwiseXor;
}

inline bool isUnallowedComparisonBetweenPtrAndInteger(const Parsing::BinaryExpr::Operator oper)
{
    using Oper = Parsing::BinaryExpr::Operator;
    return oper == Oper::GreaterThan || oper == Oper::GreaterOrEqual ||
           oper == Oper::LessThan || oper == Oper::LessOrEqual;
}

bool areValidNonArithmeticTypesInTernaryExpr(const Parsing::TernaryExpr& ternaryExpr);
Parsing::TypeBase* getTypeFromMembers(
    const std::unordered_map<std::string, MemberEntry>& memberMap,
    const std::string& identifier);
} // Semantics