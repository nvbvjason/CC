#include "VariableResolution.hpp"

namespace Semantics {

bool VariableResolution::resolve(Parsing::Program& program)
{
    m_valid = true;
    program.accept(*this);
    return m_valid;
}

void VariableResolution::visit(Parsing::Program& program)
{
    ScopeRAII scopeRAII(m_scopeStack);
    ASTTraverser::visit(program);
}

void VariableResolution::visit(Parsing::FunDecl& funDecl)
{
    if (!validateFunctionDeclaration(funDecl, m_functions, m_scopeStack, inFunction())) {
        m_valid = false;
        return;
    }
    if (shouldSkipFunDecl(funDecl, m_scopeStack, inFunction())) {
        m_functions.emplace(
            funDecl.name,
            FunctionClarification(funDecl.params, funDecl.body != nullptr, funDecl.storage)
            );
        m_scopeStack.addDecl(funDecl.name, funDecl.name,
                             Variable::Type::Function, StorageClass::ExternFunction);
    }
    m_scopeStack.addArgs(funDecl.params);
    ASTTraverser::visit(funDecl);
    m_scopeStack.clearArgs();
}

void VariableResolution::visit(Parsing::Block& block)
{
    ScopeRAII scopeRAII(m_scopeStack);
    ASTTraverser::visit(block);
}

void VariableResolution::visit(Parsing::ForStmt& forStmt)
{
    ScopeRAII scopeRAII(m_scopeStack);
    ASTTraverser::visit(forStmt);
}

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    if (!allowedVarDecl(m_scopeStack, varDecl, inFunction())) {
        m_valid = false;
        return;
    }
    if (!shouldSkipVarDecl(varDecl, m_scopeStack, inFunction())) {
        std::string newName = varDecl.name;
        if (!(varDecl.storage == StorageClass::ExternGlobal ||
              varDecl.storage == StorageClass::GlobalScopeDeclaration ||
              varDecl.storage == StorageClass::ExternLocal ||
              varDecl.storage == StorageClass::GlobalDefinition)) {
            newName = makeTemporary(varDecl.name);
        }
        if (varDecl.storage == StorageClass::StaticGlobalInitialized ||
            varDecl.storage == StorageClass::StaticGlobalTentative ||
            varDecl.storage == StorageClass::StaticLocal)
            newName = makeStatic(varDecl.name);
        if (varDecl.storage == StorageClass::ExternLocal)
            m_scopeStack.addExternGlobal(varDecl.name);
        m_scopeStack.addDecl(varDecl.name, newName,
                             Variable::Type::Int, varDecl.storage);
        varDecl.name = newName;
    }
    ASTTraverser::visit(varDecl);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    if (notDeclared(m_scopeStack, varExpr)) {
        m_valid = false;
        return;
    }
    auto [newName, success] = m_scopeStack.tryCall(varExpr.name, Variable::Type::Int);
    varExpr.name = newName;
    ASTTraverser::visit(varExpr);
}

void VariableResolution::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    ASTTraverser::visit(assignmentExpr);
}

void VariableResolution::visit(Parsing::FunCallExpr& funCallExpr)
{
    if (!isFunctionCallValid(m_functions, m_scopeStack, funCallExpr))
        m_valid = false;
    ASTTraverser::visit(funCallExpr);
}

bool validateFunctionDeclaration(const Parsing::FunDecl& funDecl,
                                const std::unordered_map<std::string, FunctionClarification>& functions,
                                const ScopeStack& scopeStack,
                                const bool inFunction)
{
    if (inFunction && funDecl.storage == Parsing::Declaration::StorageClass::StaticLocalFunction)
        return false;
    if (isInvalidInFunctionBody(funDecl, inFunction))
        return false;
    if (hasDuplicateParameters(funDecl.params))
        return false;
    if (!matchesExistingDeclaration(funDecl, functions))
        return false;
    if (inFunction && conflictingIdenInScope(funDecl, scopeStack))
        return false;
    if (conflictingIdenGlobal(funDecl, scopeStack))
        return false;
    return true;
}

bool conflictingIdenInScope(const Parsing::FunDecl& funDecl,
                            const ScopeStack& scopeStack)
{
    return scopeStack.existInInnerMost(funDecl.name, funDecl.storage);
}

bool conflictingIdenGlobal(const Parsing::FunDecl& funDecl, const ScopeStack& scopeStack)
{
    auto [varIden, found] = scopeStack.showIdenGlobal(funDecl.name);
    if (!found)
        return false;
    if (varIden.type != Variable::Type::Function)
        return true;
    return false;
}


