#pragma once

#ifndef CC_SEMANTICS_VARIABLE_SOLUTION_HPP
#define CC_SEMANTICS_VARIABLE_SOLUTION_HPP

#include "ASTTraverser.hpp"
#include "ShortTypes.hpp"

#include <string>
#include <unordered_map>

namespace Semantics {


class VariableResolution : public Parsing::ASTTraverser {
    std::unordered_map<std::string, std::string> m_variables;
    i32 m_nameCounter;
    bool m_valid;
public:
    bool resolve(Parsing::Program& program);
    void visit(Parsing::VarDecl& varDecl) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
    void visit(Parsing::VarExpr& varExpr) override;
private:
    void reset();
    std::string makeTemporaryName(const std::string &name);
};

} // Semantics

#endif // CC_SEMANTICS_VARIABLE_SOLUTION_HPP