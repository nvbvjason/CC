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
    if (!isFunctionDeclarationValid(funDecl, m_functions, m_scopeStack, m_inFunction)) {
        m_valid = false;
        return;
    }
    m_functions.emplace(
        funDecl.name,
        FunctionClarification(funDecl.params, funDecl.body != nullptr, funDecl.storageClass)
        );
    m_scopeStack.addDecl(funDecl.name, funDecl.name,
                         Variable::Type::Function, StorageClass::ExternFunction);
    m_scopeStack.addArgs(funDecl.params);
    m_inFunction = true;
    ASTTraverser::visit(funDecl);
    m_inFunction = false;
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
    if (!allowedVarDecl(m_scopeStack, varDecl, m_inFunction)) {
        m_valid = false;
        return;
    }
    const std::string newUniqueName = makeTemporary(varDecl.name);
    m_scopeStack.addDecl(varDecl.name, newUniqueName,
                         Variable::Type::Int, varDecl.storageClass);
    varDecl.name = newUniqueName;
    ASTTraverser::visit(varDecl);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    if (notDeclared(m_scopeStack, varExpr)) {
        m_valid = false;
        return;
    }
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

bool isFunctionDeclarationValid(const Parsing::FunDecl& funDecl,
                                const std::unordered_map<std::string, FunctionClarification>& functions,
                                const ScopeStack& scopeStack,
                                const bool inFunction)
{
    if (isInvalidInFunctionBody(funDecl, inFunction))
        return false;
    if (hasDuplicateParameters(funDecl.params))
        return false;
    if (!matchesExistingDeclaration(funDecl, functions))
        return false;
    if (idenAlreadyDeclaredInScope(funDecl, scopeStack, inFunction))
        return false;
    if (funDecl.storageClass == Parsing::Declaration::StorageClass::StaticGlobal && inFunction)
        return false;
    return true;
}

bool idenAlreadyDeclaredInScope(const Parsing::FunDecl& funDecl,
                                const ScopeStack& scopeStack,
                                const bool inFunction)
{
    return inFunction && scopeStack.existInInnerMost(funDecl.name, funDecl.storageClass);
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
        funDecl.storageClass == StorageClass::StaticGlobal)
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
    return !variableStack.existInInnerMost(varDecl.name, varDecl.storageClass);
}

bool allowedVarDeclGlobal(const ScopeStack& variableStack, const Parsing::VarDecl& varDecl)
{
    return variableStack.tryDeclare(varDecl.name, Variable::Type::Int, varDecl.storageClass);
}

bool notDeclared(const ScopeStack& variableStack, const Parsing::VarExpr& varExpr)
{
    auto [name, err] = variableStack.tryCall(varExpr.name, Variable::Type::Int);
    return !err;
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

bool functionNotFound(
    const std::unordered_map<std::string, FunctionClarification>& functions,
    const std::unordered_map<std::string, FunctionClarification>::const_iterator& functionsIt)
{
    return functionsIt == functions.end();
}

} // Semantics