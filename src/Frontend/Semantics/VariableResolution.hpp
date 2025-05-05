#pragma once

#ifndef CC_PARSING_VARIABLE_SOLUTION_HPP
#define CC_PARSING_VARIABLE_SOLUTION_HPP

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"

#include <string>
#include <unordered_map>

namespace Semantics {

class VariableStack {
    std::vector<std::unordered_map<std::string, std::string>> m_stack;
public:
    VariableStack() = default;
    void push();
    void pop();
    void addDecl(const std::string& name, const std::string& value);
    bool tryRename(const std::string& oldName, const std::string& newName);
    [[nodiscard]] bool isDeclared(const std::string& name) const;
    [[nodiscard]] std::string* find(const std::string& name) noexcept;
    [[nodiscard]] const std::string* find(const std::string& name) const noexcept;
    [[nodiscard]] bool contains(const std::string& name) const noexcept;
};

class VariableResolution : public Parsing::ASTTraverser {
    VariableStack m_variableStack;
    i32 m_counter = 0;
    Parsing::Program& program;
    bool m_valid = true;
public:
    explicit VariableResolution(Parsing::Program& program)
        : program(program) {}

    bool resolve();
    void visit(Parsing::Block& function) override;

    void visit(Parsing::ContinueStmt& continueStmt) override;
    void visit(Parsing::BreakStmt& breakStmt) override;
    void visit(Parsing::ForStmt& function) override;

    void visit(Parsing::Declaration& declaration) override;
    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
private:
    std::string makeTemporary(const std::string& name);
};

} // Semantics

#endif // CC_PARSING_VARIABLE_SOLUTION_HPP
