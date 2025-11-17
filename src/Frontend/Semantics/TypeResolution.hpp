#pragma once

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"
#include "ASTTypes.hpp"
#include "TypeConversion.hpp"
#include "ASTDeepCopy.hpp"
#include "Error.hpp"
#include "Utils.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Semantics {

template<typename TargetType, Type TargetKind>
void convertConstantExprRudolf(Parsing::VarDecl& varDecl, const Parsing::ConstExpr& constExpr)
{
    const TargetType value = getValueFromConst<TargetType>(constExpr);
    varDecl.init = std::make_unique<Parsing::SingleInitializer>(
            std::make_unique<Parsing::ConstExpr>(
        value, std::make_unique<Parsing::VarType>(TargetKind)));
}

class TypeResolution final : public Parsing::ASTTraverser {
    static constexpr auto s_boolType = Type::I32;

    struct FuncEntry {
        std::vector<std::unique_ptr<Parsing::TypeBase>> paramTypes;
        Type returnType;
        Parsing::Declaration::StorageClass storage;
        bool defined;
        FuncEntry(const std::vector<std::unique_ptr<Parsing::TypeBase>>& params,
                  const Type returnType,
                  const Parsing::Declaration::StorageClass storage,
                  const bool defined)
            : returnType(returnType), storage(storage), defined(defined)
        {
            for (const auto& paramType : params)
                paramTypes.emplace_back(std::move(Parsing::deepCopy(*paramType)));
        }
    };
    using Storage = Parsing::Declaration::StorageClass;
    std::unordered_map<std::string, FuncEntry> m_functions;
    std::unordered_set<std::string> m_definedFunctions;
    std::unordered_set<std::string> m_localExternVars;
    std::unordered_set<std::string> m_globalStaticVars;
    std::vector<Error> m_errors;
    bool m_isConst = true;
    bool m_global = true;
    bool m_inArrayInit = false;
public:
    std::vector<Error> validate(Parsing::Program& program);

    void visit(Parsing::FuncDeclaration& funDecl) override;
    void visit(Parsing::VarDecl& varDecl) override;
    bool isIllegalVarDecl(const Parsing::VarDecl& varDecl);
    void visit(Parsing::DeclForInit& declForInit) override;
    void visit(Parsing::ExprForInit& exprForInit) override;

    void visit(Parsing::SingleInitializer& singleInitializer) override;
    void visit(Parsing::CompoundInitializer& compoundInitializer) override;

    void visit(Parsing::ReturnStmt& stmt) override;
    void visit(Parsing::ExprStmt& stmt) override;
    void visit(Parsing::IfStmt& ifStmt) override;
    void visit(Parsing::CaseStmt& caseStmt) override;
    void visit(Parsing::WhileStmt& whileStmt) override;
    void visit(Parsing::DoWhileStmt& doWhileStmt) override;
    void visit(Parsing::ForStmt& forStmt) override;
    void visit(Parsing::SwitchStmt& switchStmt) override;

    std::unique_ptr<Parsing::Expr> convertArrayType(Parsing::Expr& expr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::Expr& expr);

