#pragma once

#ifndef CC_PARSING_VARIABLE_SOLUTION_HPP
#define CC_PARSING_VARIABLE_SOLUTION_HPP

#include "ASTParser.hpp"
#include "Frontend/Traverser/ASTTraverser.hpp"

#include <string>
#include <unordered_map>

namespace Semantics {

class VariableResolution : public Parsing::ASTTraverser {
    std::unordered_map<std::string, std::string> variableMap;
    i32 m_counter = 0;
    Parsing::Program& program;
    bool m_valid = true;
public:
    explicit VariableResolution(Parsing::Program& program)
        : program(program) {}

    bool resolve();
    void visit(Parsing::Declaration& declaration) override;
    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
private:
    std::string makeTemporary(const std::string& name);
};

} // Semantics

#endif // CC_PARSING_VARIABLE_SOLUTION_HPP
