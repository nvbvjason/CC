 #pragma once

#ifndef CC_SEMANTICS_TYPE_RESOLUTION_HPP
#define CC_SEMANTICS_TYPE_RESOLUTION_HPP

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>


 namespace Semantics {

class TypeResolution : public Parsing::ASTTraverser {
    using Storage = Parsing::Declaration::StorageClass;
    std::unordered_map<std::string, Parsing::Declaration::StorageClass> m_storageClassesFuncs;
    std::unordered_map<std::string, Parsing::Type::Kind> m_returnTypesFuncs;
    std::unordered_map<std::string, size_t> m_functionArgCounts;
    std::unordered_set<std::string> m_definedFunctions;
    std::unordered_set<std::string> m_localExternVars;
    std::unordered_set<std::string> m_globalStaticVars;
    bool m_valid = true;
    bool m_isConst = true;
    bool m_global = true;
public:
    bool validate(Parsing::Program& program);
    bool hasConflictingFuncLinkage(const Parsing::FunDecl& funDecl);

    void visit(Parsing::FunDecl& funDecl) override;
    void visit(Parsing::DeclForInit& declForInit) override;

    void visit(Parsing::FunCallExpr& funCallExpr) override;
    void visit(Parsing::VarDecl& varDecl) override;
    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::UnaryExpr& unaryExpr) override;
    void visit(Parsing::BinaryExpr& binaryExpr) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
    void visit(Parsing::TernaryExpr& ternaryExpr) override;
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

inline Parsing::Type::Kind getCommonType(const Parsing::Type::Kind type1, const Parsing::Type::Kind type2)
{
    if (type1 == type2)
        return type1;
    return Parsing::Type::Kind::Long;
}

} // Semantics

#endif // CC_SEMANTICS_TYPE_RESOLUTION_HPP