    static std::unique_ptr<Parsing::Expr> convertConstExpr(const Parsing::ConstExpr& expr);
    std::unique_ptr<Parsing::Expr> convertStringExpr(Parsing::StringExpr& expr);
    std::unique_ptr<Parsing::Expr> convertVarExpr(Parsing::VarExpr& varExpr);
    std::unique_ptr<Parsing::Expr> convertCastExpr(Parsing::CastExpr& castExpr);
    std::unique_ptr<Parsing::Expr> convertUnaryExpr(Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<Parsing::Expr> convertBinaryExpr(Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<Parsing::Expr> handleAddSubtractPtrToIntegerTypes(Parsing::BinaryExpr& binaryExpr,
                                                                    Type leftType, Type rightType);
    std::unique_ptr<Parsing::Expr> handlePtrToPtrBinaryOpers(Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<Parsing::Expr> handleBinaryPtr(Parsing::BinaryExpr& binaryExpr,
                                                   Type leftType, Type rightType, Type commonType);
    std::unique_ptr<Parsing::Expr> convertAssignExpr(Parsing::AssignmentExpr& assignmentExpr);
    std::unique_ptr<Parsing::Expr> convertTernaryExpr(Parsing::TernaryExpr& ternaryExpr);
    std::unique_ptr<Parsing::Expr> convertFuncCallExpr(Parsing::FuncCallExpr& funCallExpr);
    void validateAndConvertFuncCallArgs(
        Parsing::FuncCallExpr& funCallExpr,
        const std::unordered_map<std::string, FuncEntry>::iterator& it);
    std::unique_ptr<Parsing::Expr> convertDerefExpr(Parsing::DereferenceExpr& dereferenceExpr);
    std::unique_ptr<Parsing::Expr> convertAddrOfExpr(Parsing::AddrOffExpr& addrOffExpr);
    std::unique_ptr<Parsing::Expr> convertSubscriptExpr(Parsing::SubscriptExpr& subscriptExpr);
    std::unique_ptr<Parsing::Expr> convertSizeOfExprExpr(Parsing::SizeOfExprExpr& sizeOfExprExpr);
    std::unique_ptr<Parsing::Expr> convertSizeOfExprType(const Parsing::SizeOfTypeExpr& sizeOfTypeExpr);

    bool isLegalAssignExpr(Parsing::AssignmentExpr& assignmentExpr);
    std::unique_ptr<Parsing::Expr> validateAndConvertPtrsInTernaryExpr(
        Parsing::TernaryExpr& ternaryExpr, Type trueType, Type falseType);
    static void assignTypeToArithmeticUnaryExpr(Parsing::VarDecl& varDecl);
    [[nodiscard]] bool validFuncDecl(const FuncEntry& funcEntry, const Parsing::FuncDeclaration& funDecl);
    void handelCompoundInit(const Parsing::VarDecl& varDecl);
    void verifyArrayInSingleInit(
        const Parsing::VarDecl& varDecl,  const Parsing::SingleInitializer& singleInitializer);
    void handleSingleInit(Parsing::VarDecl& varDecl);
    static bool hasStorageClassSpecifier(const Parsing::DeclForInit& declForInit);
private:
    void addError(const std::string& error, const i64 location) { m_errors.emplace_back(error, location); }
    [[nodiscard]] bool hasError() const { return !m_errors.empty(); }
    std::unique_ptr<Parsing::TypeBase> getCommonPointerType(
        const std::unique_ptr<Parsing::Expr>& left,
        const std::unique_ptr<Parsing::Expr>& right);
};

inline bool TypeResolution::hasStorageClassSpecifier(const Parsing::DeclForInit& declForInit)
{
    return declForInit.decl->storage != Storage::None;
}

inline bool illegalNonConstInitialization(const Parsing::VarDecl& varDecl,
                                          const bool isConst,
                                          const bool global)
{
    return !isConst && (global || varDecl.storage ==  Parsing::Declaration::StorageClass::Static);
}

inline bool isBinaryBitwise(const Parsing::BinaryExpr::Operator binOper)
{
    using Operator = Parsing::BinaryExpr::Operator;
    return binOper == Operator::BitwiseAnd || binOper == Operator::BitwiseOr ||
           binOper == Operator::BitwiseXor || binOper == Operator::LeftShift ||
           binOper == Operator::RightShift;
}

inline bool isIllegalUnaryPointerOperator(const Parsing::UnaryExpr::Operator oper)
{
    using Operator = Parsing::UnaryExpr::Operator;
    return oper == Operator::Complement || oper == Operator::Negate;
}

inline bool isIllegalDoubleCompoundAssignOperation(const Parsing::AssignmentExpr::Operator oper)
{
    using Oper = Parsing::AssignmentExpr::Operator;
    return oper == Oper::ModuloAssign || oper == Oper::BitwiseAndAssign ||
           oper == Oper::BitwiseOrAssign || oper == Oper::BitwiseXorAssign ||
           oper == Oper::LeftShiftAssign || oper == Oper::RightShiftAssign;;
}

inline bool isIllegalPointerCompoundAssignOperation(const Parsing::AssignmentExpr::Operator oper)
{
    using Oper = Parsing::AssignmentExpr::Operator;
    return oper == Oper::BitwiseAndAssign || oper == Oper::BitwiseOrAssign ||
           oper == Oper::DivideAssign || oper == Oper::ModuloAssign;
}

inline bool isUnallowedPtrBinaryOperation(const Parsing::BinaryExpr::Operator oper)
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

bool areValidNonArithmeticTypesInBinaryExpr(const Parsing::BinaryExpr& binaryExpr,
    Type leftType, Type rightType, Type commonType);
bool areValidNonArithmeticTypesInTernaryExpr(const Parsing::TernaryExpr& ternaryExpr);
} // Semantics