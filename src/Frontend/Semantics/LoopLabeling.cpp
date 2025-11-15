#include "LoopLabeling.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTDeepCopy.hpp"
#include "AstToIrOperators.hpp"
#include "Utils.hpp"

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
    if (!varDecl.init)
        return;
    if (varDecl.type->type == Type::Array) {
        if (varDecl.init->kind == Parsing::Initializer::Kind::Compound) {
            initArray(varDecl);
            return;
        }
        const auto arrayType = dynCast<Parsing::ArrayType>(varDecl.type.get());
        auto singleInitializer = dynCast<Parsing::SingleInitializer>(varDecl.init.get());
        if (isCharacterType(arrayType->elementType->type))
            initCharacterArray(varDecl, *singleInitializer, *arrayType);
    }
    // if (varDecl.type->type == Type::Pointer) {
    //     const auto ptrType = dynCast<Parsing::PointerType>(varDecl.type.get());
    //     if (!isCharacterType(ptrType->referenced->type))
    //         return;
    //     if (varDecl.init->kind == Parsing::Initializer::Kind::String) {
    //         const auto stringInit = dynCast<Parsing::StringInitializer>(varDecl.init.get());
    //
    //     }
    // }
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
        case Type::I8: {
            processSwitchCase<i32>(constantExpr, switchCases, switchLabel, caseStmt, errors);
            break;
        }
        case Type::U8: {
            processSwitchCase<u32>(constantExpr, switchCases, switchLabel, caseStmt, errors);
            break;
        }
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
        case Type::Char: {
            processSwitchCase<i32>(constantExpr, switchCases, switchLabel, caseStmt, errors);
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
    switchCases[switchStmt.identifier] = std::vector<std::variant<i8, u8, i32, i64, u32, u64>>();
    ASTTraverser::visit(switchStmt);
    switchStmt.cases = switchCases[switchStmt.identifier];
    if (m_default.contains(switchStmt.identifier))
        switchStmt.hasDefault = true;
    breakLabel = breakTemp;
    switchLabel = switchTemp;
    conditionType = conditionTypeTemp;
}

void initCharacterArray(Parsing::VarDecl& varDecl,
                        Parsing::SingleInitializer& singleInit,
                        const Parsing::ArrayType& arrayType)
{
    if (singleInit.expr->kind == Parsing::Expr::Kind::AddrOf) {
        const auto addrOf = dynCast<Parsing::AddrOffExpr>(singleInit.expr.get());
        if (addrOf->reference->kind != Parsing::Expr::Kind::String)
            return;
        auto stringExpr = dynCast<Parsing::StringExpr>(addrOf->reference.get());
        const i64 diff = arrayType.size - static_cast<i64>(stringExpr->value.size());
        auto initString = std::make_unique<Parsing::StringInitializer>(
            std::move(stringExpr->value), diff > 0);
        if (1 < diff) {
            std::vector<std::unique_ptr<Parsing::Initializer>> initializers;
            initializers.push_back(std::move(initString));
            auto zeroInit = std::make_unique<Parsing::ZeroInitializer>(diff - 1);
            initializers.push_back(std::move(zeroInit));
            auto compound = std::make_unique<Parsing::CompoundInitializer>(std::move(initializers));
            varDecl.init = std::move(compound);
        }
        else
            varDecl.init = std::move(initString);
    }
}

struct Node {
    Parsing::Initializer* init;
    const std::vector<i64> position;
    Node() = delete;
    Node(Parsing::Initializer* init, const std::vector<i64>& position)
        : init(init), position(position) {}
};

void initArray(Parsing::VarDecl& array)
{
    const Type innerArrayType = Ir::getArrayType(array.type.get());
    const auto arrayInit = array.init.get();
    auto[staticInitializer, emplacedPositions] = getSingleInitAndPositions(innerArrayType, arrayInit);
    const std::vector<i64> dimensions = getDimensions(array);
    staticInitializer = getZeroInits(staticInitializer, dimensions, emplacedPositions);
    auto newArrayInit = std::make_unique<Parsing::CompoundInitializer>(std::move(staticInitializer));
    array.init = std::move(newArrayInit);
}

std::tuple<std::vector<std::unique_ptr<Parsing::Initializer>>, std::vector<std::vector<i64>>>
    getSingleInitAndPositions(const Type innerArrayType, Parsing::Initializer* arrayInit)
{
    std::vector<std::unique_ptr<Parsing::Initializer>> staticInitializer;
    std::vector<std::vector<i64>> emplacedPositions;
    std::stack<Node, std::vector<Node>> stack;
    std::vector<i64> firstPosition;
    stack.emplace(arrayInit, firstPosition);
    while (!stack.empty()) {
        const auto[init, position] = stack.top();
        stack.pop();
        switch (init->kind) {
            case Parsing::Initializer::Kind::Compound: {
                const auto compoundInit = dynCast<Parsing::CompoundInitializer>(init);
                for (i64 i = compoundInit->initializers.size() - 1; 0 <= i; --i) {
                    std::vector<i64> positionInCompound = position;
                    positionInCompound.emplace_back(i);
                    stack.emplace(compoundInit->initializers[i].get(), positionInCompound);
                }
                break;
            }
            case Parsing::Initializer::Kind::Single: {
                const auto singleInit = dynCast<Parsing::SingleInitializer>(init);
                auto newInit = emplaceNewSingleInit(innerArrayType, *singleInit);
                staticInitializer.emplace_back(std::move(newInit));
                emplacedPositions.emplace_back(position);
                break;
            }
            default:
                std::abort();
        }
    }
    return {std::move(staticInitializer), emplacedPositions};
}

