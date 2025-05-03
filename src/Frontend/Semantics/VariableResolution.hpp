#pragma once

#ifndef CC_PARSING_VARIABLE_SOLUTION_HPP
#define CC_PARSING_VARIABLE_SOLUTION_HPP

#include "../Parsing/ASTParser.hpp"

#include <string>
#include <unordered_map>


namespace Semantics {

class VariableResolution {
    std::unordered_map<std::string, std::string> variableMap;
    i32 m_counter = 0;
    Parsing::Program& program;
    bool m_valid = true;
public:
    explicit VariableResolution(Parsing::Program& program)
        : program(program) {}

    bool resolve();
    void resolveFunction(Parsing::Function& func);
    void resolveBlockItem(Parsing::BlockItem& blockItem);
    void resolveDeclaration(Parsing::Declaration& declaration);
    void resolveStmt(Parsing::Stmt& declaration);
    void resolveExpr(Parsing::Expr& declaration);
private:
    std::string makeTemporary(const std::string& name);
};

} // Semantics

#endif // CC_PARSING_VARIABLE_SOLUTION_HPP