bool matchesExistingDeclaration(const Parsing::FunDecl& funDecl,
                                const std::unordered_map<std::string, FunctionClarification>& functions)
{
    using StorageClass = Parsing::Declaration::StorageClass;
    const auto it = functions.find(funDecl.name);
    if (it == functions.end())
        return true;
    if (it->second.defined && funDecl.body != nullptr)
        return false;
    if (it->second.storage == StorageClass::ExternFunction &&
        funDecl.storage == StorageClass::StaticGlobalInitialized)
        return false;
    return it->second.args.size() == funDecl.params.size();
}

bool isInvalidInFunctionBody(const Parsing::FunDecl& function,
                              const bool inFunction)
{
    return function.body != nullptr && inFunction;
}

bool hasDuplicateParameters(const std::vector<std::string>& names)
{
    std::unordered_set<std::string> duplicates;
    for (const std::string& name : names) {
        if (duplicates.contains(name))
            return true;
        duplicates.insert(name);
    }
    return false;
}

bool allowedVarDecl(const ScopeStack& variableStack,
                    const Parsing::VarDecl& varDecl,
                    const bool inFunction)
{
    if (variableStack.inArgs(varDecl.name))
        return false;
    if (!inFunction && allowedVarDeclGlobal(variableStack, varDecl))
        return true;
    if (allowedInnermost(variableStack, varDecl))
        return true;
    return false;
}

bool allowedInnermost(const ScopeStack& scopeStack,
                      const Parsing::VarDecl& varDecl)
{
    auto [varGlobal, globalFound] = scopeStack.showIdenGlobal(varDecl.name);
    if (globalFound && varGlobal.type == Variable::Type::Function &&
        varDecl.storage == Parsing::Declaration::StorageClass::ExternLocal)
        return false;
    auto [varInner, found] = scopeStack.showIdenInnermost(varDecl.name);
    if (!found)
        return true;
    if (varInner.storage == Parsing::Declaration::StorageClass::ExternLocal &&
        varDecl.storage == Parsing::Declaration::StorageClass::ExternLocal)
        return true;
    return false;
}

bool allowedVarDeclGlobal(const ScopeStack& variableStack, const Parsing::VarDecl& varDecl)
{
    using StorageClass = Parsing::Declaration::StorageClass;
    if (varDecl.storage == StorageClass::ExternGlobal)
        return true;
    auto [idenVar, found] = variableStack.showIdenInnermost(varDecl.name);
    if (!found)
        return true;
    if (idenVar.type == Variable::Type::Function)
        return false;
    if (idenVar.storage == StorageClass::StaticGlobalTentative &&
        varDecl.storage == StorageClass::GlobalDefinition)
        return false;
    if (idenVar.storage == StorageClass::ExternLocal &&
        varDecl.storage == StorageClass::StaticGlobalInitialized)
        return false;
    if (idenVar.storage == StorageClass::GlobalDefinition &&
        varDecl.storage == StorageClass::GlobalDefinition)
        return false;
    return true;
}

bool notDeclared(const ScopeStack& variableStack, const Parsing::VarExpr& varExpr)
{
    auto [name, found] = variableStack.tryCall(varExpr.name, Variable::Type::Int);
    return !found;
}

bool isFunctionCallValid(const std::unordered_map<std::string, FunctionClarification>& functions,
                         const ScopeStack& scopeStack,
                         const Parsing::FunCallExpr& funCallExpr)
{
    auto [str, fine] = scopeStack.tryCall(funCallExpr.identifier, Variable::Type::Function);
    if (!fine)
        return false;
    const auto it = functions.find(funCallExpr.identifier);
    if (functionNotFound(functions, it))
        return false;
    return it->second.args.size() == funCallExpr.args.size();
}

bool shouldSkipVarDecl(const Parsing::VarDecl& varDecl, const ScopeStack& variableStack, bool inFunction)
{
    using StorageClass = Parsing::Declaration::StorageClass;
    auto [idenVar, found] = variableStack.showIden(varDecl.name);
    if (!found)
        return false;
    if (varDecl.storage == StorageClass::ExternGlobal ||
        varDecl.storage == StorageClass::GlobalScopeDeclaration)
        return true;
    return false;
}

bool shouldSkipFunDecl(const Parsing::FunDecl& funDecl, const ScopeStack& variableStack, bool inFunction)
{
    return true;
}

bool functionNotFound(
    const std::unordered_map<std::string, FunctionClarification>& functions,
    const std::unordered_map<std::string, FunctionClarification>::const_iterator& functionsIt)
{
    return functionsIt == functions.end();
}

bool internalLinkage(Parsing::Declaration::StorageClass storageClass)
{
    using StorageClass = Parsing::Declaration::StorageClass;
    if (storageClass == StorageClass::StaticGlobalInitialized ||
        storageClass == StorageClass::StaticGlobalTentative)
        return true;
    return false;
}

} // Semantics