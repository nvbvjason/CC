#include "TypeConversion.hpp"

Type getCommonType(const Type t1, const Type t2)
{
    if (t1 == t2)
        return t1;
    return Type::I64;
}