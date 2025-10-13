 #pragma once

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"
#include "ASTTypes.hpp"
#include "TypeConversion.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

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
    varDecl.init = std::make_unique<Parsing::ConstExpr>(
        value, std::make_unique<Parsing::VarType>(TargetKind));
}

class TypeResolution : public Parsing::ASTTraverser {
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

    void visit(Parsing::FunDecl& funDecl) override;
    void visit(Parsing::VarDecl& varDecl) override;
    void visit(Parsing::DeclForInit& declForInit) override;

    void visit(Parsing::FuncCallExpr& funCallExpr) override;
    bool isIllegalVarDecl(const Parsing::VarDecl& varDecl) const;
    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::UnaryExpr& unaryExpr) override;
    void visit(Parsing::BinaryExpr& binaryExpr) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
    void visit(Parsing::CastExpr& castExpr) override;
    void visit(Parsing::TernaryExpr& ternaryExpr) override;
    void visit(Parsing::AddrOffExpr& addrOffExpr) override;
    void visit(Parsing::DereferenceExpr& dereferenceExpr) override;

    static void assignTypeToArithmeticUnaryExpr(Parsing::VarDecl& varDecl);
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
} // Semantics