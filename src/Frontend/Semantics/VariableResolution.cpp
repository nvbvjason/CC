#include "VariableResolution.hpp"

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
    std::unordered_map<std::string, MapEntry> copyMap = copyMapForBlock(m_identifiers);
    std::swap(copyMap, m_identifiers);
    ASTTraverser::visit(block);
    std::swap(copyMap, m_identifiers);
}

void VariableResolution::visit(Parsing::ForStmt& forStmt)
{
    std::unordered_map<std::string, MapEntry> copyMap = copyMapForBlock(m_identifiers);
    std::swap(copyMap, m_identifiers);
    ASTTraverser::visit(forStmt);
    std::swap(copyMap, m_identifiers);
}

void VariableResolution::visit(Parsing::FunDecl& funDecl)
{
    if (containsDuplicate(funDecl.params)) {
        m_valid = false;
        return;
    }
    const auto it = m_identifiers.find(funDecl.name);
    if (it != m_identifiers.end()) {
        if (it->second.fromCurrentScope && !it->second.hasLinkage) {
            m_valid = false;
            return;
        }

    }
    m_identifiers.emplace_hint(it, funDecl.name, MapEntry(funDecl.name, true, true));
    for (const std::string& arg : funDecl.params)
        m_args.insert(arg);
    ASTTraverser::visit(funDecl);
    m_args.clear();
}

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    if (m_args.contains(varDecl.name)) {
        m_valid = false;
        return;
    }
    const auto it = m_identifiers.find(varDecl.name);
    if (it != m_identifiers.end() && it->second.fromCurrentScope) {
        m_valid = false;
        return;
    }
    const std::string uniqueName = makeTemporaryName(varDecl.name);
    m_identifiers.emplace_hint(it, varDecl.name, MapEntry(uniqueName, true, false));
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

void VariableResolution::visit(Parsing::FunCallExpr& funCallExpr)
{
    const auto it = m_identifiers.find(funCallExpr.name);
    if (it == m_identifiers.end()) {
        m_valid = false;
        return;
    }
    ASTTraverser::visit(funCallExpr);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    if (!m_args.contains(varExpr.name)) {
        const auto it = m_identifiers.find(varExpr.name);
        if (it == m_identifiers.end()) {
            m_valid = false;
            return;
        }
        varExpr.name = it->second.uniqueName;
    }
    ASTTraverser::visit(varExpr);
}

bool containsDuplicate(const std::vector<std::string>& args)
{
    std::unordered_set<std::string> duplicates;
    for (const std::string& arg : args) {
        if (duplicates.contains(arg))
            return true;
        duplicates.insert(arg);
    }
    return false;
}


std::string VariableResolution::makeTemporaryName(const std::string& name)
{
    return name + '.' + std::to_string(m_nameCounter++) + ".tmp";
}

std::unordered_map<std::string, MapEntry> copyMapForBlock(const std::unordered_map<std::string, MapEntry> &map)
{
    std::unordered_map<std::string, MapEntry> copyMap;
    for (const auto& [fst, snd] : map)
        copyMap.emplace(fst, MapEntry(snd.uniqueName, false, snd.hasLinkage));
    return copyMap;
}
} // Semantics