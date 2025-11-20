#include "ASTUtils.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTExpr.hpp"
#include "Types/TypeConversion.hpp"

#include <cassert>

namespace Parsing {

BinaryExpr::Operator convertAssignOperation(const AssignmentExpr::Operator assignOperation)
{
    using Assign = AssignmentExpr::Operator;
    using Binary = BinaryExpr::Operator;
    switch (assignOperation) {
        case Assign::PlusAssign:         return Binary::Add;
        case Assign::MinusAssign:        return Binary::Subtract;
        case Assign::MultiplyAssign:     return Binary::Multiply;
        case Assign::DivideAssign:       return Binary::Divide;
        case Assign::ModuloAssign:       return Binary::Modulo;
        case Assign::BitwiseAndAssign:   return Binary::BitwiseAnd;
        case Assign::BitwiseOrAssign:    return Binary::BitwiseOr;
        case Assign::BitwiseXorAssign:   return Binary::BitwiseXor;
        case Assign::LeftShiftAssign:    return Binary::LeftShift;
        case Assign::RightShiftAssign:   return Binary::RightShift;
        default:
            std::abort();
    }
}

std::unique_ptr<TypeBase> deepCopy(const TypeBase& typeBase)
{
    assert(typeBase.type != Type::Invalid);
    switch (typeBase.kind) {
        case TypeBase::Kind::Pointer: {
            const auto pointerType = dynCast<const PointerType>(&typeBase);
            auto referencedCopy = deepCopy(*pointerType->referenced);
            return std::make_unique<PointerType>(std::move(referencedCopy));
        }
        case TypeBase::Kind::Func: {
            const auto functionType = dynCast<const FuncType>(&typeBase);
            return deepCopy(*functionType);
        }
        case TypeBase::Kind::Array: {
            const auto arrayType = dynCast<const ArrayType>(&typeBase);
            return deepCopy(*arrayType);
        }
        case TypeBase::Kind::Var: {
            const auto varType = dynCast<const VarType>(&typeBase);
            return deepCopy(*varType);
        }
        case TypeBase::Kind::Structured: {
            const auto structuredType = dynCast<const StructuredType>(&typeBase);
            return deepCopy(*structuredType);
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

std::unique_ptr<TypeBase> deepCopy(const StructuredType& structuredType)
{
    return std::make_unique<StructuredType>(
        structuredType.type,
        structuredType.identifier,
        structuredType.location
    );
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
        case Type::Struct:
        case Type::Union:{
            const auto typeVarRight = dynCast<const StructuredType>(&left);
            const auto typeVarLeft = dynCast<const StructuredType>(&right);
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

bool areEquivalentTypes(const StructuredType& left, const StructuredType& right)
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
    switch (constExpr.type->type) {
        case Type::I32:     return 0 == std::get<i32>(constExpr.value);
        case Type::U32:     return 0 == std::get<u32>(constExpr.value);
        case Type::I64:     return 0 == std::get<i64>(constExpr.value);
        case Type::U64:     return 0 == std::get<u64>(constExpr.value);
        case Type::Double:  return 0 == std::get<double>(constExpr.value);
        default:
            return false;
    }
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
            convertedValue = constExpr->getValue<i8>();
            break;
        case Type::U8:
            convertedValue = constExpr->getValue<u8>();
            break;
        case Type::I32:
            convertedValue = constExpr->getValue<i32>();
            break;
        case Type::U32:
            convertedValue = constExpr->getValue<u32>();
            break;
        case Type::I64:
            convertedValue = constExpr->getValue<i64>();
            break;
        case Type::U64:
            convertedValue = constExpr->getValue<u64>();
            break;
        case Type::Double:
            convertedValue = constExpr->getValue<double>();
            break;
        case Type::Char:
            convertedValue = constExpr->getValue<char>();
            break;
        default:
            std::abort();
    }

    return std::make_unique<ConstExpr>(
        constExpr->location,
        convertedValue,
        std::make_unique<VarType>(targetType));
}

std::unique_ptr<Expr> converOrAssign(const TypeBase& left,
                                     const TypeBase& right,
                                     std::unique_ptr<Expr>& expr,
                                     std::vector<Error>& errors)
{
    if (left.type == Type::Void) {
        errors.emplace_back("Cannot assign to void", expr->location);
        return std::move(expr);
    }
    if (areEquivalentTypes(left, right))
        return std::move(expr);

    if (isArithmeticTypeBase(left) && isArithmeticTypeBase(*expr->type))
        return convertOrCastToType(expr, left.type);

    if (canConvertToNullPtr(*expr) && left.type == Type::Pointer)
        return convertOrCastToType(expr, Type::U64);

    if (expr->type->type == Type::Pointer && isVoidPointer(left)) {
        expr->type = std::make_unique<Parsing::PointerType>(std::make_unique<Parsing::VarType>(Type::Void));
        return std::move(expr);
    }

    if (left.type == Type::Pointer && isVoidPointer(right)) {
        expr->type = Parsing::deepCopy(left);
        return std::move(expr);
    }

    errors.emplace_back("Faulty assignment", expr->location);
    return std::move(expr);
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

bool isArithmeticTypeBase(const TypeBase& type)
{
    return isArithmetic(type.type);
}

bool isStructuredTypeBase(const TypeBase& type)
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
        case Type::Struct:
        case Type::Union:
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

const TypeBase* getPointerBaseType(const TypeBase& highestType)
{
    const TypeBase* type = &highestType;
    while (type->kind == TypeBase::Kind::Array || type->kind == TypeBase::Kind::Pointer) {
        switch (type->kind) {
            case TypeBase::Kind::Array: {
                const auto arrayType = dynCast<const ArrayType>(type);
                type = arrayType->elementType.get();
                break;
            }
            case TypeBase::Kind::Pointer: {
                const auto pointer = dynCast<const PointerType>(type);
                type = pointer->referenced.get();
                break;
            }
            default:
                return type;
        }

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

i64 getArraySize(const TypeBase* const type)
{
    i64 result = 1;
    const TypeBase* itType = dynCast<const ArrayType>(type);
    while (itType->kind == TypeBase::Kind::Array) {
        const auto arrayType = dynCast<const ArrayType>(itType);
        result *= arrayType->size;
        itType = arrayType->elementType.get();
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