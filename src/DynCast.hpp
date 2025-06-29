#pragma once

#ifndef CC_DYN_CAST_HPP
#define CC_DYN_CAST_HPP

#include <cstdlib>

template <typename To, typename From>
To *dyn_cast(From *Val)
{
    if (To::classOf(Val))
        return static_cast<To*>(Val);
    std::abort();
}

#endif // CC_DYN_CAST_HPP