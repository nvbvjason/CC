#include "VariableResolution.hpp"
#include "ASTParser.hpp"

#include <unordered_set>

namespace {
using Flag = SymbolTable::State;
using Storage = Parsing::Declaration::StorageClass;
}

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
    const bool isGlobal = isGlobalFunc(funDecl);
    symbolTable.addFuncEntry(funDecl.name, funDecl.params.size(), true, isGlobal);
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
    const bool isGlobal = isGlobalVar(varDecl, symbolTable);
    if (!symbolTable.inFunc() || varDecl.storage == Storage::ExternLocal) {
        symbolTable.addVarEntry(varDecl.name, varDecl.name, true, isGlobal);
    }
    else {
        const std::string uniqueName = makeTemporaryName(varDecl.name);
        symbolTable.addVarEntry(varDecl.name, uniqueName, false, isGlobal);
        varDecl.name = uniqueName;
    }
    ASTTraverser::visit(varDecl);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    if (!isValidVarExpr(varExpr, symbolTable)) {
        m_valid = false;
        return;
    }
    if (!symbolTable.isInArgs(varExpr.name))
        varExpr.name = symbolTable.getUniqueName(varExpr.name);
    ASTTraverser::visit(varExpr);
}

void VariableResolution::visit(Parsing::FunCallExpr& funCallExpr)
{
    if (!isValidFuncCall(funCallExpr, symbolTable)) {
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

bool isValidFuncCall(const Parsing::FunCallExpr& funCallExpr, const SymbolTable& symbolTable)
{
    const SymbolTable::ReturnedFuncEntry returnedEntry = symbolTable.lookupFunc(funCallExpr.name);
    if (!returnedEntry.isSet(Flag::Contains))
        return false;
    if (!returnedEntry.isSet(Flag::CorrectType))
        return false;
    return true;
}

bool isValidVarExpr(const Parsing::VarExpr& varExpr, const SymbolTable& symbolTable)
{
    if (symbolTable.isInArgs(varExpr.name))
        return true;
    const SymbolTable::ReturnedVarEntry returnedEntry = symbolTable.lookupVar(varExpr.name);
    if (!returnedEntry.isSet(Flag::Contains))
        return false;
    if (!returnedEntry.isSet(Flag::CorrectType))
        return false;
    return true;
}

bool isValidVarDecl(const Parsing::VarDecl& varDecl, const SymbolTable& symbolTable)
{
    const SymbolTable::ReturnedVarEntry prevEntry = symbolTable.lookupVar(varDecl.name);
    if (prevEntry.isSet(Flag::InArgs))
        return false;
    if (!prevEntry.isSet(Flag::Contains))
        return true;
    if (prevEntry.isSet(Flag::FromCurrentScope)) {
        if (!prevEntry.isSet(Flag::HasLinkage) || varDecl.storage != Storage::ExternLocal)
            return false;
    }
    if (prevEntry.isSet(Flag::CorrectType) && prevEntry.isSet(Flag::FromCurrentScope))
        return false;
    return true;
}

bool isValidFuncDecl(const Parsing::FunDecl& funDecl, const SymbolTable& symbolTable)
{
    if (funDecl.storage == Storage::StaticLocalFunction)
        return false;
    if (duplicatesInArgs(funDecl.params))
        return false;
    if (symbolTable.inFunc() && funDecl.body != nullptr)
        return false;
    const SymbolTable::ReturnedFuncEntry returnedEntry = symbolTable.lookupFunc(funDecl.name);
    if (!returnedEntry.isSet(Flag::Contains))
        return true;
    if (returnedEntry.argSize != funDecl.params.size())
        return false;
    if (!returnedEntry.isSet(Flag::CorrectType) && returnedEntry.isSet(Flag::FromCurrentScope))
        return false;
    if (returnedEntry.isSet(Flag::IsGlobal) != isGlobalFunc(funDecl))
        return false;
    return true;
}

bool isGlobalFunc(const Parsing::FunDecl& funDecl)
{
    return funDecl.storage != Storage::StaticGlobalInitialized
        && funDecl.storage != Storage::StaticGlobalTentative;
}

bool isGlobalVar(const Parsing::VarDecl& varDecl, const SymbolTable& symbolTable)
{
    return varDecl.storage != Storage::StaticGlobalInitialized
        && varDecl.storage != Storage::StaticGlobalTentative;
}

} // Semantics