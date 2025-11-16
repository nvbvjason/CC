#include "TypeConversion.hpp"

#include <cassert>
#include <utility>

Type getCommonType(const Type t1, const Type t2)
{
    assert(t1 != Type::Function);
    assert(t2 != Type::Function);
    if (t1 == t2)
        return t1;
    if (t1 == Type::Double || t2 == Type::Double)
        return Type::Double;
    if (isCharacterType(t1) || isCharacterType(t2))
        return Type::I32;
    if (getTypeSize(t1) == getTypeSize(t2)) {
        if (isSigned(t1))
            return t2;
        return t1;
    }
    if (getTypeSize(t2) < getTypeSize(t1))
        return t1;
    return t2;
}

bool isSigned(const Type t)
{
    assert(t != Type::Invalid);
    if (!isIntegerType(t))
        return false;
    switch (t) {
        case Type::I64:
        case Type::I32:
        case Type::I8:
        case Type::Char:
            return true;
        case Type::U64:
        case Type::U32:
        case Type::U8:
            return false;
        default: ;
        std::unreachable();
    }
}

i64 getTypeSize(const Type t)
{
    assert(t != Type::Invalid);
    assert(t != Type::Function);
    switch (t) {
        case Type::I64:
        case Type::U64:
        case Type::Double:
        case Type::Pointer:
            return 8;
        case Type::I32:
        case Type::U32:
            return 4;
        case Type::I8:
        case Type::U8:
        case Type::Char:
            return 1;
        default:
            assert("getSize");
    }
    std::unreachable();
}

bool isIntegerType(const Type t)
{
    switch (t) {
        case Type::Char:
        case Type::I8:
        case Type::U8:
        case Type::I32:
        case Type::U32:
        case Type::I64:
        case Type::U64:
            return true;
        default:
            return false;
    }
}

bool isArithmetic(const Type t)
{
    if (isIntegerType(t))
        return true;
    if (t == Type::Double)
        return true;
    return false;
}

bool isCharacterType(const Type t)
{
    return t == Type::Char || t == Type::I8 || t == Type::U8;
}