#include "ASTDeepCopy.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTExpr.hpp"

#include <cassert>

namespace Parsing {

std::unique_ptr<TypeBase> deepCopy(const TypeBase& typeBase)
{
    assert(typeBase.type != Type::Invalid);
    switch (typeBase.kind) {
        case TypeBase::Kind::Pointer: {
            const auto typePtr = dynCast<const PointerType>(&typeBase);
            auto referencedCopy = deepCopy(*typePtr->referenced);
            return std::make_unique<PointerType>(std::move(referencedCopy));
        }
        case TypeBase::Kind::Func: {
            const auto typeFunction = dynCast<const FuncType>(&typeBase);
            return deepCopy(*typeFunction);
        }
        case TypeBase::Kind::Array: {
            const auto typeArray = dynCast<const ArrayType>(&typeBase);
            return deepCopy(*typeArray);
        }
        case TypeBase::Kind::Var: {
            const auto typeVar = dynCast<const VarType>(&typeBase);
            return deepCopy(*typeVar);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<TypeBase> deepCopy(const ArrayType& arrayType)
{
    auto typeBase = deepCopy(*arrayType.elementType);
    return std::make_unique<ArrayType>(std::move(typeBase), arrayType.size);
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

bool areEquivalentTypes(const TypeBase& left, const TypeBase& right)
{
    assert(left.type != Type::Invalid);
    assert(right.type != Type::Invalid);
    if (left.type != right.type)
        return false;
    switch (left.type) {
        case Type::Pointer: {
            const auto typePtrLeft = dynCast<const PointerType>(&left);
            const auto typePtrRight = dynCast<const PointerType>(&right);
            return areEquivalentTypes(*typePtrLeft->referenced, *typePtrRight->referenced);
        }
        case Type::Function: {
            const auto typeFunctionLeft = dynCast<const FuncType>(&left);
            const auto typeFunctionRight = dynCast<const FuncType>(&right);
            return areEquivalentTypes(*typeFunctionLeft, *typeFunctionRight);
        }
        case Type::Array: {
            const auto typeFunctionLeft = dynCast<const ArrayType>(&left);
            const auto typeFunctionRight = dynCast<const ArrayType>(&right);
            return areEquivalentTypes(*typeFunctionLeft, *typeFunctionRight);
        }
        default: {
            const auto typeVarRight = dynCast<const VarType>(&left);
            const auto typeVarLeft = dynCast<const VarType>(&right);
            return areEquivalentTypes(*typeVarRight, *typeVarLeft);
        }
    }
}

bool areEquivalentTypes(const VarType& left, const VarType& right)
{
    return left.type == right.type;
}

bool areEquivalentTypes(const FuncType& left, const FuncType& right)
{
    if (!areEquivalentTypes(*left.returnType, *right.returnType))
        return false;
    if (left.params.size() != right.params.size())
        return false;
    for (std::size_t i = 0; i < left.params.size(); ++i)
        if (!areEquivalentTypes(*left.params[i], *right.params[i]))
            return false;
    return true;
}

bool areEquivalentTypes(const PointerType& left, const PointerType& right)
{
    return areEquivalentTypes(*left.referenced, *right.referenced);
}

bool areEquivalentTypes(const ArrayType& left, const ArrayType& right)
{
    if (left.size != right.size)
        return false;
    return areEquivalentTypes(*left.elementType, *right.elementType);
}

std::unique_ptr<TypeBase> convertArrayFirstDimToPtr(const TypeBase& typeBase)
{
    const TypeBase* ptrBase = &typeBase;
    if (typeBase.type == Type::Array) {
        const auto arrayType = dynCast<const ArrayType>(&typeBase);
        return std::make_unique<PointerType>(deepCopy(*arrayType->elementType));
    }
    return deepCopy(*ptrBase);
}

bool areEquivalentArrayConversion(const TypeBase& left, const TypeBase& right)
{
    const std::unique_ptr<TypeBase> leftPtr = convertArrayFirstDimToPtr(left);
    const std::unique_ptr<TypeBase> rightPtr = convertArrayFirstDimToPtr(right);
    return areEquivalentTypes(*leftPtr, *rightPtr);
}

std::unique_ptr<Expr> deepCopy(const Expr& expr)
{
    switch (expr.kind) {
        case Expr::Kind::Constant: {
            const auto constExpr = dynCast<const ConstExpr>(&expr);
            return deepCopy(*constExpr);
        }
        case Expr::Kind::String: {
            const auto stringExpr = dynCast<const StringExpr>(&expr);
            return deepCopy(*stringExpr);
        }
        case Expr::Kind::Var: {
            const auto varExpr = dynCast<const VarExpr>(&expr);
            return deepCopy(*varExpr);
        }
        case Expr::Kind::Cast: {
            const auto castExpr = dynCast<const CastExpr>(&expr);
            return deepCopy(*castExpr);
        }
        case Expr::Kind::Unary: {
            const auto unaryExpr = dynCast<const UnaryExpr>(&expr);
            return deepCopy(*unaryExpr);
        }
        case Expr::Kind::Binary: {
            const auto binaryExpr = dynCast<const BinaryExpr>(&expr);
            return deepCopy(*binaryExpr);
        }
        case Expr::Kind::Assignment: {
            const auto assignmentExpr = dynCast<const AssignmentExpr>(&expr);
            return deepCopy(*assignmentExpr);
        }
        case Expr::Kind::Ternary: {
            const auto ternaryExpr = dynCast<const TernaryExpr>(&expr);
            return deepCopy(*ternaryExpr);
        }
        case Expr::Kind::FunctionCall: {
            const auto funcCallExpr = dynCast<const FuncCallExpr>(&expr);
            return deepCopy(*funcCallExpr);
        }
        case Expr::Kind::Dereference: {
            const auto dereferenceExpr = dynCast<const DereferenceExpr>(&expr);
            return deepCopy(*dereferenceExpr);
        }
        case Expr::Kind::AddrOf: {
            const auto addrOffExpr = dynCast<const AddrOffExpr>(&expr);
            return deepCopy(*addrOffExpr);
        }
        case Expr::Kind::Subscript: {
            const auto subscriptExpr = dynCast<const SubscriptExpr>(&expr);
            return deepCopy(*subscriptExpr);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<Expr> deepCopy(const ConstExpr& expr)
{
    switch (expr.type->type) {
        case Type::I8:
            return std::make_unique<ConstExpr>(expr.location, expr.getValue<i8>(), std::make_unique<VarType>(Type::I8));
        case Type::U8:
            return std::make_unique<ConstExpr>(expr.location, expr.getValue<u8>(), std::make_unique<VarType>(Type::U8));
        case Type::I32:
            return std::make_unique<ConstExpr>(expr.location, expr.getValue<i32>(), std::make_unique<VarType>(Type::I32));
        case Type::U32:
            return std::make_unique<ConstExpr>(expr.location, expr.getValue<u32>(), std::make_unique<VarType>(Type::U32));
        case Type::I64:
            return std::make_unique<ConstExpr>(expr.location, expr.getValue<i64>(), std::make_unique<VarType>(Type::I64));
        case Type::U64:
            return std::make_unique<ConstExpr>(expr.location, expr.getValue<u64>(), std::make_unique<VarType>(Type::U64));
        case Type::Double:
            return std::make_unique<ConstExpr>(expr.location, expr.getValue<double>(), std::make_unique<VarType>(Type::Double));
        case Type::Char:
            return std::make_unique<ConstExpr>(expr.location, expr.getValue<char>(), std::make_unique<VarType>(Type::Char));
        default:
            std::abort();
    }
}

std::unique_ptr<Expr> deepCopy(const StringExpr& expr)
{
    std::string value = expr.value;
    auto result = std::make_unique<StringExpr>(expr.location, std::move(value));
    if (expr.type)
        result->type = deepCopy(*expr.type);
    return result;
}

std::unique_ptr<Expr> deepCopy(const VarExpr& expr)
{
    auto result = std::make_unique<VarExpr>(expr.location, expr.name);
    if (expr.type)
        result->type = deepCopy(*expr.type);
    result->referingTo = expr.referingTo;
    return result;
}

std::unique_ptr<Expr> deepCopy(const CastExpr& expr)
{
    return std::make_unique<CastExpr>(
        expr.location, deepCopy(*expr.type), deepCopy(*expr.innerExpr));
}

std::unique_ptr<Expr> deepCopy(const UnaryExpr& expr)
{
    auto result = std::make_unique<UnaryExpr>(expr.location, expr.op, deepCopy(*expr.innerExpr));
    if (expr.type)
        result->type = deepCopy(*expr.type);
    return result;
}

std::unique_ptr<Expr> deepCopy(const BinaryExpr& expr)
{
    auto result = std::make_unique<BinaryExpr>(
        expr.location, expr.op, deepCopy(*expr.lhs), deepCopy(*expr.rhs));
    if (expr.type)
        result->type = deepCopy(*expr.type);
    return result;
}

std::unique_ptr<Expr> deepCopy(const AssignmentExpr& expr)
{
    auto result = std::make_unique<AssignmentExpr>(
        expr.location, expr.op, deepCopy(*expr.lhs), deepCopy(*expr.rhs));
    if (expr.type)
        result->type = deepCopy(*expr.type);
    return result;
}

std::unique_ptr<Expr> deepCopy(const TernaryExpr& expr)
{
    auto result = std::make_unique<TernaryExpr>(
        expr.location,
        deepCopy(*expr.condition), deepCopy(*expr.trueExpr), deepCopy(*expr.falseExpr));
    if (expr.type)
        result->type = deepCopy(*expr.type);
    return result;
}

std::unique_ptr<Expr> deepCopy(const FuncCallExpr& expr)
{
    std::vector<std::unique_ptr<Expr>> args;
    for (const auto& arg : expr.args)
        args.push_back(deepCopy(*arg));
    auto result = std::make_unique<FuncCallExpr>(expr.location, expr.name, std::move(args));
    if (expr.type)
        result->type = deepCopy(*expr.type);
    return result;
}

std::unique_ptr<Expr> deepCopy(const DereferenceExpr& expr)
{
    auto result = std::make_unique<DereferenceExpr>(expr.location, deepCopy(*expr.reference));
    if (expr.type)
        result->type = deepCopy(*expr.type);
    return result;
}

std::unique_ptr<Expr> deepCopy(const AddrOffExpr& expr)
{
    auto result = std::make_unique<AddrOffExpr>(expr.location, deepCopy(*expr.reference));
    if (expr.type)
        result->type = deepCopy(*expr.type);
    return result;
}

std::unique_ptr<Expr> deepCopy(const SubscriptExpr& expr)
{
    auto result = std::make_unique<SubscriptExpr>(
        expr.location, deepCopy(*expr.referencing), deepCopy(*expr.index));
    if (expr.type)
        result->type = deepCopy(*expr.type);
    return result;
}
} // Parsing