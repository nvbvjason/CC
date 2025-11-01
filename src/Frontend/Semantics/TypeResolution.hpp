 #pragma once

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"
#include "ASTTypes.hpp"
#include "TypeConversion.hpp"
#include "ASTDeepCopy.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Error.hpp"

 namespace Semantics {

template<typename TargetType, Type TargetKind>
void convertConstantExpr(Parsing::VarDecl& varDecl, const Parsing::ConstExpr& constExpr)
{
    TargetType value;
    switch (constExpr.type->type) {
        case Type::I32:
            value = std::get<i32>(constExpr.value);
            break;
        case Type::I64:
            value = std::get<i64>(constExpr.value);
            break;
        case Type::U32:
            value = std::get<u32>(constExpr.value);
            break;
        case Type::U64:
            value = std::get<u64>(constExpr.value);
            break;
        case Type::Double:
            value = std::get<double>(constExpr.value);
            break;
        default:
            std::abort();
    }
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
public:
    std::vector<Error> validate(Parsing::Program& program);

    void visit(Parsing::FunDeclaration& funDecl) override;
    void visit(Parsing::VarDecl& varDecl) override;
    bool isIllegalVarDecl(const Parsing::VarDecl& varDecl);
    void visit(Parsing::DeclForInit& declForInit) override;
    void visit(Parsing::ExprForInit& exprForInit) override;

    void visit(Parsing::SingleInitializer& singleInitializer) override;
    void visit(Parsing::CompoundInitializer& compoundInitializer) override;

    void visit(Parsing::ReturnStmt& stmt) override;
    void visit(Parsing::ExprStmt& stmt) override;
    void visit(Parsing::IfStmt& ifStmt) override;
    void visit(Parsing::CaseStmt& stmt) override;
    void visit(Parsing::WhileStmt& stmt) override;
    void visit(Parsing::DoWhileStmt& stmt) override;
    void visit(Parsing::ForStmt& stmt) override;
    void visit(Parsing::SwitchStmt& stmt) override;

    std::unique_ptr<Parsing::Expr> convertArrayType(Parsing::Expr& expr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::Expr& expr);

    std::unique_ptr<Parsing::Expr> convert(Parsing::ConstExpr& expr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::VarExpr& varExpr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::CastExpr& castExpr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::AssignmentExpr& assignmentExpr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::TernaryExpr& ternaryExpr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::FuncCallExpr& funCallExpr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::DereferenceExpr& dereferenceExpr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::AddrOffExpr& addrOffExpr);
    std::unique_ptr<Parsing::Expr> convert(Parsing::SubscriptExpr& subscriptExpr);

    bool isLegalAssignExpr(Parsing::AssignmentExpr& assignmentExpr);
    static void assignTypeToArithmeticUnaryExpr(Parsing::VarDecl& varDecl);
    [[nodiscard]] bool validFuncDecl(const FuncEntry& funcEntry, const Parsing::FunDeclaration& funDecl);
    static bool hasStorageClassSpecifier(const Parsing::DeclForInit& declForInit);
private:
    void addError(const std::string& error, const i64 location) { m_errors.emplace_back(error, location); }
    [[nodiscard]] bool hasError() const { return !m_errors.empty(); }
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

bool areValidNonArithmeticTypesInBinaryExpr(const Parsing::BinaryExpr& binaryExpr);
bool areValidNonArithmeticTypesInTernaryExpr(const Parsing::TernaryExpr& ternaryExpr);
} // Semantics