#include "ASTUtils.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTExpr.hpp"

#include <cassert>

#include "Types/TypeConversion.hpp"

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
        case TypeBase::Kind::Struct: {
            const auto typeStruct = dynCast<const StructType>(&typeBase);
            return deepCopy(*typeStruct);
        }
        case TypeBase::Kind::Union: {
            const auto typeUnion = dynCast<const UnionType>(&typeBase);
            return deepCopy(*typeUnion);
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

std::unique_ptr<TypeBase> deepCopy(const StructType& structType)
{
    return std::make_unique<StructType>(structType.identifier, structType.location);
}

std::unique_ptr<TypeBase> deepCopy(const UnionType& unionType)
{
    return std::make_unique<UnionType>(unionType.identifier, unionType.location);
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
        case Type::Struct: {
            const auto typeVarRight = dynCast<const StructType>(&left);
            const auto typeVarLeft = dynCast<const StructType>(&right);
            return areEquivalentTypes(*typeVarRight, *typeVarLeft);
        }
        case Type::Union: {
            const auto typeVarRight = dynCast<const UnionType>(&left);
            const auto typeVarLeft = dynCast<const UnionType>(&right);
            return areEquivalentTypes(*typeVarRight, *typeVarLeft);
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

bool areEquivalentTypes(const StructType& left, const StructType& right)
{
    return left.identifier == right.identifier;
}

bool areEquivalentTypes(const UnionType& left, const UnionType& right)
{
    return left.identifier == right.identifier;
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

void assignTypeToArithmeticBinaryExpr(BinaryExpr& binaryExpr)
{
    using BinaryOp = BinaryExpr::Operator;
    const Type leftType = binaryExpr.lhs->type->type;
    const Type rightType = binaryExpr.rhs->type->type;
    const Type commonType = getCommonType(leftType, rightType);

    if (commonType != leftType)
        binaryExpr.lhs = convertOrCastToType(binaryExpr.lhs, commonType);
    if (commonType != rightType)
        binaryExpr.rhs = convertOrCastToType(binaryExpr.rhs, commonType);
    if (binaryExpr.op == BinaryOp::LeftShift || binaryExpr.op == BinaryOp::RightShift) {
        if (isCharacterType(leftType)) {
            binaryExpr.lhs = std::make_unique<CastExpr>(
                std::make_unique<VarType>(Type::I32), std::move(binaryExpr.lhs));
            binaryExpr.type = std::make_unique<VarType>(Type::I32);
            return;
        }
        binaryExpr.type = std::make_unique<VarType>(leftType);
        return;
    }
    if (isBinaryComparison(binaryExpr.op)) {
        if (commonType == Type::Double || isSigned(commonType))
            binaryExpr.type = std::make_unique<VarType>(Type::I32);
        else
            binaryExpr.type = std::make_unique<VarType>(Type::U32);
        return;
    }
    binaryExpr.type = std::make_unique<VarType>(commonType);
}

bool isZeroArithmeticType(const ConstExpr& constExpr)
{
    const Type type  = constExpr.type->type;
    if (type == Type::I32)
        return 0 == std::get<i32>(constExpr.value);
    if (type == Type::U32)
        return 0 == std::get<u32>(constExpr.value);
    if (type == Type::I64)
        return 0 == std::get<i64>(constExpr.value);
    if (type == Type::U64)
        return 0 == std::get<u64>(constExpr.value);
    if (type == Type::Double)
        return 0.0 == std::get<double>(constExpr.value);
    return false;
}

bool canConvertToNullPtr(const Expr& expr)
{
    if (expr.kind != Expr::Kind::Constant)
        return false;
    const auto constExpr = dynCast<const ConstExpr>(&expr);
    if (!isIntegerType(constExpr->type->type))
        return false;
    return isZeroArithmeticType(*constExpr);
}

bool isBinaryComparison(const BinaryExpr::Operator oper)
{
    using Operator = BinaryExpr::Operator;
    return oper == Operator::Equal       || oper == Operator::NotEqual ||
           oper == Operator::LessThan    || oper == Operator::LessOrEqual ||
           oper == Operator::GreaterThan || oper == Operator::GreaterOrEqual;
}

std::unique_ptr<Expr> convertToArithmeticType(const Expr& expr, const Type targetType)
{
    const auto constExpr = dynCast<const ConstExpr>(&expr);
    std::variant<char, i8, u8, i32, i64, u32, u64, double> convertedValue;

    switch (targetType) {
        case Type::I8:
            convertedValue = getValueFromConst<i8>(*constExpr);
            break;
        case Type::U8:
            convertedValue = getValueFromConst<u8>(*constExpr);
            break;
        case Type::I32:
            convertedValue = getValueFromConst<i32>(*constExpr);
            break;
        case Type::U32:
            convertedValue = getValueFromConst<u32>(*constExpr);
            break;
        case Type::I64:
            convertedValue = getValueFromConst<i64>(*constExpr);
            break;
        case Type::U64:
            convertedValue = getValueFromConst<u64>(*constExpr);
            break;
        case Type::Double:
            convertedValue = getValueFromConst<double>(*constExpr);
            break;
        case Type::Char:
            convertedValue = getValueFromConst<char>(*constExpr);
            break;
        default:
            std::abort();
    }

    return std::make_unique<ConstExpr>(
        constExpr->location,
        convertedValue,
        std::make_unique<VarType>(targetType));
}

bool isVoidPointer(const TypeBase& type)
{
    if (type.kind != TypeBase::Kind::Pointer)
        return false;
    const auto pointer = dynCast<const PointerType>(&type);
    return pointer->referenced->type == Type::Void;
}

bool isVoidArray(const TypeBase& type)
{
    return getArrayType(&type) == Type::Void;
}

bool isStructuredType(const TypeBase& type)
{
    return type.type == Type::Struct || type.type == Type::Union;
}

bool isArrayOfVoidPointer(const TypeBase& type)
{
    if (type.kind != TypeBase::Kind::Array)
        return false;
    const TypeBase* currentType = &type;
    while (currentType->kind == TypeBase::Kind::Array) {
        const auto arrayType = dynCast<const ArrayType>(currentType);
        currentType = arrayType->elementType.get();
    }
    return isVoidPointer(*currentType);
}

bool isPointerToVoidArray(const TypeBase& type)
{
    if (type.kind != TypeBase::Kind::Pointer)
        return false;
    const auto pointer = dynCast<const PointerType>(&type);
    if (pointer->referenced->kind != TypeBase::Kind::Array)
        return false;
    return isVoidArray(*pointer->referenced);
}

bool isScalarType(const TypeBase& type)
{
    switch (type.type) {
        case Type::Void:
        case Type::Array:
        case Type::Function:
            return false;
        default:
            return true;
    }
}

const TypeBase* getArrayBaseType(const TypeBase& highestType)
{
    const TypeBase* type = &highestType;
    while (type->kind == TypeBase::Kind::Array) {
        const auto arrayType = dynCast<const ArrayType>(type);
        type = arrayType->elementType.get();
    }
    return type;
}

std::unique_ptr<Expr> convertOrCastToType(std::unique_ptr<Expr>& expr, const Type targetType)
{
    if (expr->kind != Expr::Kind::Constant) {
        return std::make_unique<CastExpr>(
            expr->location,
            std::make_unique<VarType>(targetType),
            std::move(expr));
    }
    return convertToArithmeticType(*expr, targetType);
}

i64 getArraySize(TypeBase* type)
{
    i64 result = 1;
    while (type->kind == TypeBase::Kind::Array) {
        const auto arrayType = dynCast<ArrayType>(type);
        result *= arrayType->size;
        type = arrayType->elementType.get();
    }
    return result;
}

Type getArrayType(const TypeBase* const type)
{
    return getArrayBaseType(*type)->type;
}

i64 getArrayAlignment(const i64 size, const Type type)
{
    const i64 realSize = size * getTypeSize(type);
    if (16 <= realSize)
        return 16;
    return getTypeSize(type);
}
} // Parsing