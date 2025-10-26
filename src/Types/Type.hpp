#pragma once

#ifndef CC_TYPS_HPP
#define CC_TYPS_HPP

#include "../ShortTypes.hpp"

enum class ReferingTo : u8 {
    Static, Extern, Arg, Local
};

enum class Type : u16 {
    Invalid, I32, I64, U32, U64, Double, Function, Pointer
};

#endif // CC_TYPS_HPP