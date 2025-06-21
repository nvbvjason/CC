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
    if (getSize(t1) == getSize(t2)) {
        if (isSigned(t1))
            return t2;
        return t1;
    }
    if (getSize(t2) < getSize(t1))
        return t1;
    return t2;
}

bool isSigned(const Type t)
{
    assert(t != Type::Invalid);
    if (!isInteger(t))
        return false;
    switch (t) {
        case Type::I64:
        case Type::I32:
            return true;
        case Type::U64:
        case Type::U32:
            return false;
        default: ;
        std::unreachable();
    }
}

i32 getSize(const Type t)
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
        default:
            assert("getSize");
    }
    std::unreachable();
}

bool isInteger(const Type t)
{
    switch (t) {
        case Type::I32:
        case Type::I64:
        case Type::U32:
        case Type::U64:
            return true;
        default:
            return false;
    }
}