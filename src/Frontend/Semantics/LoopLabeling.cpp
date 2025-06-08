#include "LoopLabeling.hpp"

#include <assert.h>

#include "ASTParser.hpp"
#include "ASTTypes.hpp"

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
            i32 value = 0;
            if (constantExpr->type->kind == Type::I32)
                value = std::get<i32>(constantExpr->value);
            if (constantExpr->type->kind == Type::I64)
                value = static_cast<i64>(std::get<i64>(constantExpr->value));
            auto it = switchCases.find(switchLabel);
            for (const std::variant<i32, i64>& v : it->second) {
                if (value == std::get<i32>(v)) {
                    valid = false;
                    return;
                }
            }
            it->second.emplace_back(value);
            caseStmt.identifier += std::to_string(value);
            break;
        }
        case Type::I64: {
            i64 value = 0;
            if (constantExpr->type->kind == Type::I32)
                value = static_cast<i64>(std::get<i32>(constantExpr->value));
            if (constantExpr->type->kind == Type::I64)
                value = std::get<i64>(constantExpr->value);
            auto it = switchCases.find(switchLabel);
            for (const std::variant<i32, i64>& v : it->second) {
                if (value == std::get<i64>(v)) {
                    valid = false;
                    return;
                }
            }
            it->second.emplace_back(value);
            caseStmt.identifier += std::to_string(value);
            break;
        }
        default:
            assert("Should never be reached");
            std::unreachable();
    }
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
    switchCases[switchStmt.identifier] = std::vector<std::variant<i32, i64>>();
    ASTTraverser::visit(switchStmt);
    switchStmt.cases = switchCases[switchStmt.identifier];
    if (m_default.contains(switchStmt.identifier))
        switchStmt.hasDefault = true;
    breakLabel = breakTemp;
    switchLabel = switchTemp;
    conditionType = conditionTypeTemp;
}
} // Semantics