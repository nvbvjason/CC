#pragma once

#ifndef CC_SEMANTICS_VARIABLE_SOLUTION_HPP
#define CC_SEMANTICS_VARIABLE_SOLUTION_HPP

#include "ASTTraverser.hpp"
#include "ShortTypes.hpp"

#include <string>
#include <unordered_map>

namespace Semantics {

struct MapEntry {
    std::string name;
    bool fromCurrentScope;
    MapEntry(std::string name, bool fromCurrentScope)
        : name(std::move(name)), fromCurrentScope(fromCurrentScope) {}
};

class VariableResolution : public Parsing::ASTTraverser {
    std::unordered_map<std::string, MapEntry> m_variables;
    i32 m_nameCounter = 0;
    bool m_valid = true;
public:
    bool resolve(Parsing::Program& program);
    void visit(Parsing::Block& block) override;
    void visit(Parsing::ForStmt& forStmt) override;
    void visit(Parsing::VarDecl& varDecl) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
    void visit(Parsing::VarExpr& varExpr) override;
private:
    void reset();
    std::string makeTemporaryName(const std::string &name);
};

std::unordered_map<std::string, MapEntry> copyMapForBlock(const std::unordered_map<std::string, MapEntry> &map);

} // Semantics
#endif // CC_SEMANTICS_VARIABLE_SOLUTION_HPP