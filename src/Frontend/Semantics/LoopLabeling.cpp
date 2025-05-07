#include "LoopLabeling.hpp"
#include "ASTParser.hpp"

namespace Semantics {
bool LoopLabeling::programValidate(Parsing::Program& program)
{
    m_valid = true;
    program.accept(*this);
    return m_valid;
}

void LoopLabeling::visit(Parsing::SwitchStmt& switchStmt)
{
    m_case[switchStmt.identifier] = std::vector<i32>();
    ASTTraverser::visit(switchStmt);
    for (const i32 value : m_case[switchStmt.identifier])
        switchStmt.cases.push_back(
            std::make_unique<Parsing::ConstExpr>(value)
            );
    if (m_default.contains(switchStmt.identifier))
        switchStmt.hasDefault = true;
}

void LoopLabeling::visit(Parsing::CaseStmt& caseStmt)
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
    ASTTraverser::visit(caseStmt);
}

void LoopLabeling::visit(Parsing::DefaultStmt& defaultStmt)
{
    if (m_default.contains(defaultStmt.identifier))
        m_valid = false;
    m_default.insert(defaultStmt.identifier);
    if (defaultStmt.identifier.empty())
        m_valid = false;
    ASTTraverser::visit(defaultStmt);
}

void LoopLabeling::visit(Parsing::ContinueStmt& continueStmt)
{
    if (!m_valid)
        return;
    if (continueStmt.identifier.empty())
        m_valid = false;
}

void LoopLabeling::visit(Parsing::BreakStmt& breakStmt)
{
    if (!m_valid)
        return;
    if (breakStmt.identifier.empty())
        m_valid = false;
}
} // Semantics