std::unique_ptr<Parsing::Initializer> emplaceNewSingleInit(
    const Type innerArrayType, Parsing::SingleInitializer& singleInit)
{
    std::unique_ptr<Parsing::Expr> newExpr = nullptr;
    if (singleInit.expr->kind != Parsing::Expr::Kind::Cast)
        newExpr = convertOrCastToType(*singleInit.expr, innerArrayType);
    else {
        const auto castExpr = dynCast<Parsing::CastExpr>(singleInit.expr.get());
        if (castExpr->innerExpr->kind == Parsing::Expr::Kind::Constant) {
            if (isArithmetic(castExpr->type->type))
                newExpr = convertToArithmeticType(*castExpr->innerExpr, innerArrayType);
            if (castExpr->type->type == Type::Pointer && canConvertToNullPtr(*castExpr->innerExpr)) {
                constexpr i64 one = 1;
                return std::make_unique<Parsing::ZeroInitializer>(one);
            }
        }
    }
    if (newExpr)
        return std::make_unique<Parsing::SingleInitializer>(std::move(newExpr));
    return std::make_unique<Parsing::SingleInitializer>(std::move(singleInit.expr));
}

std::vector<std::unique_ptr<Parsing::Initializer>> getZeroInits(
    const std::vector<std::unique_ptr<Parsing::Initializer>>& staticInitializer,
    const std::vector<i64>& dimensions,
    const std::vector<std::vector<i64>>& emplacedPositions)
{
    using InitKind = Parsing::Initializer::Kind;

    std::vector<i64> positionBefore;
    std::vector<std::unique_ptr<Parsing::Initializer>> newInitializers;
    for (size_t i = 0; i < staticInitializer.size(); ++i) {
        auto init = staticInitializer[i].get();
        const std::vector<i64>& position = emplacedPositions[i];
        if (init->kind != InitKind::Single)
            continue;
        const i64 distance = getDistance(positionBefore, position, dimensions);
        positionBefore = position;
        if (distance != 0)
            newInitializers.emplace_back(std::make_unique<Parsing::ZeroInitializer>(distance));
        const auto sinleInit = dynCast<Parsing::SingleInitializer>(init);
        newInitializers.emplace_back(std::make_unique<Parsing::SingleInitializer>(
            Parsing::deepCopy(*sinleInit->expr)));
    }
    const i64 arrayLastPosition = std::accumulate(dimensions.begin(), dimensions.end(), i64{1}, std::multiplies<>{});
    const i64 positionLast = getPosition(positionBefore, dimensions);
    const i64 distanceToLast = arrayLastPosition - positionLast - 1;
    if (distanceToLast != 0)
        newInitializers.emplace_back(std::make_unique<Parsing::ZeroInitializer>(distanceToLast));
    return newInitializers;
}

i64 getDistance(const std::vector<i64>& positionBefore,
                const std::vector<i64>& position,
                const std::vector<i64>& dimensions)
{
    const i64 before = getPosition(positionBefore, dimensions);
    const i64 now = getPosition(position, dimensions);
    return now - before - 1;
}

i64 getPosition(const std::vector<i64>& position, const std::vector<i64>& dimensions)
{
    if (position.empty())
        return -1;
    i64 result = 0;
    i64 dimension = 1;
    for (i64 i = position.size() - 1; 0 <= i; --i) {
        const i64 localPosition = position[i];
        result += dimension * localPosition;
        dimension *= dimensions[i];
    }
    return result;
}

bool isZeroSingleInit(const Parsing::Initializer& init)
{
    if (init.kind != Parsing::Initializer::Kind::Single)
        return false;
    const auto singleInit = dynCast<const Parsing::SingleInitializer>(&init);
    if (singleInit->expr->kind != Parsing::Expr::Kind::Constant)
        return false;
    const auto constExpr = dynCast<const Parsing::ConstExpr>(singleInit->expr.get());
    switch (constExpr->type->type) {
        case Type::I32: return 0 == constExpr->getValue<i32>();
        case Type::I64: return 0 == constExpr->getValue<i64>();
        case Type::U32: return 0 == constExpr->getValue<u32>();
        case Type::U64: return 0 == constExpr->getValue<u64>();
        default:
            return false;
    }
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