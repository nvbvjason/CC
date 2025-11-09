#include "LoopLabeling.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTDeepCopy.hpp"

#include <cassert>
#include <numeric>
#include <stack>

namespace Semantics {
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

struct Node {
    Parsing::Initializer* init;
    const i64 at;
    const i32 depth;
    Node() = delete;
    Node(Parsing::Initializer* init, const i64 at, const i32 depth)
        : init(init), at(at), depth (depth) {}
};

void initArray(Parsing::VarDecl& array)
{
    std::vector<i64> dimensions = getDimensions(array);
    const i64 size = std::accumulate(dimensions.begin(), dimensions.end(), i64{1}, std::multiplies<>{});
    auto arrayInit = array.init.get();
    std::vector<std::unique_ptr<Parsing::Initializer>> staticInitializer;
    i64 atInFlattened = 0;
    std::stack<Node, std::vector<Node>> stack;
    stack.emplace(arrayInit, 0, 0);
    do {
        const auto[init, atInCompound, depth] = stack.top();
        stack.pop();
        switch (init->kind) {
            case Parsing::Initializer::Kind::Compound: {
                const auto compoundInit = dynCast<Parsing::CompoundInitializer>(init);
                if (compoundInit->initializers.size() <= atInCompound) {
                    const i64 dimension = dimensions[depth];
                    if (atInCompound < dimension) {
                        staticInitializer.emplace_back(std::make_unique<Parsing::ZeroInitializer>(
                            dimension - atInCompound));
                        atInFlattened += dimension - atInCompound;
                    }
                }
                else {
                    stack.emplace(init, atInCompound + 1, depth);
                    stack.emplace(compoundInit->initializers[atInCompound].get(), 0, depth + 1);
                }
                break;
            }
            case Parsing::Initializer::Kind::Single: {
                const auto singleInit = dynCast<Parsing::SingleInitializer>(init);
                staticInitializer.emplace_back(std::make_unique<Parsing::SingleInitializer>(
                    Parsing::deepCopy(*singleInit->exp)));
                ++atInFlattened;
                break;
            }
            default:
                std::abort();
        }
    } while (!stack.empty());
    if (atInFlattened < size)
        staticInitializer.emplace_back(std::make_unique<Parsing::ZeroInitializer>(size - atInFlattened));
    staticInitializer = combineZeroInits(staticInitializer);
    auto newArrayInit = std::make_unique<Parsing::CompoundInitializer>(std::move(staticInitializer));
    array.init = std::move(newArrayInit);
}

std::vector<std::unique_ptr<Parsing::Initializer>> combineZeroInits(
    std::vector<std::unique_ptr<Parsing::Initializer>>& staticInitializer)
{
    std::vector<std::unique_ptr<Parsing::Initializer>> newInitializers;
    for (size_t i = 0; i < staticInitializer.size(); ++i) {
        if (staticInitializer[i]->kind == Parsing::Initializer::Kind::Single) {
            const auto sinleInit = dynCast<Parsing::SingleInitializer>(staticInitializer[i].get());
            newInitializers.emplace_back(std::make_unique<Parsing::SingleInitializer>(
                Parsing::deepCopy(*sinleInit->exp)));
            continue;
        }
        i64 length = 0;
        while (i < staticInitializer.size() && staticInitializer[i]->kind == Parsing::Initializer::Kind::Zero) {
            const auto zeroInit = dynCast<Parsing::ZeroInitializer>(staticInitializer[i].get());
            length += zeroInit->size;
            ++i;
        }
        --i;
        newInitializers.emplace_back(std::make_unique<Parsing::ZeroInitializer>(length));
    }
    return newInitializers;
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
    dimensions.emplace_back(1);
    return dimensions;
}
} // Semantics