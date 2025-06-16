 #pragma once

#ifndef CC_SEMANTICS_TYPE_RESOLUTION_HPP
#define CC_SEMANTICS_TYPE_RESOLUTION_HPP

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"
#include "ASTTypes.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

 namespace Semantics {

template<typename TargetType, Type TargetKind>
void convertConstantExpr(Parsing::VarDecl& varDecl, const Parsing::ConstExpr& constExpr)
{
    TargetType value;
    if (constExpr.type->kind == Type::I32)
        value = std::get<i32>(constExpr.value);
    else if (constExpr.type->kind == Type::I64)
        value = std::get<i64>(constExpr.value);
    else if (constExpr.type->kind == Type::U32)
        value = std::get<u32>(constExpr.value);
    else if (constExpr.type->kind == Type::U64)
        value = std::get<u64>(constExpr.value);
    else if (constExpr.type->kind == Type::Double)
        value = std::get<double>(constExpr.value);
    varDecl.init = std::make_unique<Parsing::ConstExpr>(
        value, std::make_unique<Parsing::VarType>(TargetKind)
    );
}

class TypeResolution : public Parsing::ASTTraverser {
    struct FuncEntry {
        std::vector<Type> paramTypes;
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
                paramTypes.emplace_back(paramType->kind);
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

    void visit(Parsing::FunCallExpr& funCallExpr) override;
    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::UnaryExpr& unaryExpr) override;
    void visit(Parsing::BinaryExpr& binaryExpr) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
    void visit(Parsing::TernaryExpr& ternaryExpr) override;
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

inline bool isBinaryBitwise(const Parsing::BinaryExpr& binaryExpr)
{
    using Operator = Parsing::BinaryExpr::Operator;
    return binaryExpr.op == Operator::BitwiseAnd || binaryExpr.op == Operator::BitwiseOr ||
           binaryExpr.op == Operator::BitwiseXor || binaryExpr.op == Operator::LeftShift ||
           binaryExpr.op == Operator::RightShift;
}

inline bool isBinaryComparison(const Parsing::BinaryExpr& binaryExpr)
{
    using Operator = Parsing::BinaryExpr::Operator;
    return binaryExpr.op == Operator::Equal || binaryExpr.op == Operator::NotEqual ||
           binaryExpr.op == Operator::LessThan || binaryExpr.op == Operator::LessOrEqual ||
           binaryExpr.op == Operator::GreaterThan || binaryExpr.op == Operator::GreaterOrEqual;
}
} // Semantics
#endif // CC_SEMANTICS_TYPE_RESOLUTION_HPP