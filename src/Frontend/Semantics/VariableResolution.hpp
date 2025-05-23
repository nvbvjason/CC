#pragma once

#ifndef CC_SEMANTICS_VARIABLE_SOLUTION_HPP
#define CC_SEMANTICS_VARIABLE_SOLUTION_HPP

#include "ASTTraverser.hpp"
#include "ShortTypes.hpp"
#include "ASTParser.hpp"
#include "Frontend/SymbolTable.hpp"

#include <string>

namespace Semantics {

class VariableResolution : public Parsing::ASTTraverser {
    using Storage = Parsing::Declaration::StorageClass;
    SymbolTable& symbolTable;
    i32 m_nameCounter = 0;
    bool m_valid = true;
public:
    explicit VariableResolution(SymbolTable& symbolTable)
        : symbolTable(symbolTable) {}
    bool resolve(Parsing::Program& program);
    void visit(Parsing::FunDecl& funDecl) override;
    void visit(Parsing::CompoundStmt& compoundStmt) override;
    void visit(Parsing::ForStmt& forStmt) override;
    void visit(Parsing::VarDecl& varDecl) override;
    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::FunCallExpr& funCallExpr) override;
private:
    void reset();
    std::string makeTemporaryName(const std::string &name);
};

bool isValidFuncCall(const Parsing::FunCallExpr& funCallExpr, const SymbolTable& symbolTable);
bool isValidVarExpr(const Parsing::VarExpr& varExpr, const SymbolTable& symbolTable);
bool isValidVarDecl(const Parsing::VarDecl& varDecl, const SymbolTable& symbolTable);
bool isValidFuncDecl(const Parsing::FunDecl& funDecl, const SymbolTable& symbolTable);

bool duplicatesInArgs(const std::vector<std::string>& args);
bool isGlobalFunc(const Parsing::FunDecl& funDecl);
bool isGlobalVar(const Parsing::VarDecl& varDecl, const SymbolTable& symbolTable);

} // Semantics
#endif // CC_SEMANTICS_VARIABLE_SOLUTION_HPP