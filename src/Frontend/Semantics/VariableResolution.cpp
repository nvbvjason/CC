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

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    if (m_variables.contains(varDecl.name)) {
        m_valid = false;
        return;
    }
    const std::string uniqueName = makeTemporaryName(varDecl.name);
    m_variables.emplace(varDecl.name, uniqueName);
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
    varExpr.name = it->second;
    ASTTraverser::visit(varExpr);
}

std::string VariableResolution::makeTemporaryName(const std::string& name)
{
    return name + '.' + std::to_string(m_nameCounter++) + ".tmp";
}

} // Semantics