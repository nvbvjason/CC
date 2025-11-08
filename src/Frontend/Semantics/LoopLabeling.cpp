#include "LoopLabeling.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"

#include <cassert>

namespace Semantics {
std::tuple<std::vector<Error>, std::vector<Parsing::VarDecl*>> LoopLabeling::programValidate(
    Parsing::Program& program)
{
    ASTTraverser::visit(program);
    return {std::move(errors), std::move(varDecls)};
}

void LoopLabeling::visit(Parsing::VarDecl& varDecl)
{
    if (varDecl.init && varDecl.init->kind == Parsing::Initializer::Kind::Compound)
        varDecls.emplace_back(&varDecl);
    ASTTraverser::visit(varDecl);
}

void LoopLabeling::visit(Parsing::CaseStmt& caseStmt)
{
    caseStmt.identifier = switchLabel;
    if (isOutsideSwitchStmt(caseStmt)) {
        errors.emplace_back("Case outside of switch ", caseStmt.location);
        return;
    }
    if (isNonConstantInSwitchCase(caseStmt)) {
        errors.emplace_back("Non constant in in switch ", caseStmt.location);
        return;
    }
    const auto constantExpr = dynCast<const Parsing::ConstExpr>(caseStmt.condition.get());
    if (constantExpr->type->type == Type::Double || constantExpr->type->type == Type::Pointer) {
        if (constantExpr->type->type == Type::Double)
            errors.emplace_back("Double in switch case ", caseStmt.location);
        if (constantExpr->type->type == Type::Pointer)
            errors.emplace_back("Pointer in switch case ", caseStmt.location);
        return;
    }
    switch (conditionType) {
        case Type::I32: {
            processSwitchCase<i32>(constantExpr, switchCases, switchLabel, caseStmt, errors);
            break;
        }
        case Type::I64: {
            processSwitchCase<i64>(constantExpr, switchCases, switchLabel, caseStmt, errors);
            break;
        }
        case Type::U32: {
            processSwitchCase<u32>(constantExpr, switchCases, switchLabel, caseStmt, errors);
            break;
        }
        case Type::U64: {
            processSwitchCase<u64>(constantExpr, switchCases, switchLabel, caseStmt, errors);
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
        errors.emplace_back("Duplicate default statement ", defaultStmt.location);
    m_default.insert(defaultStmt.identifier);
    if (defaultStmt.identifier.empty())
        errors.emplace_back("Default must be in switch statement ", defaultStmt.location);
    ASTTraverser::visit(defaultStmt);
}

void LoopLabeling::visit(Parsing::ContinueStmt& continueStmt)
{
    continueStmt.identifier = continueLabel;
    if (continueStmt.identifier.empty())
        errors.emplace_back("Continue statement has nothing to refer to ", continueStmt.location);
}

void LoopLabeling::visit(Parsing::BreakStmt& breakStmt)
{
    breakStmt.identifier = breakLabel;
    if (breakStmt.identifier.empty())
        errors.emplace_back("Break statement has nothing to refer to ", breakStmt.location);
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
    conditionType = switchStmt.condition->type->type;
    if (conditionType == Type::Double || conditionType == Type::Pointer) {
        if (conditionType == Type::Double)
            errors.emplace_back("Double as switch condition ", switchStmt.location);
        if (conditionType == Type::Pointer)
            errors.emplace_back("Pointer as switch condition ", switchStmt.location);
        return;
    }
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