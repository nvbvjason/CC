#include "InitCompound.hpp"
#include "ASTUtils.hpp"
#include "DynCast.hpp"
#include "TypeConversion.hpp"

#include <numeric>
#include <stack>

namespace Semantics {

InitCompound::InitCompound(Parsing::VarDecl& varDecl, std::vector<Error>& errors)
    : varDecl(varDecl), errors(errors)
{
    dimensions = getDimensions(varDecl);
}

void InitCompound::initCharacterArray(const Parsing::SingleInitializer& singleInit,
                                      const Parsing::ArrayType& arrayType) const
{
    if (singleInit.expr->kind != Parsing::Expr::Kind::String)
        return;
    const auto stringExpr = dynCast<Parsing::StringExpr>(singleInit.expr.get());
    const i64 diff = arrayType.size - static_cast<i64>(stringExpr->value.size());
    std::vector<std::unique_ptr<Parsing::Initializer>> initializers;
    initStringWithI32(stringExpr->value, initializers);
    if (0 < diff) {
        auto zeroInit = std::make_unique<Parsing::ZeroInitializer>(diff);
        initializers.push_back(std::move(zeroInit));
    }
    varDecl.init = std::make_unique<Parsing::CompoundInitializer>(std::move(initializers));
}

void InitCompound::initArray()
{
    const Type innerArrayType = Parsing::getArrayType(varDecl.type.get());
    if (isStructuredType(innerArrayType)) {
        errors.emplace_back("Structured inner arrays types not supported", varDecl.location);
        return;
    }
    const auto arrayInit = varDecl.init.get();
    auto[staticInitializer, emplacedPositions] =
        flattenWithPositions(innerArrayType, arrayInit);
    staticInitializer = getZeroInits(staticInitializer, emplacedPositions);
    auto newArrayInit = std::make_unique<Parsing::CompoundInitializer>(std::move(staticInitializer));
    varDecl.init = std::move(newArrayInit);
}

void InitCompound::initString(const Type innerArrayType, Parsing::Initializer* const init) const
{
    const auto stringInit = dynCast<Parsing::StringInitializer>(init);
    if (innerArrayType != Type::Pointer && innerArrayType != Type::Char) {
        errors.emplace_back("Wrong type for String init", stringInit->location);
        return;
    }
    std::vector<std::unique_ptr<Parsing::Initializer>> stringInitializer;
    initStringWithI32(stringInit->value, stringInitializer);
}

std::tuple<std::vector<std::unique_ptr<Parsing::Initializer>>, std::vector<std::vector<i64>>>
    InitCompound::flattenWithPositions(
        const Type innerArrayType,
        Parsing::Initializer* arrayInit)
{
    std::vector<std::unique_ptr<Parsing::Initializer>> staticInitializer;
    std::vector<std::vector<i64>> emplacedPositions;
    std::vector<i64> firstPosition;
    stack.emplace(arrayInit, firstPosition);
    while (!stack.empty()) {
        const auto[init, position] = stack.top();
        stack.pop();
        switch (init->kind) {
            case Parsing::Initializer::Kind::Compound: {
                const auto compoundInit = dynCast<Parsing::CompoundInitializer>(init);
                createInitsWithPositionsCompound(position, *compoundInit);
                break;
            }
            case Parsing::Initializer::Kind::Single: {
                const auto singleInit = dynCast<Parsing::SingleInitializer>(init);
                createInitsWithPositionsSingle(
                        innerArrayType,
                        staticInitializer,
                        emplacedPositions,
                        position,
                        *singleInit);
                break;
            }
            case Parsing::Initializer::Kind::String: {
                initString(innerArrayType, init);
                break;
            }
            default:
                std::abort();
        }
    }
    return {std::move(staticInitializer), emplacedPositions};
}

void InitCompound::createInitsWithPositionsCompound(
    const std::vector<i64>& position,
    const Parsing::CompoundInitializer& compoundInit)
{
    for (i64 i = compoundInit.initializers.size() - 1; 0 <= i; --i) {
        std::vector<i64> positionInCompound = position;
        positionInCompound.emplace_back(i);
        stack.emplace(compoundInit.initializers[i].get(), positionInCompound);
    }
}

void InitCompound::createInitsWithPositionsSingle(
    const Type innerArrayType,
    std::vector<std::unique_ptr<Parsing::Initializer>>& staticInitializer,
    std::vector<std::vector<i64>>& emplacedPositions,
    const std::vector<i64>& position,
    Parsing::SingleInitializer& singleInit) const
{
    if (singleInit.expr->kind != Parsing::Expr::Kind::String) {
        auto newInit = emplaceNewSingleInit(innerArrayType, singleInit);
        staticInitializer.emplace_back(std::move(newInit));
        emplacedPositions.emplace_back(position);
        return;
    }
    const auto stringExpr = dynCast<Parsing::StringExpr>(singleInit.expr.get());
    const std::string& stringValue = stringExpr->value;
    if (innerArrayType != Type::Pointer && !isCharacterType(innerArrayType)) {
        errors.emplace_back("Wrong type for String init", stringExpr->location);
        return;
    }
    if (position.size() < dimensions.size() && dimensions[position.size()] < stringExpr->value.size()) {
        errors.emplace_back("Argument is too long", stringExpr->location);
        return;
    }
    if (innerArrayType == Type::Pointer) {
        std::vector<i64> positionInCompound = position;
        emplacedPositions.emplace_back(positionInCompound);
        std::string copy = stringValue;
        auto newStringExpr = std::make_unique<Parsing::StringExpr>(stringExpr->location, std::move(copy));
        auto newSingleInit = std::make_unique<Parsing::SingleInitializer>(std::move(newStringExpr));
        staticInitializer.push_back(std::move(newSingleInit));
        return;
    }
    for (i64 i = 0; i < stringValue.size(); ++i) {
        std::vector<i64> positionInCompound = position;
        positionInCompound.emplace_back(i);
        emplacedPositions.emplace_back(positionInCompound);
        const char ch = stringValue[i];
        auto constExpr = std::make_unique<Parsing::ConstExpr>(
            ch, std::make_unique<Parsing::VarType>(Type::Char));
        auto newSingleInit = std::make_unique<Parsing::SingleInitializer>(std::move(constExpr));
        staticInitializer.push_back(std::move(newSingleInit));
    }
}

std::unique_ptr<Parsing::Initializer> emplaceNewSingleInit(
    const Type innerArrayType, Parsing::SingleInitializer& singleInit)
{
    std::unique_ptr<Parsing::Expr> newExpr = nullptr;
    if (singleInit.expr->kind != Parsing::Expr::Kind::Cast)
        newExpr = convertOrCastToType(singleInit.expr, innerArrayType);
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

std::vector<std::unique_ptr<Parsing::Initializer>> InitCompound::getZeroInits(
    const std::vector<std::unique_ptr<Parsing::Initializer>>& staticInitializer,
    const std::vector<std::vector<i64>>& emplacedPositions)
{
    using InitKind = Parsing::Initializer::Kind;

    std::vector<i64> positionBefore;
    std::vector<std::unique_ptr<Parsing::Initializer>> newInitializers;
    for (size_t i = 0; i < staticInitializer.size(); ++i) {
        const auto init = staticInitializer[i].get();
        const std::vector<i64>& position = emplacedPositions[i];
        if (init->kind != InitKind::Single)
            continue;
        const i64 distance = getDistance(positionBefore, position);
        positionBefore = position;
        if (distance != 0)
            newInitializers.emplace_back(std::make_unique<Parsing::ZeroInitializer>(distance));
        const auto singleInit = dynCast<Parsing::SingleInitializer>(init);
        newInitializers.emplace_back(std::make_unique<Parsing::SingleInitializer>(
            std::move(singleInit->expr)));
    }
    const i64 arrayLastPosition = std::accumulate(dimensions.begin(), dimensions.end(), i64{1}, std::multiplies<>{});
    const i64 positionLast = getPosition(positionBefore);
    const i64 distanceToLast = arrayLastPosition - positionLast - 1;
    if (distanceToLast != 0)
        newInitializers.emplace_back(std::make_unique<Parsing::ZeroInitializer>(distanceToLast));
    return newInitializers;
}

i64 InitCompound::getDistance(const std::vector<i64>& positionBefore, const std::vector<i64>& position) const
{
    const i64 before = getPosition(positionBefore);
    const i64 now = getPosition(position);
    return now - before - 1;
}

i64 InitCompound::getPosition(const std::vector<i64>& position) const
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
    return Parsing::isZeroArithmeticType(*constExpr);
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

void initStringWithI32(const std::string& value,
                       std::vector<std::unique_ptr<Parsing::Initializer>>& stringInitializer)
{
    i64 i = 0;
    for (; i + 4 < value.size(); i += 4) {
        i32 integer = 0;
        integer += value[i];
        integer += value[i + 1] << 8;
        integer += value[i + 2] << 16;
        integer += value[i + 3] << 24;
        auto constExpr = std::make_unique<Parsing::ConstExpr>(
            integer, std::make_unique<Parsing::VarType>(Type::I32));
        auto singleInit = std::make_unique<Parsing::SingleInitializer>(std::move(constExpr));
        stringInitializer.push_back(std::move(singleInit));
    }
    for (; i < value.size(); ++i) {
        auto constExpr = std::make_unique<Parsing::ConstExpr>(
            value[i], std::make_unique<Parsing::VarType>(Type::Char));
        auto singleInit = std::make_unique<Parsing::SingleInitializer>(std::move(constExpr));
        stringInitializer.push_back(std::move(singleInit));
    }
}
} // Semantics