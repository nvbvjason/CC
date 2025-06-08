#include "TypeConversion.hpp"

#include <cassert>
#include <utility>

Type getCommonType(const Type t1, const Type t2)
{
    if (t1 == t2)
        return t1;
    if (getSize(t1) == getSize(t2)) {
        if (isSigned(t1))
            return t1;
        return t2;
    }
    if (getSize(t2) < getSize(t1))
        return t1;
    return t2;
}

bool isSigned(const Type t)
{
    switch (t) {
        case Type::I64:
        case Type::I32:
            return true;
        case Type::U64:
        case Type::U32:
            return false;
    }
    return false;
}

i32 getSize(const Type t)
{
    switch (t) {
        case Type::I64:
            return 8;
        case Type::I32:
            return 4;
        case Type::U64:
            return 8;
        case Type::U32:
            return 4;
        default:
            assert("getSize");
    }
    std::unreachable();
}