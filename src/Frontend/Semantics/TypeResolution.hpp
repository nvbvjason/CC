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
};
} // Semantics

#endif // CC_SEMANTICS_TYPE_RESOLUTION_HPP
