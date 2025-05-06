#include "Switch.hpp"
#include "ASTParser.hpp"

namespace Semantics {
bool Switch::programValidate(const Parsing::Program& program)
{
    m_valid = true;
    program.accept(*this);
    return m_valid;
}

void Switch::visit(const Parsing::SwitchStmt& switchStmt)
{
    m_case[switchStmt.identifier] = std::vector<i32>();
    switchStmt.body->accept(*this);
}

void Switch::visit(const Parsing::CaseStmt& caseStmt)
{
    if (caseStmt.identifier.empty())
        m_valid = false;
    if (caseStmt.condition->kind != Parsing::Expr::Kind::Constant)
        m_valid = false;
    const auto constantExpr = static_cast<const Parsing::ConstExpr*>(caseStmt.condition.get());
    const i32 value = constantExpr->value;
    std::vector<i32>& vec = m_case[caseStmt.identifier];
    if (std::ranges::find(vec, value) != vec.end())
        m_valid = false;
    else
        vec.push_back(value);
    ConstASTTraverser::visit(caseStmt);
}

void Switch::visit(const Parsing::DefaultStmt& defaultStmt)
{
    if (m_default.contains(defaultStmt.identifier))
        m_valid = false;
    m_default.insert(defaultStmt.identifier);
    if (defaultStmt.identifier.empty())
        m_valid = false;
    ConstASTTraverser::visit(defaultStmt);
}
} // Semantics