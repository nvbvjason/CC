#pragma once

#include <cstdlib>

template<typename To, typename From>
To *dynCast(From *Val)
{
    if (To::classOf(Val))
        return static_cast<To*>(Val);
    std::abort();
}