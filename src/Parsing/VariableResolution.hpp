#pragma once

#ifndef CC_PARSING_VARIABLE_SOLUTION_HPP
#define CC_PARSING_VARIABLE_SOLUTION_HPP

#include "AbstractTree.hpp"

#include <string>
#include <unordered_map>


namespace Parsing {

class VariableResolution {
    std::unordered_map<std::string, std::string> variableMap;
    i32 m_counter = 0;
    Program& program;
    bool m_valid = true;
public:
    explicit VariableResolution(Program& program)
        : program(program) {}

    bool resolve();
    void resolveFunction(Function& func);
    void resolveBlockItem(BlockItem& blockItem);
    void resolveDeclaration(Declaration& declaration);
    void resolveStmt(Stmt& declaration);
    void resolveExpr(Expr& declaration);
private:
    std::string makeTemporary(const std::string& name);
};

} // Parsing

#endif // CC_PARSING_VARIABLE_SOLUTION_HPP
