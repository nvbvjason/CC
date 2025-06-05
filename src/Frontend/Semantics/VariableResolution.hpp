#pragma once

#ifndef CC_SEMANTICS_VARIABLE_SOLUTION_HPP
#define CC_SEMANTICS_VARIABLE_SOLUTION_HPP

#include "ASTTraverser.hpp"
#include "ShortTypes.hpp"
#include "ASTParser.hpp"
#include "Frontend/SymbolTable.hpp"
#include "ASTTypes.hpp"

#include <string>

namespace Semantics {

class VariableResolution : public Parsing::ASTTraverser {
    using Storage = Parsing::Declaration::StorageClass;
    struct ScopeGuard {
        SymbolTable& table;
        explicit ScopeGuard(SymbolTable& t)
            : table(t) { table.addScope(); }
        ~ScopeGuard() { table.removeScope(); }
    };
    SymbolTable& m_symbolTable;
    i32 m_nameCounter = 0;
    bool m_valid = true;
public:
    explicit VariableResolution(SymbolTable& symbolTable)
        : m_symbolTable(symbolTable) {}
    bool resolve(Parsing::Program& program);
    void visit(Parsing::FunDecl& funDecl) override;
    void visit(Parsing::CompoundStmt& compoundStmt) override;
    void visit(Parsing::ForStmt& forStmt) override;
    void visit(Parsing::VarDecl& varDecl) override;

    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::FunCallExpr& functionCallExpr) override;
private:
    void reset();
    std::string makeTemporaryName(const std::string &name);
};

bool isValidVarDecl(const Parsing::VarDecl& varDecl, const SymbolTable& symbolTable, SymbolTable::ReturnedVarEntry prevEntry);
bool isValidVarDeclGlobal(const Parsing::VarDecl& varDecl, const SymbolTable::ReturnedVarEntry& prevEntry);

bool isValidFuncDecl(const Parsing::FunDecl& funDecl,
                     const SymbolTable& symbolTable,
                     const SymbolTable::ReturnedFuncEntry& returnedEntry);

bool isValidFuncCall(const Parsing::FunCallExpr& funCallExpr, const SymbolTable& symbolTable);
bool isValidVarExpr(const Parsing::VarExpr& varExpr, SymbolTable::ReturnedVarEntry returnedEntry);

bool duplicatesInArgs(const std::vector<std::string>& args);
inline bool isIllegalVarRedecl(const Parsing::VarDecl& varDecl, const SymbolTable::ReturnedVarEntry prevEntry)
{
    using Flag = SymbolTable::State;
    using Storage = Parsing::Declaration::StorageClass;
    return prevEntry.isSet(Flag::FromCurrentScope)
        && (!prevEntry.isSet(Flag::ExternalLinkage) || varDecl.storage != Storage::Extern);
}
inline bool hasInternalLinkageVar(const Parsing::VarDecl& varDecl)
{
    using Storage = Parsing::Declaration::StorageClass;
    return varDecl.storage == Storage::Static;
}
inline bool hasExternalLinkageVar(const Parsing::VarDecl& varDecl, bool global)
{
    using Storage = Parsing::Declaration::StorageClass;
    if (global && varDecl.storage != Storage::Static)
        return true;
    return varDecl.storage == Storage::Extern;
}
} // Semantics
#endif // CC_SEMANTICS_VARIABLE_SOLUTION_HPP