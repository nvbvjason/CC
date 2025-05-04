#include "VariableResolution.hpp"

namespace Semantics {
bool VariableResolution::resolve()
{
    m_valid = true;
    m_counter = 0;
    m_variableMap.clear();
    program.accept(*this);
    return m_valid;
}

void VariableResolution::visit(Parsing::Block& block)
{
    std::unordered_map<std::string, std::string> blockMap;
    blockMap.swap(m_variableMap);
    for (auto& blockItem : block.body)
        blockItem->accept(*this);
    blockMap.swap(m_variableMap);
}

void VariableResolution::visit(Parsing::Declaration& declaration)
{
    if (!m_valid)
        return;
    if (m_variableMap.contains(declaration.name)) {
        m_valid = false;
        return;
    }
    std::string uniqueName = makeTemporary(declaration.name);
    m_variableMap[declaration.name] = uniqueName;
    declaration.name = uniqueName;
    if (declaration.init != nullptr)
        declaration.init->accept(*this);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    if (!m_valid)
        return;
    if (!m_variableMap.contains(varExpr.name)) {
        m_valid = false;
        return;
    }
    varExpr.name = m_variableMap.at(varExpr.name);
}

void VariableResolution::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    if (!m_valid)
        return;
    if (assignmentExpr.lhs->kind != Parsing::Expr::Kind::Var) {
        m_valid = false;
        return;
    }
    auto varExpr = static_cast<Parsing::VarExpr*>(assignmentExpr.lhs.get());
    if (!m_variableMap.contains(varExpr->name)) {
        m_valid = false;
        return;
    }
    varExpr->name = m_variableMap.at(varExpr->name);
    assignmentExpr.rhs->accept(*this);
}

std::string VariableResolution::makeTemporary(const std::string& name)
{
    return name + '.' + std::to_string(m_counter++);
}
} // Semantics