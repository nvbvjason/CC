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
    const SymbolTable::ReturnedFuncEntry prevEntry = m_symbolTable.lookupFunc(funDecl.name);
    if (!isValidFuncDecl(funDecl, m_symbolTable, prevEntry)) {
        m_valid = false;
        return;
    }
    bool global = !m_symbolTable.inFunc();
    if (prevEntry.isSet(Flag::Contains) && prevEntry.isSet(Flag::Global))
        global = true;
    const bool defined = funDecl.body != nullptr;
    const bool internal = prevEntry.isSet(Flag::InternalLinkage) || funDecl.storage == Storage::Static;
    const bool external = !prevEntry.isSet(Flag::InternalLinkage) && funDecl.storage != Storage::Static;
    m_symbolTable.addFuncEntry(funDecl.name, funDecl.params.size(), internal, external, global, defined);
    ScopeGuard scopeGuard(m_symbolTable);
    m_symbolTable.setArgs(funDecl);
    ASTTraverser::visit(funDecl);
    m_symbolTable.clearArgs();
}

bool isValidFuncDecl(const Parsing::FunDecl& funDecl,
                     const SymbolTable& symbolTable,
                     const SymbolTable::ReturnedFuncEntry& returnedEntry)
{
    if (funDecl.storage == Storage::Static && symbolTable.inFunc())
        return false;
    if (duplicatesInArgs(funDecl.params))
        return false;
    if (symbolTable.inFunc() && funDecl.body != nullptr)
        return false;
    if (!returnedEntry.isSet(Flag::Contains))
        return true;
    if (returnedEntry.argSize != funDecl.params.size())
        return false;
    if (returnedEntry.isSet(Flag::Defined) && funDecl.body != nullptr)
        return false;
    if (returnedEntry.type != funDecl.type->kind && returnedEntry.isSet(Flag::FromCurrentScope))
        return false;
    if (returnedEntry.type != funDecl.type->kind && returnedEntry.isSet(Flag::Global))
        return false;
    if (returnedEntry.isSet(Flag::ExternalLinkage) && funDecl.storage == Storage::Static)
        return false;
    return true;
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

void VariableResolution::visit(Parsing::CompoundStmt& compoundStmt)
{
    ScopeGuard scopeGuard(m_symbolTable);
    ASTTraverser::visit(compoundStmt);
}

void VariableResolution::visit(Parsing::ForStmt& forStmt)
{
    ScopeGuard scopeGuard(m_symbolTable);
    ASTTraverser::visit(forStmt);
}

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    const SymbolTable::ReturnedVarEntry prevEntry = m_symbolTable.lookupVar(varDecl.name);
    if (!isValidVarDecl(varDecl, m_symbolTable, prevEntry)) {
        m_valid = false;
        return;
    }
    const bool global = !m_symbolTable.inFunc();
    const bool defined = prevEntry.isSet(Flag::Defined) || varDecl.init != nullptr;
    const bool internal = prevEntry.isSet(Flag::InternalLinkage) || hasInternalLinkageVar(varDecl);
    const bool external = !prevEntry.isSet(Flag::InternalLinkage) && hasExternalLinkageVar(varDecl, !m_symbolTable.inFunc());
    if (!m_symbolTable.inFunc() || varDecl.storage == Storage::Extern) {
        m_symbolTable.addVarEntry(
            varDecl.name, varDecl.name, varDecl.type->kind,
            internal, external, global, defined);
    }
    else {
        const std::string uniqueName = makeTemporaryName(varDecl.name);
        m_symbolTable.addVarEntry(
            varDecl.name, uniqueName, varDecl.type->kind,
            internal, external, global, defined);
        varDecl.name = uniqueName;
    }
    ASTTraverser::visit(varDecl);
}

bool isValidVarDecl(const Parsing::VarDecl& varDecl, const SymbolTable& symbolTable,
                    const SymbolTable::ReturnedVarEntry prevEntry)
{
    if (prevEntry.isSet(Flag::InArgs))
        return false;
    if (!prevEntry.isSet(Flag::Contains))
        return true;
    if (!symbolTable.inFunc())
        return isValidVarDeclGlobal(varDecl, prevEntry);
    if (isIllegalVarRedecl(varDecl, prevEntry))
        return false;
    if (varDecl.storage == Storage::Extern
        && prevEntry.isSet(Flag::ExternalLinkage)
        && prevEntry.type != varDecl.type->kind)
        return false;
    if (prevEntry.type != varDecl.type->kind &&
        prevEntry.isSet(Flag::FromCurrentScope) &&
        varDecl.storage != Storage::Extern)
        return false;
    return true;
}

bool isValidVarDeclGlobal(const Parsing::VarDecl& varDecl, const SymbolTable::ReturnedVarEntry& prevEntry)
{
    if (prevEntry.type != varDecl.type->kind)
        return false;
    if (varDecl.init != nullptr && prevEntry.isSet(Flag::Defined))
        return false;
    if (hasInternalLinkageVar(varDecl) && !prevEntry.isSet(Flag::InternalLinkage))
        return false;
    if (varDecl.storage == Storage::None && prevEntry.isSet(Flag::InternalLinkage))
        return false;
    return true;
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    const SymbolTable::ReturnedVarEntry returnedEntry = m_symbolTable.lookupVar(varExpr.name);
    if (!isValidVarExpr(varExpr, returnedEntry)) {
        m_valid = false;
        return;
    }
    if (returnedEntry.isSet(Flag::ExternalLinkage) && !returnedEntry.isSet(Flag::Global))
        varExpr.referingTo = ReferingTo::Extern;
    else if (!returnedEntry.isSet(Flag::InArgs))
        varExpr.name = m_symbolTable.getUniqueName(varExpr.name);
    if (returnedEntry.isSet(Flag::InArgs))
        varExpr.referingTo = ReferingTo::Arg;
    if (returnedEntry.type == Type::I32)
        varExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
    else
        varExpr.type = std::make_unique<Parsing::VarType>(Type::I64);
    ASTTraverser::visit(varExpr);
}

bool isValidVarExpr(const Parsing::VarExpr& varExpr, const SymbolTable::ReturnedVarEntry returnedEntry)
{
    if (returnedEntry.isSet(Flag::InArgs))
        return true;
    if (!returnedEntry.isSet(Flag::Contains))
        return false;
    if (returnedEntry.type == Type::Function)
        return false;
    return true;
}

void VariableResolution::visit(Parsing::FunCallExpr& funCallExpr)
{
    if (!isValidFuncCall(funCallExpr, m_symbolTable)) {
        m_valid = false;
        return;
    }
    ASTTraverser::visit(funCallExpr);
}

bool isValidFuncCall(const Parsing::FunCallExpr& funCallExpr, const SymbolTable& symbolTable)
{
    const SymbolTable::ReturnedFuncEntry returnedEntry = symbolTable.lookupFunc(funCallExpr.name);
    if (!returnedEntry.isSet(Flag::Contains))
        return false;
    if (returnedEntry.type != Type::Function)
        return false;
    return true;
}

std::string VariableResolution::makeTemporaryName(const std::string& name)
{
    return name + '.' + std::to_string(m_nameCounter++) + ".tmp";
}

} // Semantics