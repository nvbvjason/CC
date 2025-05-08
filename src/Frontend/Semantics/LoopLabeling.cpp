#include "LoopLabeling.hpp"
#include "ASTParser.hpp"

namespace Semantics {
bool LoopLabeling::programValidate(Parsing::Program& program)
{
    m_valid = true;
    ASTTraverser::visit(program);
    return m_valid;
}

void LoopLabeling::visit(Parsing::CaseStmt& caseStmt)
{
    caseStmt.identifier = m_switchLabel;
    if (isOutsideSwitchStmt(caseStmt))
        m_valid = false;
    if (isNonConstantInSwitchCase(caseStmt))
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
    defaultStmt.identifier = m_switchLabel;
    if (m_default.contains(defaultStmt.identifier))
        m_valid = false;
    m_default.insert(defaultStmt.identifier);
    if (defaultStmt.identifier.empty())
        m_valid = false;
    ASTTraverser::visit(defaultStmt);
}

void LoopLabeling::visit(Parsing::ContinueStmt& continueStmt)
{
    continueStmt.identifier = m_continueLabel;
    if (!m_valid)
        return;
    if (continueStmt.identifier.empty())
        m_valid = false;
}

void LoopLabeling::visit(Parsing::BreakStmt& breakStmt)
{
    breakStmt.identifier = m_breakLabel;
    if (!m_valid)
        return;
    if (breakStmt.identifier.empty())
        m_valid = false;
}

void LoopLabeling::visit(Parsing::WhileStmt& whileStmt)
{
    const std::string continueTemp = m_continueLabel;
    const std::string breakTemp = m_breakLabel;
    const std::string whileLabel = makeTemporary("while");
    m_continueLabel = whileLabel;
    m_breakLabel = whileLabel;
    ASTTraverser::visit(whileStmt);
    m_continueLabel = continueTemp;
    m_breakLabel = breakTemp;
    whileStmt.identifier = whileLabel;
}

void LoopLabeling::visit(Parsing::DoWhileStmt& doWhileStmt)
{
    const std::string continueTemp = m_continueLabel;
    const std::string breakTemp = m_breakLabel;
    const std::string doLabel = makeTemporary("do.While");
    m_continueLabel = doLabel;
    m_breakLabel = doLabel;
    ASTTraverser::visit(doWhileStmt);
    m_continueLabel = continueTemp;
    m_breakLabel = breakTemp;
    doWhileStmt.identifier = doLabel;
}

void LoopLabeling::visit(Parsing::ForStmt& forStmt)
{
    const std::string continueTemp = m_continueLabel;
    const std::string breakTemp = m_breakLabel;
    const std::string forLabel = makeTemporary("for");
    m_continueLabel = forLabel;
    m_breakLabel = forLabel;
    ASTTraverser::visit(forStmt);
    m_continueLabel = continueTemp;
    m_breakLabel = breakTemp;
    forStmt.identifier = forLabel;
}

void LoopLabeling::visit(Parsing::SwitchStmt& switchStmt)
{
    const std::string breakTemp = m_breakLabel;
    const std::string switchTemp = m_switchLabel;
    const std::string switchLabel = makeTemporary("switch");
    m_breakLabel = switchLabel;
    m_switchLabel = switchLabel;
    m_case[switchStmt.identifier] = std::vector<i32>();
    ASTTraverser::visit(switchStmt);
    for (const i32 value : m_case[switchStmt.identifier])
        switchStmt.cases.push_back(
            std::make_unique<Parsing::ConstExpr>(value)
            );
    if (m_default.contains(switchStmt.identifier))
        switchStmt.hasDefault = true;
    m_breakLabel = breakTemp;
    m_switchLabel = switchTemp;
    switchStmt.identifier = switchLabel;
}
} // Semantics