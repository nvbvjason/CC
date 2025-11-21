#pragma once

#include "ASTTraverser.hpp"
#include "ShortTypes.hpp"
#include "ASTParser.hpp"
#include "Frontend/SymbolTable.hpp"
#include "Error.hpp"
#include "VarTable.hpp"

#include <string>
#include <unordered_set>

namespace Semantics {

class VariableResolution : public Parsing::ASTTraverser {
    using Storage = Parsing::Declaration::StorageClass;
    struct ScopeGuard {
        SymbolTable& table;
        explicit ScopeGuard(SymbolTable& t)
            : table(t) { table.addScope(); }
        ~ScopeGuard() { table.removeScope(); }
    };
    struct FunctionGuard {
        SymbolTable& table;
        Parsing::FuncDecl& funDecl;
        FunctionGuard(SymbolTable& table, Parsing::FuncDecl& funDecl)
            : table(table), funDecl(funDecl)
        {
            table.setArgs(funDecl);
        }
        ~FunctionGuard() { table.clearArgs(); }
    };
    SymbolTable& m_symbolTable;
    VarTable& m_varTable;
    i32 m_nameCounter = 0;
    std::vector<Error> m_errors;
public:
    explicit VariableResolution(SymbolTable& symbolTable, VarTable& varTable)
        : m_symbolTable(symbolTable), m_varTable(varTable) {}
    std::vector<Error> resolve(Parsing::Program& program);
    void visit(Parsing::FuncDecl& funDecl) override;
    void visit(Parsing::VarDecl& varDecl) override;
    void visit(Parsing::StructuredDecl& structuredDecl) override;

    void visit(Parsing::StructuredType& structuredType) override;

    void visit(Parsing::CompoundStmt& compoundStmt) override;
    void visit(Parsing::ForStmt& forStmt) override;
    void handleVarDeclOfStructuredType(const Parsing::VarDecl& varDecl);

    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::FuncCallExpr& funcCallExpr) override;

    void addVarToSymbolTable(Parsing::VarDecl& varDecl, const SymbolTable::ReturnedEntry& prevEntry);
    void addFuncToSymbolTable(
        const Parsing::FuncDecl& funDecl,
        const SymbolTable::ReturnedEntry& prevEntry
    ) const;
    void validateVarDecl(
        const Parsing::VarDecl& varDecl,
        const SymbolTable& symbolTable,
        const SymbolTable::ReturnedEntry& prevEntry);
    void validateFuncDecl(const Parsing::FuncDecl& funDecl,
                          const SymbolTable& symbolTable,
                          const SymbolTable::ReturnedEntry& returnedEntry);
    void validateVarDeclGlobal(const Parsing::VarDecl& varDecl,
                               const SymbolTable::ReturnedEntry& prevEntry);
    bool isValidFuncCall(i64 location, const SymbolTable::ReturnedEntry& returnedEntry);
    bool isValidVarExpr(i64 location, const SymbolTable::ReturnedEntry& returnedEntry);
private:
    void checkFuncDeclForTypeVoid(const Parsing::FuncDecl& funDecl);
    std::string makeTemporaryName(const std::string &name);
    void addError(const std::string& msg, const i64 location) { m_errors.emplace_back(msg, location); }
};

bool duplicatesInArgs(const std::vector<std::string>& args);
inline bool isIllegalVarReDecl(const Parsing::VarDecl& varDecl, const SymbolTable::ReturnedEntry& prevEntry)
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
inline bool hasExternalLinkageVar(const Parsing::VarDecl& varDecl, const bool global)
{
    using Storage = Parsing::Declaration::StorageClass;
    if (global && varDecl.storage != Storage::Static)
        return true;
    return varDecl.storage == Storage::Extern;
}

inline bool duplicateIdentifierInMembers(const std::vector<std::unique_ptr<Parsing::MemberDecl>>& memberDecls)
{
    std::unordered_set<std::string> identifiers;
    for (const auto& member : memberDecls) {
        if (identifiers.contains(member->identifier))
            return true;
        identifiers.insert(member->identifier);
    }
    return false;
}
bool isNewType(const SymbolTable::ReturnedStructuredEntry& prevEntry);
} // Semantics