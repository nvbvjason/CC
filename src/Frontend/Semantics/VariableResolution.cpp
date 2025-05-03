#include "VariableResolution.hpp"

namespace Semantics {
bool VariableResolution::resolve()
{
    m_valid = true;
    m_counter = 0;
    variableMap.clear();
    program.accept(*this);
    return m_valid;
}

void VariableResolution::visit(Parsing::Declaration& declaration)
{
    if (!m_valid)
        return;
    if (variableMap.contains(declaration.name)) {
        m_valid = false;
        return;
    }
    std::string uniqueName = makeTemporary(declaration.name);
    variableMap[declaration.name] = uniqueName;
    declaration.name = uniqueName;
    if (declaration.init != nullptr)
        declaration.init->accept(*this);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    if (!m_valid)
        return;
    if (!variableMap.contains(varExpr.name)) {
        m_valid = false;
        return;
    }
    varExpr.name = variableMap.at(varExpr.name);
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
    if (!variableMap.contains(varExpr->name)) {
        m_valid = false;
        return;
    }
    varExpr->name = variableMap.at(varExpr->name);
    assignmentExpr.rhs->accept(*this);
}

std::string VariableResolution::makeTemporary(const std::string& name)
{
    return name + '.' + std::to_string(m_counter++);
}
} // Semantics