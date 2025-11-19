#include "Labeling.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTUtils.hpp"
#include "AstToIrOperators.hpp"
#include "InitArray.hpp"

#include <cassert>
#include <numeric>
#include <ranges>

namespace Semantics {
std::vector<Error> Labeling::programValidate(Parsing::Program& program)
{
    ASTTraverser::visit(program);
    return std::move(errors);
}

void Labeling::visit(Parsing::FuncDecl& funDecl)
{
    if (funDecl.body == nullptr)
        return;
    m_labels.clear();
    m_goto.clear();
    m_funName = funDecl.name;
    ASTTraverser::visit(funDecl);
    for (const std::vector<i64>& locations: m_labels | std::views::values)
        if (1 < locations.size())
            for (const auto& location: locations)
                emplaceError("Duplicate labels at ", location);
    for (auto& gotoStmt : m_goto)
        if (!m_labels.contains(gotoStmt->identifier))
            emplaceError("Did not find goto label ", gotoStmt->location);
}

void Labeling::visit(Parsing::VarDecl& varDecl)
{
    if (!varDecl.init)
        return;
    if (varDecl.type->type == Type::Array) {
        if (varDecl.init->kind == Parsing::Initializer::Kind::Compound) {
            InitArray initArray(varDecl, errors);
            initArray.initArray();
            return;
        }
        const auto arrayType = dynCast<Parsing::ArrayType>(varDecl.type.get());
        auto singleInitializer = dynCast<Parsing::SingleInitializer>(varDecl.init.get());
        if (isCharacterType(arrayType->elementType->type)) {
            InitArray initArray(varDecl, errors);
            initArray.initCharacterArray(*singleInitializer, *arrayType);
        }
    }
}

void Labeling::visit(Parsing::CaseStmt& caseStmt)
{
    caseStmt.identifier = switchLabel;
    if (isOutsideSwitchStmt(caseStmt)) {
        emplaceError("Case outside of switch ", caseStmt.location);
        return;
    }
    if (isNonConstantInSwitchCase(caseStmt)) {
        emplaceError("Non constant in in switch ", caseStmt.location);
        return;
    }
    const auto constantExpr = dynCast<const Parsing::ConstExpr>(caseStmt.condition.get());
    if (constantExpr->type->type == Type::Double || constantExpr->type->type == Type::Pointer) {
        if (constantExpr->type->type == Type::Double)
            emplaceError("Double in switch case ", caseStmt.location);
        if (constantExpr->type->type == Type::Pointer)
            emplaceError("Pointer in switch case ", caseStmt.location);
        return;
    }
    switch (conditionType) {
        case Type::I32: {
            processSwitchCase<i32>(constantExpr, switchCases, switchLabel, caseStmt, errors);
            break;
        }
        case Type::U32: {
            processSwitchCase<u32>(constantExpr, switchCases, switchLabel, caseStmt, errors);
            break;
        }
        case Type::I64: {
            processSwitchCase<i64>(constantExpr, switchCases, switchLabel, caseStmt, errors);
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

void Labeling::visit(Parsing::DefaultStmt& defaultStmt)
{
    defaultStmt.identifier = switchLabel;
    if (defaultCase.contains(defaultStmt.identifier))
        emplaceError("Duplicate default statement ", defaultStmt.location);
    defaultCase.insert(defaultStmt.identifier);
    if (defaultStmt.identifier.empty())
        emplaceError("Default must be in switch statement ", defaultStmt.location);
    ASTTraverser::visit(defaultStmt);
}

void Labeling::visit(Parsing::ContinueStmt& continueStmt)
{
    continueStmt.identifier = continueLabel;
    if (continueStmt.identifier.empty())
        emplaceError("Continue statement has nothing to refer to ", continueStmt.location);
}

void Labeling::visit(Parsing::BreakStmt& breakStmt)
{
    breakStmt.identifier = breakLabel;
    if (breakStmt.identifier.empty())
        emplaceError("Break statement has nothing to refer to ", breakStmt.location);
}

void Labeling::visit(Parsing::WhileStmt& whileStmt)
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

void Labeling::visit(Parsing::DoWhileStmt& doWhileStmt)
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

void Labeling::visit(Parsing::ForStmt& forStmt)
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

void Labeling::visit(Parsing::SwitchStmt& switchStmt)
{
    const std::string breakTemp = breakLabel;
    const std::string switchTemp = switchLabel;
    const Type conditionTypeTemp = conditionType;
    switchStmt.identifier = makeTemporary("switch");
    breakLabel = switchStmt.identifier;
    switchLabel = switchStmt.identifier;
    conditionType = switchStmt.condition->type->type;
    if (isCharacterType(conditionType)) {
        conditionType = Type::I32;
        switchStmt.condition = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(Type::I32), std::move(switchStmt.condition));
    }
    if (conditionType == Type::Double) {
        emplaceError("Double as switch condition ", switchStmt.location);
        return;
    }
    if (conditionType == Type::Pointer) {
        emplaceError("Pointer as switch condition ", switchStmt.location);
        return;
    }
    if (isStructuredType(conditionType)) {
        emplaceError("structured type as switch condition ", switchStmt.location);
        return;
    }
    switchCases[switchStmt.identifier] = std::vector<std::variant<i32, i64, u32, u64>>();
    ASTTraverser::visit(switchStmt);
    switchStmt.cases = switchCases[switchStmt.identifier];
    if (defaultCase.contains(switchStmt.identifier))
        switchStmt.hasDefault = true;
    breakLabel = breakTemp;
    switchLabel = switchTemp;
    conditionType = conditionTypeTemp;
}

void Labeling::visit(Parsing::GotoStmt& gotoStmt)
{
    gotoStmt.identifier += '.' + m_funName;
    m_goto.insert(&gotoStmt);
    ASTTraverser::visit(gotoStmt);
}

void Labeling::visit(Parsing::LabelStmt& labelStmt)
{
    labelStmt.identifier += '.' + m_funName;
    m_labels[labelStmt.identifier].emplace_back(labelStmt.location);
    ASTTraverser::visit(labelStmt);
}
} // Semantics