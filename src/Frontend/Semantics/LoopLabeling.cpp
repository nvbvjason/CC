#include "LoopLabeling.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"

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
    if (isOutsideSwitchStmt(caseStmt)) {
        m_valid = false;
        return;
    }
    if (isNonConstantInSwitchCase(caseStmt)) {
        m_valid = false;
        return;
    }
    const auto constantExpr = static_cast<const Parsing::ConstExpr*>(caseStmt.condition.get());
    const i32 value = std::get<i32>(constantExpr->value);
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
    whileStmt.identifier = makeTemporary("while");
    m_continueLabel = whileStmt.identifier;
    m_breakLabel = whileStmt.identifier;
    ASTTraverser::visit(whileStmt);
    m_continueLabel = continueTemp;
    m_breakLabel = breakTemp;
}

void LoopLabeling::visit(Parsing::DoWhileStmt& doWhileStmt)
{
    const std::string continueTemp = m_continueLabel;
    const std::string breakTemp = m_breakLabel;
    doWhileStmt.identifier = makeTemporary("do.While");
    m_continueLabel = doWhileStmt.identifier;
    m_breakLabel = doWhileStmt.identifier;
    ASTTraverser::visit(doWhileStmt);
    m_continueLabel = continueTemp;
    m_breakLabel = breakTemp;
}

void LoopLabeling::visit(Parsing::ForStmt& forStmt)
{
    const std::string continueTemp = m_continueLabel;
    const std::string breakTemp = m_breakLabel;
    forStmt.identifier = makeTemporary("for");
    m_continueLabel = forStmt.identifier;
    m_breakLabel = forStmt.identifier;
    ASTTraverser::visit(forStmt);
    m_continueLabel = continueTemp;
    m_breakLabel = breakTemp;
}

void LoopLabeling::visit(Parsing::SwitchStmt& switchStmt)
{
    const std::string breakTemp = m_breakLabel;
    const std::string switchTemp = m_switchLabel;
    switchStmt.identifier = makeTemporary("switch");
    m_breakLabel = switchStmt.identifier;
    m_switchLabel = switchStmt.identifier;
    const Type conditionType = switchStmt.condition->type->kind;
    m_case[switchStmt.identifier] = std::vector<i32>();
    ASTTraverser::visit(switchStmt);
    for (const i32 value : m_case[switchStmt.identifier])
        switchStmt.cases.push_back(
            std::make_unique<Parsing::ConstExpr>(value, std::make_unique<Parsing::VarType>(conditionType))
            );
    if (m_default.contains(switchStmt.identifier))
        switchStmt.hasDefault = true;
    m_breakLabel = breakTemp;
    m_switchLabel = switchTemp;
}
} // Semantics