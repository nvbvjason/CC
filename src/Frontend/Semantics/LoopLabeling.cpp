#include "LoopLabeling.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"

#include <cassert>

namespace Semantics {
bool LoopLabeling::programValidate(Parsing::Program& program)
{
    valid = true;
    ASTTraverser::visit(program);
    return valid;
}

void LoopLabeling::visit(Parsing::CaseStmt& caseStmt)
{
    caseStmt.identifier = switchLabel;
    if (isOutsideSwitchStmt(caseStmt)) {
        valid = false;
        return;
    }
    if (isNonConstantInSwitchCase(caseStmt)) {
        valid = false;
        return;
    }
    const auto constantExpr = static_cast<const Parsing::ConstExpr*>(caseStmt.condition.get());
    switch (conditionType) {
        case Type::I32: {
            processSwitchCase<i32>(constantExpr, switchCases, switchLabel, caseStmt, valid);
            break;
        }
        case Type::I64: {
            processSwitchCase<i64>(constantExpr, switchCases, switchLabel, caseStmt, valid);
            break;
        }
        case Type::U32: {
            processSwitchCase<u32>(constantExpr, switchCases, switchLabel, caseStmt, valid);
            break;
        }
        case Type::U64: {
            processSwitchCase<u64>(constantExpr, switchCases, switchLabel, caseStmt, valid);
            break;
        }
        default:
            assert("Should never be reached");
            std::unreachable();
    }
    if (!valid)
        return;
    ASTTraverser::visit(caseStmt);
}

void LoopLabeling::visit(Parsing::DefaultStmt& defaultStmt)
{
    defaultStmt.identifier = switchLabel;
    if (m_default.contains(defaultStmt.identifier))
        valid = false;
    m_default.insert(defaultStmt.identifier);
    if (defaultStmt.identifier.empty())
        valid = false;
    ASTTraverser::visit(defaultStmt);
}

void LoopLabeling::visit(Parsing::ContinueStmt& continueStmt)
{
    continueStmt.identifier = continueLabel;
    if (!valid)
        return;
    if (continueStmt.identifier.empty())
        valid = false;
}

void LoopLabeling::visit(Parsing::BreakStmt& breakStmt)
{
    breakStmt.identifier = breakLabel;
    if (!valid)
        return;
    if (breakStmt.identifier.empty())
        valid = false;
}

void LoopLabeling::visit(Parsing::WhileStmt& whileStmt)
{
    const std::string continueTemp = continueLabel;
    const std::string breakTemp = breakLabel;
    whileStmt.identifier = makeTemporary("while");
    continueLabel = whileStmt.identifier;
    breakLabel = whileStmt.identifier;
    ASTTraverser::visit(whileStmt);
    continueLabel = continueTemp;
    breakLabel = breakTemp;
}

void LoopLabeling::visit(Parsing::DoWhileStmt& doWhileStmt)
{
    const std::string continueTemp = continueLabel;
    const std::string breakTemp = breakLabel;
    doWhileStmt.identifier = makeTemporary("do.While");
    continueLabel = doWhileStmt.identifier;
    breakLabel = doWhileStmt.identifier;
    ASTTraverser::visit(doWhileStmt);
    continueLabel = continueTemp;
    breakLabel = breakTemp;
}

void LoopLabeling::visit(Parsing::ForStmt& forStmt)
{
    const std::string continueTemp = continueLabel;
    const std::string breakTemp = breakLabel;
    forStmt.identifier = makeTemporary("for");
    continueLabel = forStmt.identifier;
    breakLabel = forStmt.identifier;
    ASTTraverser::visit(forStmt);
    continueLabel = continueTemp;
    breakLabel = breakTemp;
}

void LoopLabeling::visit(Parsing::SwitchStmt& switchStmt)
{
    const std::string breakTemp = breakLabel;
    const std::string switchTemp = switchLabel;
    const Type conditionTypeTemp = conditionType;
    switchStmt.identifier = makeTemporary("switch");
    breakLabel = switchStmt.identifier;
    switchLabel = switchStmt.identifier;
    conditionType = switchStmt.condition->type->kind;
    switchCases[switchStmt.identifier] = std::vector<std::variant<i32, i64, u32, u64>>();
    ASTTraverser::visit(switchStmt);
    switchStmt.cases = switchCases[switchStmt.identifier];
    if (m_default.contains(switchStmt.identifier))
        switchStmt.hasDefault = true;
    breakLabel = breakTemp;
    switchLabel = switchTemp;
    conditionType = conditionTypeTemp;
}
} // Semantics