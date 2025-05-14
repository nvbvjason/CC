#include "VariableResolution.hpp"

#include <utility>

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

void VariableResolution::visit(Parsing::Block& block)
{
    const std::unordered_set<std::string> declaredInScope = resolveScopeDecls(m_variables);
    ASTTraverser::visit(block);
    addCurrentScope(m_variables, declaredInScope);
}

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    const auto it = m_variables.find(varDecl.name);
    if (it != m_variables.end() && it->second.fromCurrentScope) {
        m_valid = false;
        return;
    }
    const std::string uniqueName = makeTemporaryName(varDecl.name);
    m_variables.emplace_hint(it, varDecl.name, MapEntry(uniqueName, true));
    varDecl.name = uniqueName;
    ASTTraverser::visit(varDecl);
}

void VariableResolution::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    if (assignmentExpr.lhs->kind != Parsing::Expr::Kind::Var) {
        m_valid = false;
        return;
    }
    ASTTraverser::visit(assignmentExpr);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    const auto it = m_variables.find(varExpr.name);
    if (it == m_variables.end()) {
        m_valid = false;
        return;
    }
    varExpr.name = it->second.name;
    ASTTraverser::visit(varExpr);
}

std::unordered_set<std::string> resolveScopeDecls(std::unordered_map<std::string, MapEntry> &map)
{
    std::unordered_set<std::string> declaredInScope;
    for (auto& pair : map) {
        if (pair.second.fromCurrentScope) {
            declaredInScope.insert(pair.first);
            pair.second.fromCurrentScope = false;
        }
    }
    return declaredInScope;
}

void addCurrentScope(std::unordered_map<std::string, MapEntry> &map, const std::unordered_set<std::string> &scope)
{
    for (auto it = map.begin(); it != map.end(); ) {
        if (it->second.fromCurrentScope) {
            it = map.erase(it);
            continue;
        }
        if (scope.contains(it->first))
            it->second.fromCurrentScope = true;
        ++it;
    }
}


std::string VariableResolution::makeTemporaryName(const std::string& name)
{
    return name + '.' + std::to_string(m_nameCounter++) + ".tmp";
}

} // Semantics