#pragma once

#include "ASTTraverser.hpp"
#include "ShortTypes.hpp"
#include "ASTParser.hpp"
#include "Frontend/SymbolTable.hpp"
#include "Error.hpp"

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
    std::vector<Error> m_errors;
public:
    explicit VariableResolution(SymbolTable& symbolTable)
        : m_symbolTable(symbolTable) {}
    std::vector<Error> resolve(Parsing::Program& program);
    void visit(Parsing::FunDeclaration& funDecl) override;
    void visit(Parsing::VarDecl& varDecl) override;
    void visit(Parsing::CompoundStmt& compoundStmt) override;
    void visit(Parsing::ForStmt& forStmt) override;

    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::FuncCallExpr& funcCallExpr) override;

    void addVarToSymbolTable(Parsing::VarDecl& varDecl, const SymbolTable::ReturnedEntry& prevEntry);
    void addFuncToSymbolTable(
        const Parsing::FunDeclaration& funDecl,
        const SymbolTable::ReturnedEntry& prevEntry
    ) const;
private:
    std::string makeTemporaryName(const std::string &name);
};

void validateVarDecl(
    const Parsing::VarDecl& varDecl,
    const SymbolTable& symbolTable,
    const SymbolTable::ReturnedEntry& prevEntry,
    std::vector<Error>& errors
);
void validateFuncDecl(const Parsing::FunDeclaration& funDecl,
                     const SymbolTable& symbolTable,
                     const SymbolTable::ReturnedEntry& returnedEntry,
                     std::vector<Error>& errors);
void validateVarDeclGlobal(const Parsing::VarDecl& varDecl,
                           const SymbolTable::ReturnedEntry& prevEntry,
                           std::vector<Error>& errors);
bool isValidFuncCall(i64 location,
                     const SymbolTable::ReturnedEntry& returnedEntry,
                     std::vector<Error>& errors);
bool isValidVarExpr(i64 location, const SymbolTable::ReturnedEntry& returnedEntry, std::vector<Error>& errors);
bool duplicatesInArgs(const std::vector<std::string>& args);
inline bool isIllegalVarRedecl(const Parsing::VarDecl& varDecl, const SymbolTable::ReturnedEntry& prevEntry)
{
    using Storage = Parsing::Declaration::StorageClass;
    return prevEntry.isFromCurrentScope()
        && (!prevEntry.hasExternalLinkage() || varDecl.storage != Storage::Extern);
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