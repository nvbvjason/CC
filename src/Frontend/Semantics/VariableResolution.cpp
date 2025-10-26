#include "VariableResolution.hpp"
#include "ASTParser.hpp"
#include "DynCast.hpp"
#include "ASTTypes.hpp"

#include <unordered_set>

namespace {
using Storage = Parsing::Declaration::StorageClass;
}

namespace Semantics {

std::vector<Error> VariableResolution::resolve(Parsing::Program& program)
{
    ASTTraverser::visit(program);
    return std::move(m_errors);
}

void VariableResolution::visit(Parsing::FunDecl& funDecl)
{
    const SymbolTable::ReturnedEntry prevEntry = m_symbolTable.lookup(funDecl.name);
    validateFuncDecl(funDecl, m_symbolTable, prevEntry, m_errors);
    addFuncToSymbolTable(funDecl, prevEntry);
    ScopeGuard scopeGuard(m_symbolTable);
    m_symbolTable.setArgs(funDecl);
    ASTTraverser::visit(funDecl);
    m_symbolTable.clearArgs();
}

void validateFuncDecl(const Parsing::FunDecl& funDecl,
                     const SymbolTable& symbolTable,
                     const SymbolTable::ReturnedEntry& returnedEntry,
                     std::vector<Error>& errors)
{
    if (funDecl.storage == Storage::Static && symbolTable.inFunc())
        errors.emplace_back("Declare static function in function ", funDecl.location);
    if (duplicatesInArgs(funDecl.params))
        errors.emplace_back("Declare function of the same name as arg ", funDecl.location);
    if (symbolTable.inFunc() && funDecl.body != nullptr)
        errors.emplace_back("Define function in body of another one ", funDecl.location);
    if (!returnedEntry.contains())
        return;
    if (returnedEntry.isDefined() && funDecl.body != nullptr)
        errors.emplace_back("Function defined more than once ", funDecl.location);
    if (returnedEntry.typeBase->type != funDecl.type->type && returnedEntry.isFromCurrentScope())
        errors.emplace_back("Function defined more than once ", funDecl.location);
    if (returnedEntry.typeBase->type != funDecl.type->type && returnedEntry.isGlobal())
        errors.emplace_back("Functions with different return types ", funDecl.location);
    if (returnedEntry.hasExternalLinkage() && funDecl.storage == Storage::Static)
        errors.emplace_back("Functions with different linkage ", funDecl.location);
    if (returnedEntry.typeBase->type == Type::Function) {
        const auto funcType = dynCast<const Parsing::FuncType>(returnedEntry.typeBase.get());
        if (funcType->params.size() != funDecl.params.size())
            errors.emplace_back("Functions with different parameter count ", funDecl.location);
    }
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
    const SymbolTable::ReturnedEntry prevEntry = m_symbolTable.lookup(varDecl.name);
    validateVarDecl(varDecl, m_symbolTable, prevEntry, m_errors);
    addVarToSymbolTable(varDecl, prevEntry);
    ASTTraverser::visit(varDecl);
}

void validateVarDecl(const Parsing::VarDecl& varDecl,
                     const SymbolTable& symbolTable,
                     const SymbolTable::ReturnedEntry& prevEntry,
                     std::vector<Error>& errors)
{
    if (prevEntry.isInArgs())
        errors.emplace_back("Variable declarations with the same name as arg ", varDecl.location);
    if (!prevEntry.contains())
        return;
    if (!symbolTable.inFunc()) {
        validateVarDeclGlobal(varDecl, prevEntry, errors);
        return;
    }
    if (isIllegalVarRedecl(varDecl, prevEntry))
        errors.emplace_back("Illegal variable redeclaration in same scope ", varDecl.location);
    if (varDecl.storage == Storage::Extern
        && prevEntry.hasExternalLinkage()
        && prevEntry.typeBase->type != varDecl.type->type)
        errors.emplace_back("Two variable declarations with external linkage of same type ",
                            varDecl.location);
    if (prevEntry.typeBase->type != varDecl.type->type &&
        prevEntry.isFromCurrentScope() &&
        varDecl.storage != Storage::Extern)
        errors.emplace_back("Illegal variable redeclaration in same scope ", varDecl.location);
}

void validateVarDeclGlobal(const Parsing::VarDecl& varDecl,
                          const SymbolTable::ReturnedEntry& prevEntry,
                          std::vector<Error>& errors)
{
    if (prevEntry.typeBase->type != varDecl.type->type)
        errors.emplace_back("Previous global variable declaration of same name and type ", varDecl.location);
    if (varDecl.init != nullptr && prevEntry.isDefined())
        errors.emplace_back("Cannot define global variable more than once ", varDecl.location);
    if (hasInternalLinkageVar(varDecl) && !prevEntry.hasInternalLinkage())
        errors.emplace_back("Conflicting variable linkage ", varDecl.location);
    if (varDecl.storage == Storage::None && prevEntry.hasInternalLinkage())
        errors.emplace_back("Conflicting variable linkage ", varDecl.location);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    const SymbolTable::ReturnedEntry returnedEntry = m_symbolTable.lookup(varExpr.name);
    if (!isValidVarExpr(varExpr.location, returnedEntry, m_errors))
        return;
    if (returnedEntry.hasExternalLinkage() && !returnedEntry.isGlobal())
        varExpr.referingTo = ReferingTo::Extern;
    else if (!returnedEntry.isInArgs())
        varExpr.name = m_symbolTable.getUniqueName(varExpr.name);
    if (returnedEntry.hasExternalLinkage())
        varExpr.referingTo = ReferingTo::Extern;
    if (returnedEntry.hasInternalLinkage())
        varExpr.referingTo = ReferingTo::Static;
    varExpr.type = Parsing::deepCopy(*returnedEntry.typeBase);
    ASTTraverser::visit(varExpr);
}

bool isValidVarExpr(const i64 location,
                    const SymbolTable::ReturnedEntry& returnedEntry,
                    std::vector<Error>& errors)
{
    if (returnedEntry.isInArgs())
        return true;
    if (!returnedEntry.contains()) {
        errors.emplace_back("Cannot find variable declaration ", location);
        return false;
    }
    if (returnedEntry.typeBase->type == Type::Function) {
        errors.emplace_back("Refers to function ", location);
        return false;
    }
    return true;
}

void VariableResolution::visit(Parsing::FuncCallExpr& funcCallExpr)
{
    const SymbolTable::ReturnedEntry returnedEntry = m_symbolTable.lookup(funcCallExpr.name);
    if (!isValidFuncCall(funcCallExpr.location, returnedEntry, m_errors))
        return;
    const auto funcType = dynCast<const Parsing::FuncType>(returnedEntry.typeBase.get());
    funcCallExpr.type = Parsing::deepCopy(*funcType->returnType);
    ASTTraverser::visit(funcCallExpr);
}

bool isValidFuncCall(const i64 location,
                     const SymbolTable::ReturnedEntry& returnedEntry,
                     std::vector<Error>& errors)
{
    if (!returnedEntry.contains()) {
        errors.emplace_back("Cannot find function declaration ", location);
        return false;
    }
    if (returnedEntry.typeBase->type != Type::Function) {
        errors.emplace_back("Does not refer to function ", location);
        return false;
    }
    return true;
}

void VariableResolution::addFuncToSymbolTable(
    const Parsing::FunDecl& funDecl,
    const SymbolTable::ReturnedEntry& prevEntry) const
{
    bool global = !m_symbolTable.inFunc();
    if (prevEntry.contains() && prevEntry.isGlobal())
        global = true;
    const bool defined = funDecl.body != nullptr;
    const bool internal = prevEntry.hasInternalLinkage() || funDecl.storage == Storage::Static;
    const bool external = !prevEntry.hasInternalLinkage() && funDecl.storage != Storage::Static;
    m_symbolTable.addEntry(funDecl.name, funDecl.name,
                           *Parsing::deepCopy(*funDecl.type), internal, external, global, defined);
}


void VariableResolution::addVarToSymbolTable(
    Parsing::VarDecl& varDecl,
    const SymbolTable::ReturnedEntry& prevEntry)
{
    const bool global = !m_symbolTable.inFunc();
    const bool defined = prevEntry.isDefined()|| varDecl.init != nullptr;
    const bool internal = prevEntry.hasInternalLinkage() || hasInternalLinkageVar(varDecl);
    const bool external = !prevEntry.hasInternalLinkage() &&
                           hasExternalLinkageVar(varDecl, !m_symbolTable.inFunc());
    if (!m_symbolTable.inFunc() || varDecl.storage == Storage::Extern) {
        m_symbolTable.addEntry(
            varDecl.name, varDecl.name, *varDecl.type,
            internal, external, global, defined);
    }
    else {
        const std::string uniqueName = makeTemporaryName(varDecl.name);
        m_symbolTable.addEntry(
            varDecl.name, uniqueName, *varDecl.type,
            internal, external, global, defined);
        varDecl.name = uniqueName;
    }
}

std::string VariableResolution::makeTemporaryName(const std::string& name)
{
    return name + '.' + std::to_string(m_nameCounter++) + ".tmp";
}

} // Semantics