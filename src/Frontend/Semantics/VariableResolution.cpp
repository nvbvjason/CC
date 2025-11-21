#include "VariableResolution.hpp"
#include "ASTParser.hpp"
#include "DynCast.hpp"
#include "ASTTypes.hpp"
#include "ASTUtils.hpp"
#include "TypeConversion.hpp"

#include <unordered_set>

namespace Semantics {

std::vector<Error> VariableResolution::resolve(Parsing::Program& program)
{
    ASTTraverser::visit(program);
    return std::move(m_errors);
}

void VariableResolution::visit(Parsing::StructuredDecl& structuredDecl)
{
    const std::string nameBefore = structuredDecl.identifier;
    const auto structuredEntry = m_symbolTable.lookupStructuredEntry(structuredDecl.identifier);
    if (structuredEntry.isFromCurrentScope() &&
        structuredDecl.type != structuredEntry.typeBase->type) {
        addError("Cannot define union and struct of same name in same scope", structuredDecl.location);
        return;
    }
    std::string uniqueName;
    if (isNewType(structuredEntry)) {
        uniqueName = makeTemporaryName(structuredDecl.identifier);
        structuredDecl.identifier = uniqueName;
        m_symbolTable.addStructuredEntry(nameBefore,
                                     structuredDecl.identifier,
                                     Parsing::StructuredType(
                                         structuredDecl.type,
                                         structuredDecl.identifier,
                                         structuredDecl.location
                                     ),
                                     !structuredDecl.members.empty());
    }
    else {
        const auto structuredType = dynCast<const Parsing::StructuredType>(structuredEntry.typeBase.get());
        uniqueName = structuredType->identifier;
    }
    ASTTraverser::visit(structuredDecl);
    if (!structuredDecl.members.empty()) {
        if (structuredEntry.isDefined() && structuredEntry.isFromCurrentScope()) {
            addError("Cannot redefine struct in same scope", structuredDecl.location);
            return;
        }
        if (duplicateIdentifierInMembers(structuredDecl.members)) {
            addError("Cannot have duplicate identifiers in struct", structuredDecl.location);
            return;
        }
        m_varTable.addEntry(uniqueName, structuredDecl, m_errors);
    }
}

void VariableResolution::visit(Parsing::FuncDecl& funDecl)
{
    const SymbolTable::ReturnedEntry prevEntry = m_symbolTable.lookupEntry(funDecl.name);
    validateFuncDecl(funDecl, m_symbolTable, prevEntry);
    const auto funcType = dynCast<Parsing::FuncType>(funDecl.type.get());
    for (const auto& param : funcType->params)
        param->accept(*this);
    funcType->returnType->accept(*this);
    addFuncToSymbolTable(funDecl, prevEntry);
    if (funDecl.body) {
        FunctionGuard functionGuard(m_symbolTable, funDecl);
        ScopeGuard guard(m_symbolTable);
        funDecl.body->accept(*this);
    }
}

void VariableResolution::validateFuncDecl(const Parsing::FuncDecl& funDecl,
                                          const SymbolTable& symbolTable,
                                          const SymbolTable::ReturnedEntry& returnedEntry)
{
    checkFuncDeclForTypeVoid(funDecl);
    if (funDecl.storage == Storage::Static && symbolTable.inFunc())
        addError("Cannot declare a static function inside another function", funDecl.location);
    if (duplicatesInArgs(funDecl.params))
        addError("Declare function of the same name as arg", funDecl.location);
    if (symbolTable.inFunc() && funDecl.body != nullptr)
        addError("Define function in body of another one", funDecl.location);
    if (!returnedEntry.contains())
        return;
    if (returnedEntry.isDefined() && funDecl.body != nullptr)
        addError("Function defined more than once", funDecl.location);
    if (returnedEntry.typeBase->type != funDecl.type->type && returnedEntry.isFromCurrentScope())
        addError("Function defined more than once", funDecl.location);
    if (returnedEntry.typeBase->type != funDecl.type->type && returnedEntry.isGlobal())
        addError("Functions with different return types", funDecl.location);
    if (returnedEntry.hasExternalLinkage() && funDecl.storage == Storage::Static)
        addError("Functions with different linkage", funDecl.location);
    if (returnedEntry.typeBase->type == Type::Function) {
        const auto funcType = dynCast<const Parsing::FuncType>(returnedEntry.typeBase.get());
        if (funcType->params.size() != funDecl.params.size())
            addError("Functions with different parameter count", funDecl.location);
    }
}

void VariableResolution::checkFuncDeclForTypeVoid(const Parsing::FuncDecl& funDecl)
{
    const auto funcType = dynCast<const Parsing::FuncType>(funDecl.type.get());
    for (const auto& param : funcType->params) {
        if (param->type == Type::Void) {
            addError("Function with void param type", funDecl.location);
            continue;
        }
        if (isVoidArray(*param)) {
            addError("Functions with void array type", funDecl.location);
            continue;
        }
        if (isPointerToVoidArray(*param))
            addError("Functions with pointer void array type", funDecl.location);
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

void VariableResolution::handleVarDeclOfStructuredType(const Parsing::VarDecl& varDecl)
{
    const auto structuredType = dynCast<Parsing::StructuredType>(varDecl.type.get());
    const auto structuredEntry = m_symbolTable.lookupStructuredEntry(structuredType->identifier);
    if (structuredEntry.typeBase && structuredType->type != structuredEntry.typeBase->type)
        addError("Cannot declare with different structured type and same name", varDecl.location);
}

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    const SymbolTable::ReturnedEntry prevEntry = m_symbolTable.lookupEntry(varDecl.name);
    varDecl.type->accept(*this);
    if (isStructuredType(varDecl.type->type))
        handleVarDeclOfStructuredType(varDecl);
    validateVarDecl(varDecl, m_symbolTable, prevEntry);
    addVarToSymbolTable(varDecl, prevEntry);
    ASTTraverser::visit(varDecl);
}

void VariableResolution::validateVarDecl(const Parsing::VarDecl& varDecl,
                                         const SymbolTable& symbolTable,
                                         const SymbolTable::ReturnedEntry& prevEntry)
{
    if (isVoidArray(*varDecl.type))
        addError("Cannot void array", varDecl.location);
    if (isPointerToVoidArray(*varDecl.type))
        addError("Cannot declare pointer to void array", varDecl.location);
    if (varDecl.type->type == Type::Void)
        addError("Cannot declare with type of void", varDecl.location);
    if (prevEntry.isInArgs())
        addError("Variable declarations with the same name as arg", varDecl.location);
    if (!prevEntry.contains())
        return;
    if (!symbolTable.inFunc()) {
        validateVarDeclGlobal(varDecl, prevEntry);
        return;
    }
    if (isIllegalVarReDecl(varDecl, prevEntry))
        addError("Illegal variable redeclaration in same scope", varDecl.location);
    if (varDecl.storage == Storage::Extern
        && prevEntry.hasExternalLinkage()
        && prevEntry.typeBase->type != varDecl.type->type)
        addError("Two variable declarations with external linkage of same type",
                            varDecl.location);
    if (prevEntry.typeBase->type != varDecl.type->type &&
        prevEntry.isFromCurrentScope() &&
        varDecl.storage != Storage::Extern)
        addError("Illegal variable redeclaration in same scope", varDecl.location);
}

void VariableResolution::validateVarDeclGlobal(const Parsing::VarDecl& varDecl,
                                               const SymbolTable::ReturnedEntry& prevEntry)
{
    if (prevEntry.typeBase->type != varDecl.type->type)
        addError("Previous global variable declaration of same name and type", varDecl.location);
    if (varDecl.init != nullptr && prevEntry.isDefined())
        addError("Cannot define global variable more than once", varDecl.location);
    if (hasInternalLinkageVar(varDecl) && !prevEntry.hasInternalLinkage())
        addError("Conflicting variable linkage", varDecl.location);
    if (varDecl.storage == Storage::None && prevEntry.hasInternalLinkage())
        addError("Conflicting variable linkage", varDecl.location);
    if (varDecl.type->type == Type::Array && !Parsing::areEquivalentTypes(*varDecl.type, *prevEntry.typeBase))
        addError("Cannot define arrays with different types", varDecl.location);
}

void VariableResolution::visit(Parsing::StructuredType& structuredType)
{
    const auto entry = m_symbolTable.lookupStructuredEntry(structuredType.identifier);
    if (!entry.contains())
        addError("Use of undeclared struct type", structuredType.location);
    if (entry.typeBase && structuredType.type != entry.typeBase->type)
        addError("Union and struct conflict type", structuredType.location);
    if (entry.typeBase) {
        const auto returnedType = dynCast<const Parsing::StructuredType>(entry.typeBase.get());
        structuredType.identifier = returnedType->identifier;
    }
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    const SymbolTable::ReturnedEntry returnedEntry = m_symbolTable.lookupEntry(varExpr.name);
    if (isValidVarExpr(varExpr.location, returnedEntry)) {
        if (returnedEntry.hasExternalLinkage() && !returnedEntry.isGlobal())
            varExpr.referingTo = ReferingTo::Extern;
        else if (!returnedEntry.isInArgs())
            varExpr.name = m_symbolTable.getUniqueName(varExpr.name);
        if (returnedEntry.hasExternalLinkage())
            varExpr.referingTo = ReferingTo::Extern;
        if (returnedEntry.hasInternalLinkage())
            varExpr.referingTo = ReferingTo::Static;
        varExpr.type = Parsing::deepCopy(*returnedEntry.typeBase);
    }
    ASTTraverser::visit(varExpr);
}

bool VariableResolution::isValidVarExpr(const i64 location, const SymbolTable::ReturnedEntry& returnedEntry)
{
    if (returnedEntry.isInArgs())
        return true;
    if (!returnedEntry.contains()) {
        addError("Cannot find variable declaration", location);
        return false;
    }
    if (returnedEntry.typeBase->type == Type::Function) {
        addError("Refers to function", location);
        return false;
    }
    return true;
}

void VariableResolution::visit(Parsing::FuncCallExpr& funcCallExpr)
{
    const SymbolTable::ReturnedEntry returnedEntry = m_symbolTable.lookupEntry(funcCallExpr.name);
    if (isValidFuncCall(funcCallExpr.location, returnedEntry)) {
        const auto funcType = dynCast<const Parsing::FuncType>(returnedEntry.typeBase.get());
        funcCallExpr.type = Parsing::deepCopy(*funcType->returnType);
    }
    ASTTraverser::visit(funcCallExpr);
}

bool VariableResolution::isValidFuncCall(const i64 location, const SymbolTable::ReturnedEntry& returnedEntry)
{
    if (!returnedEntry.contains()) {
        addError("Cannot find function declaration", location);
        return false;
    }
    if (returnedEntry.typeBase->type != Type::Function) {
        addError("Does not refer to function", location);
        return false;
    }
    return true;
}

void VariableResolution::addFuncToSymbolTable(
    const Parsing::FuncDecl& funDecl,
    const SymbolTable::ReturnedEntry& prevEntry) const
{
    const bool defined = funDecl.body != nullptr;
    const bool internal = prevEntry.hasInternalLinkage() || funDecl.storage == Storage::Static;
    const bool external = !prevEntry.hasInternalLinkage() && funDecl.storage != Storage::Static;
    m_symbolTable.addEntry(funDecl.name, funDecl.name,
                           *Parsing::deepCopy(*funDecl.type), internal, external,
                           !m_symbolTable.inFunc(), defined);
}


void VariableResolution::addVarToSymbolTable(
    Parsing::VarDecl& varDecl,
    const SymbolTable::ReturnedEntry& prevEntry)
{
    if (prevEntry.typeBase && !Parsing::areEquivalentTypes(*varDecl.type, *prevEntry.typeBase)) {
        const bool global = !m_symbolTable.inFunc();
        const bool defined = varDecl.init != nullptr;
        const bool internal = hasInternalLinkageVar(varDecl);
        const bool external = hasExternalLinkageVar(varDecl, !m_symbolTable.inFunc());
        const std::string uniqueName = makeTemporaryName(varDecl.name);
        m_symbolTable.addEntry(
            varDecl.name, uniqueName, *varDecl.type,
            internal, external, global, defined);
        varDecl.name = uniqueName;
        return;
    }
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

bool isNewType(const SymbolTable::ReturnedStructuredEntry& prevEntry)
{
    if (prevEntry.typeBase == nullptr)
        return true;
    if (prevEntry.isFromCurrentScope())
        return false;
    return true;
}

std::string VariableResolution::makeTemporaryName(const std::string& name)
{
    return name + '.' + std::to_string(m_nameCounter++) + ".tmp";
}

} // Semantics