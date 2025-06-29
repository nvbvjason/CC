#include "ASTTypes.hpp"
#include "DynCast.hpp"

#include <cassert>

namespace Parsing {
std::unique_ptr<TypeBase> deepCopy(const TypeBase& typeBase)
{
    assert(typeBase.type != Type::Invalid);
    switch (typeBase.type) {
        case Type::Pointer: {
            const auto typePtr = dyn_cast<const PointerType>(&typeBase);
            std::unique_ptr<TypeBase> referencedCopy = deepCopy(*typePtr->referenced);
            return std::make_unique<PointerType>(std::move(referencedCopy));
        }
        case Type::Function: {
            const auto typeFunction = dyn_cast<const FuncType>(&typeBase);
            return deepCopy(*typeFunction);
        }
        default:
            const auto typeVar = dyn_cast<const VarType>(&typeBase);
            return deepCopy(*typeVar);
    }
}

std::unique_ptr<TypeBase> deepCopy(const VarType& varType)
{
    return std::make_unique<VarType>(varType.type);
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
    assert(left.type != Type::Invalid);
    assert(right.type != Type::Invalid);
    if (left.type != right.type)
        return false;
    switch (left.type) {
        case Type::Pointer: {
            const auto typePtrLeft = dyn_cast<const PointerType>(&left);
            const auto typePtrRight = dyn_cast<const PointerType>(&right);
            return areEquivalent(*typePtrLeft->referenced, *typePtrRight->referenced);
        }
        case Type::Function: {
            const auto typeFunctionLeft = dyn_cast<const FuncType>(&left);
            const auto typeFunctionRight = dyn_cast<const FuncType>(&right);
            return areEquivalent(*typeFunctionLeft, *typeFunctionRight);
        }
        default:
            const auto typeVarRight = dyn_cast<const VarType>(&left);
            const auto typeVarLeft = dyn_cast<const VarType>(&right);
            return areEquivalent(*typeVarRight, *typeVarLeft);
    }
}

bool areEquivalent(const VarType& left, const VarType& right)
{
    return left.type == right.type;
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