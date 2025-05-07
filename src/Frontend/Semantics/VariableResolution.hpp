#pragma once

#ifndef CC_SEMANTICS_VARIABLE_SOLUTION_HPP
#define CC_SEMANTICS_VARIABLE_SOLUTION_HPP

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"
#include "VariableStack.hpp"

#include <string>
#include <unordered_map>

namespace Semantics {

class VariableResolution : public Parsing::ASTTraverser {
    VariableStack m_variableStack;
    std::unordered_map<std::string, std::vector<std::string>> m_funcDecls;
    i32 m_counter = 0;
    Parsing::Program& program;
    bool m_valid = true;
    bool m_inFunctionBody = false;
public:
    explicit VariableResolution(Parsing::Program& program)
        : program(program) {}

    bool resolve();
    void visit(Parsing::Program& program) override;
    void visit(Parsing::FunDecl& funDecl) override;
    void visit(Parsing::Block& function) override;

    void visit(Parsing::ContinueStmt& continueStmt) override;
    void visit(Parsing::BreakStmt& breakStmt) override;

    void visit(Parsing::VarDecl& varDecl) override;
    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
    void visit(Parsing::FunCallExpr& funCallExpr) override;
private:
    std::string makeTemporary(const std::string& name);
    [[nodiscard]] bool hasDuplicates(const std::vector<std::string>& vec);
};

} // Semantics

#endif // CC_SEMANTICS_VARIABLE_SOLUTION_HPP
