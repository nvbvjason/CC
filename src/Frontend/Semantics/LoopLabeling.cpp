#include "LoopLabeling.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTDeepCopy.hpp"

#include <cassert>
#include <numeric>
#include <stack>

namespace Semantics {
struct Node {
    Parsing::Initializer* init;
    i64 at;
    Node() = delete;
    Node(Parsing::Initializer* init, const i64 at)
        : init(init), at{at} {}
};

std::vector<Error> LoopLabeling::programValidate(Parsing::Program& program)
{
    ASTTraverser::visit(program);
    return std::move(errors);
}

void LoopLabeling::visit(Parsing::VarDecl& varDecl)
{
    if (varDecl.init && varDecl.init->kind == Parsing::Initializer::Kind::Compound)
        initArray(varDecl);
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

void initArray(Parsing::VarDecl& array)
{
    std::vector<i64> dimensions = getDimensions(array);
    const i64 size = std::accumulate(dimensions.begin(), dimensions.end(), i64{1}, std::multiplies<>{});
    auto arrayInit = array.init.get();
    std::vector<std::unique_ptr<Parsing::Initializer>> staticInitializer;
    i64 at = 0;
    std::stack<Node, std::vector<Node>> stack;
    stack.emplace(arrayInit, 0);
    do {
        auto[init, atCompound] = stack.top();
        stack.pop();
        switch (init->kind) {
            case Parsing::Initializer::Kind::Compound: {
                auto compoundInit = dynCast<Parsing::CompoundInitializer>(init);
                if (compoundInit->initializers.size() <= atCompound) {
                    const i64 dimension = dimensions[stack.size()];
                    staticInitializer.emplace_back(std::make_unique<Parsing::ZeroInitializer>(
                        dimension - atCompound));
                }
                else {
                    stack.emplace(init, atCompound + 1);
                    stack.emplace(compoundInit->initializers[atCompound].get(), atCompound);
                }
                break;
            }
            case Parsing::Initializer::Kind::Single: {
                auto singleInit = dynCast<Parsing::SingleInitializer>(init);
                staticInitializer.emplace_back(std::make_unique<Parsing::SingleInitializer>(
                    Parsing::deepCopy(*singleInit->exp)));
                ++at;
                break;
            }
            default:
                std::abort();
        }
    } while (!stack.empty());
    if (at + 1 < size)
        staticInitializer.emplace_back(std::make_unique<Parsing::ZeroInitializer>(size - at - 1));
    auto newArrayInit = std::make_unique<Parsing::CompoundInitializer>(std::move(staticInitializer));
    array.init = std::move(newArrayInit);
}

std::vector<i64> getDimensions(const Parsing::VarDecl& array)
{
    std::vector<i64> dimensions;
    Parsing::TypeBase* type = array.type.get();
    do {
        const auto arrayType = dynCast<Parsing::ArrayType>(type);
        dimensions.emplace_back(arrayType->size);
        type = arrayType->elementType.get();
    } while (type->kind == Parsing::TypeBase::Kind::Array);
    return dimensions;
}
} // Semantics