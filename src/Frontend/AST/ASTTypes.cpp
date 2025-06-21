#include "ASTTypes.hpp"

#include <cassert>

namespace Parsing {
std::unique_ptr<TypeBase> deepCopy(const TypeBase& typeBase)
{
    assert(typeBase.kind != Type::Invalid);
    switch (typeBase.kind) {
        case Type::Pointer: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto typePtr = static_cast<const PointerType*>(&typeBase);
            auto referencedCopy = deepCopy(*typePtr->referenced);
            return std::make_unique<PointerType>(std::move(referencedCopy));
        }
        case Type::Function: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto typeFunction = static_cast<const FuncType*>(&typeBase);
            return deepCopy(*typeFunction);
        }
        default:
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto typeVar = static_cast<const VarType*>(&typeBase);
            return deepCopy(*typeVar);
    }
}

std::unique_ptr<TypeBase> deepCopy(const VarType& varType)
{
    return std::make_unique<VarType>(varType.kind);
}

std::unique_ptr<TypeBase> deepCopy(const FuncType& funcType)
{
    std::vector<std::unique_ptr<TypeBase>> args;
    args.reserve(funcType.params.size());
    for (const std::unique_ptr<TypeBase>& param : funcType.params)
        args.push_back(deepCopy(*param));
    std::unique_ptr<TypeBase> rt = deepCopy(*funcType.returnType);
    return std::make_unique<FuncType>(std::move(rt), std::move(args));
}

std::unique_ptr<TypeBase> deepCopy(const PointerType& pointerType)
{
    return std::make_unique<PointerType>(deepCopy(*pointerType.referenced));
}

bool areEquivalent(const TypeBase& left, const TypeBase& right)
{
    assert(left.kind != Type::Invalid);
    assert(right.kind != Type::Invalid);
    if (left.kind != right.kind)
        return false;
    switch (left.kind) {
        case Type::Pointer: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto typePtrLeft = static_cast<const PointerType*>(&left);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto typePtrRight = static_cast<const PointerType*>(&right);
            return areEquivalent(*typePtrLeft->referenced, *typePtrRight->referenced);
        }
        case Type::Function: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto typeFunctionLeft = static_cast<const FuncType*>(&left);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto typeFunctionRight = static_cast<const FuncType*>(&right);
            return areEquivalent(*typeFunctionLeft, *typeFunctionRight);
        }
        default:
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto typeVarRight = static_cast<const VarType*>(&left);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto typeVarLeft = static_cast<const VarType*>(&right);
            return areEquivalent(*typeVarRight, *typeVarLeft);
    }
}

bool areEquivalent(const VarType& left, const VarType& right)
{
    return left.kind == right.kind;
}

bool areEquivalent(const FuncType& left, const FuncType& right)
{
    if (!areEquivalent(*left.returnType, *right.returnType))
        return false;
    if (left.params.size() != right.params.size())
        return false;
    for (std::size_t i = 0; i < left.params.size(); ++i)
        if (!areEquivalent(*left.params[i], *right.params[i]))
            return false;
    return true;
}

bool areEquivalent(const PointerType& left, const PointerType& right)
{
    return areEquivalent(*left.referenced, *right.referenced);
}
} // Parsing