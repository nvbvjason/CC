 #pragma once

#ifndef CC_SEMANTICS_TYPE_RESOLUTION_HPP
#define CC_SEMANTICS_TYPE_RESOLUTION_HPP

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"
#include "ASTTypes.hpp"
#include "ASTInitializer.hpp"
#include "TypeConversion.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Semantics {

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

    bool m_valid = true;
    bool m_isConst = true;
    bool m_global = true;
public:
    bool validate(Parsing::Program& program);

    // Declarations
    void checkDecl(Parsing::Declaration& decl);
    void checkVarDecl(Parsing::VarDecl& varDecl);
    void checkFuncDecl(const Parsing::FunDecl& funDecl);

    // Block
    void checkBlock(const Parsing::Block& block);

    // BlockItem
    void checkBlockItem(Parsing::BlockItem& blockItem);
    void checkStmtBlockItem(const Parsing::StmtBlockItem& stmtBlockItem);
    void checkDeclBlockItem(const Parsing::DeclBlockItem& declBlockItem);

    // Initializer
    void checkInitializer(Parsing::Initializer& initializer);
    void checkSingleInit(Parsing::SingleInit& singleInit);
    void checkCompoundInit(const Parsing::CompoundInit& compoundInit);

    // ForInit
    void checkForInit(Parsing::ForInit& forInit);
    void checkDeclForInit(const Parsing::DeclForInit& declForInit);
    void checkExprForInit(const Parsing::ExprForInit& exprForInit);

    // Statements
    void checkStmt(Parsing::Stmt& stmt);
    void checkReturnStmt(const Parsing::ReturnStmt& returnStmt);
    void checkExprStmt(const Parsing::ExprStmt& exprStmt);
    void checkIfStmt(const Parsing::IfStmt& ifStmt);
    static void checkGotoStmt(Parsing::GotoStmt& gotoStmt) {}
    void checkCompoundStmt(const Parsing::CompoundStmt& compoundStmt);
    static void checkBreakStmt(Parsing::BreakStmt& breakStmt) {}
    static void checkContinueStmt(Parsing::ContinueStmt& continueStmt) {}
    void checkLabelStmt(const Parsing::LabelStmt& labelStmt);
    void checkCaseStmt(const Parsing::CaseStmt& caseStmt);
    void checkDefaultStmt(const Parsing::DefaultStmt& defaultStmt);
    void checkWhileStmt(const Parsing::WhileStmt& whileStmt);
    void checkDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt);
    void checkForStmt(const Parsing::ForStmt& forStmt);
    void checkSwitchStmt(const Parsing::SwitchStmt& switchStmt);
    static void checkNullStmt(Parsing::NullStmt& nullStmt) {}

    // Expressions
    void checkExpr(Parsing::Expr& expr);
    void checkConstExpr(Parsing::ConstExpr& expr);
    void checkVarExpr(Parsing::VarExpr& expr);
    void checkCastExpr(const Parsing::CastExpr& castExpr);
    void checkUnaryExpr(Parsing::UnaryExpr& unaryExpr);
    void checkBinaryExpr(Parsing::BinaryExpr& binaryExpr);
    void checkAssignExpr(Parsing::AssignmentExpr& assignExpr);
    void checkTernaryExpr(Parsing::TernaryExpr& ternaryExpr);
    void checkFuncCallExpr(Parsing::FuncCallExpr& funcCallExpr);
    void checkDereferenceExpr(Parsing::DereferenceExpr& dereferenceExpr);
    void checkAddrOffExpr(Parsing::AddrOffExpr& addrOffExpr);
    void checkSubscriptExpr(const Parsing::SubscriptExpr& subscriptExpr);

    bool isIllegalVarDecl(const Parsing::VarDecl& varDecl) const;

    static void assignTypeToArithmeticUnaryExpr(Parsing::VarDecl& varDecl);
    static void assignTypeToArithmeticBinaryExpr(
        Parsing::BinaryExpr& binaryExpr,
        Type leftType, Type rightType, Type commonType);
    static bool validFuncDecl(const FuncEntry& funcEntry, const Parsing::FunDecl& funDecl);
    static bool hasStorageClassSpecifier(const Parsing::DeclForInit& declForInit);
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

inline bool isBinaryComparison(const Parsing::BinaryExpr& binaryExpr)
{
    using Operator = Parsing::BinaryExpr::Operator;
    return binaryExpr.op == Operator::Equal || binaryExpr.op == Operator::NotEqual ||
           binaryExpr.op == Operator::LessThan || binaryExpr.op == Operator::LessOrEqual ||
           binaryExpr.op == Operator::GreaterThan || binaryExpr.op == Operator::GreaterOrEqual;
}

inline bool isIllegalUnaryPointerOperator(const Parsing::UnaryExpr::Operator oper)
{
    using Operator = Parsing::UnaryExpr::Operator;
    return oper == Operator::Complement || oper == Operator::Negate ||
           oper == Operator::PrefixDecrement || oper == Operator::PrefixIncrement ||
           oper == Operator::PostFixDecrement || oper == Operator::PostFixIncrement;
}

bool isLegalAssignExpr(Parsing::AssignmentExpr& assignmentExpr);
bool areValidNonArithmeticTypesInBinaryExpr(const Parsing::BinaryExpr& binaryExpr);
bool areValidNonArithmeticTypesInTernaryExpr(const Parsing::TernaryExpr& ternaryExpr);
bool isCastFromPointerToAndFromDouble(Type outerType, Type innerType);

template<typename TargetType, Type TargetKind>
void convertConstantExpr(Parsing::VarDecl& varDecl, const Parsing::ConstExpr& constExpr)
{
    TargetType value;
    if (constExpr.type->type == Type::I32)
        value = std::get<i32>(constExpr.value);
    else if (constExpr.type->type == Type::I64)
        value = std::get<i64>(constExpr.value);
    else if (constExpr.type->type == Type::U32)
        value = std::get<u32>(constExpr.value);
    else if (constExpr.type->type == Type::U64)
        value = std::get<u64>(constExpr.value);
    else if (constExpr.type->type == Type::Double)
        value = std::get<double>(constExpr.value);
    if (varDecl.init->kind != Parsing::Initializer::Kind::Single)
        return;
    varDecl.init = std::make_unique<Parsing::SingleInit>(std::make_unique<Parsing::ConstExpr>(
        value, std::make_unique<Parsing::VarType>(TargetKind)));
}
} // Semantics
#endif // CC_SEMANTICS_TYPE_RESOLUTION_HPP