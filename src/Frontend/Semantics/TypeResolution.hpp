#pragma once

#ifndef CC_SEMANTICS_TYPE_RESOLUTION_HPP
#define CC_SEMANTICS_TYPE_RESOLUTION_HPP

#include "ASTParser.hpp"
#include "ConstASTTraverser.hpp"

#include <string>
#include <unordered_map>

namespace Semantics {

class TypeResolution : public Parsing::ConstASTTraverser {
    using StorageClass = Parsing::Declaration::StorageClass;
    std::unordered_map<std::string, Parsing::Declaration::StorageClass> m_storageClassMap;
    bool m_valid;
    bool m_isConst;
    bool m_atFileScope = true;
public:
    bool validate(const Parsing::Program& program);
    void visit(const Parsing::FunDecl& funDecl) override;
    void visit(const Parsing::DeclForInit& declForInit) override;
    void visit(const Parsing::VarDecl& varDecl) override;
    void visit(const Parsing::VarExpr& varExpr) override;
    static bool mustBeConstantInitialised(const Parsing::VarDecl& varDecl, bool isConst);
    static bool hasStorageClassSpecifier(const Parsing::DeclForInit& declForInit);
};

inline bool TypeResolution::hasStorageClassSpecifier(const Parsing::DeclForInit& declForInit)
{
    return declForInit.decl->storageClass != StorageClass::AutoLocalScope;
}

inline bool TypeResolution::mustBeConstantInitialised(const Parsing::VarDecl& varDecl, const bool isConst)
{
    return !isConst && (varDecl.storageClass == StorageClass::StaticGlobal ||
                        varDecl.storageClass == StorageClass::GlobalDefinition ||
                        varDecl.storageClass == StorageClass::StaticLocal ||
                        varDecl.storageClass == StorageClass::AutoGlobalScope ||
                        varDecl.storageClass == StorageClass::ExternGlobalInitialized);
}
} // Semantics

#endif // CC_SEMANTICS_TYPE_RESOLUTION_HPP
