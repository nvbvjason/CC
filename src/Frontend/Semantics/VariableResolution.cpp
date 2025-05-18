#include "VariableResolution.hpp"

#include <unordered_set>

#include "ASTParser.hpp"

namespace Semantics {

void VariableResolution::reset()
{
    m_nameCounter = 0;
    m_valid = true;
}

bool VariableResolution::resolve(Parsing::Program& program)
{
    reset();
    ASTTraverser::visit(program);
    return m_valid;
}

void VariableResolution::visit(Parsing::FunDecl& funDecl)
{
    if (!isValidFuncDecl(funDecl, symbolTable)) {
        m_valid = false;
        return;
    }
    symbolTable.addFuncEntry(funDecl.name, funDecl.params.size(), true);
    symbolTable.addScope();
    symbolTable.setArgs(funDecl.params);
    ASTTraverser::visit(funDecl);
    symbolTable.clearArgs();
    symbolTable.removeScope();
}

void VariableResolution::visit(Parsing::CompoundStmt& compoundStmt)
{
    symbolTable.addScope();
    ASTTraverser::visit(compoundStmt);
    symbolTable.removeScope();
}

void VariableResolution::visit(Parsing::ForStmt& forStmt)
{
    symbolTable.addScope();
    ASTTraverser::visit(forStmt);
    symbolTable.removeScope();
}

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    if (!isValidVarDecl(varDecl, symbolTable)) {
        m_valid = false;
        return;
    }
    const std::string uniqueName = makeTemporaryName(varDecl.name);
    symbolTable.addVarEntry(varDecl.name, uniqueName, false);
    ASTTraverser::visit(varDecl);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    if (!symbolTable.contains(varExpr.name) && !symbolTable.isInArgs(varExpr.name)) {
        m_valid = false;
        return;
    }
    ASTTraverser::visit(varExpr);
}

void VariableResolution::visit(Parsing::FunCallExpr& funCallExpr)
{
    if (!symbolTable.contains(funCallExpr.name)) {
        m_valid = false;
        return;
    }
    ASTTraverser::visit(funCallExpr);
}

std::string VariableResolution::makeTemporaryName(const std::string& name)
{
    return name + '.' + std::to_string(m_nameCounter++) + ".tmp";
}

bool duplicatesInArgs(const std::vector<std::string>& args)
{
    std::unordered_set<std::string> duplicates;
    for (const std::string& arg : args) {
        if (duplicates.contains(arg))
            return true;
        duplicates.insert(arg);
    }
    return false;
}

bool isValidVarDecl(const Parsing::VarDecl& varDecl, const SymbolTable& symbolTable)
{
    const SymbolTable::ReturnedVarEntry returnedEntry = symbolTable.lookupVar(varDecl.name);
    if (returnedEntry.inArgs)
        return false;
    if (!returnedEntry.contains)
        return true;
    if (returnedEntry.fromCurrentScope)
        return false;
    if (returnedEntry.wrongType && returnedEntry.fromCurrentScope)
        return false;
    return true;
}

bool isValidFuncDecl(const Parsing::FunDecl& funDecl, const SymbolTable& symbolTable)
{
    if (duplicatesInArgs(funDecl.params))
        return false;
    const SymbolTable::ReturnedFuncEntry returnedEntry = symbolTable.lookupFunc(funDecl.name);
    if (symbolTable.inFunction() && funDecl.body != nullptr)
        return false;
    if (!returnedEntry.contains)
        return true;
    if (returnedEntry.argSize != funDecl.params.size())
        return false;
    if (returnedEntry.wrongType && returnedEntry.fromCurrentScope)
        return false;
    return true;
}

} // Semantics