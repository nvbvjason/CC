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
    std::unordered_map<std::string, MapEntry> copyMap = copyMapForBlock(m_variables);
    std::swap(copyMap, m_variables);
    ASTTraverser::visit(block);
    std::swap(copyMap, m_variables);
}

void VariableResolution::visit(Parsing::ForStmt& forStmt)
{
    std::unordered_map<std::string, MapEntry> copyMap = copyMapForBlock(m_variables);
    std::swap(copyMap, m_variables);
    ASTTraverser::visit(forStmt);
    std::swap(copyMap, m_variables);
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

std::string VariableResolution::makeTemporaryName(const std::string& name)
{
    return name + '.' + std::to_string(m_nameCounter++) + ".tmp";
}

std::unordered_map<std::string, MapEntry> copyMapForBlock(const std::unordered_map<std::string, MapEntry> &map)
{
    std::unordered_map<std::string, MapEntry> copyMap;
    for (const auto& [fst, snd] : map)
        copyMap.emplace(fst, MapEntry(snd.name, false));
    return copyMap;
}


} // Semantics