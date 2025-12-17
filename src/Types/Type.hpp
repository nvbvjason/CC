#pragma once

#include "../ShortTypes.hpp"

enum class ReferringTo : u8 {
    Static, Extern, Arg, Local
};

enum class Type : u8 {
    Invalid, Char, U8, I8, I32, I64, U32, U64, Double, Function, Pointer, Array, String, Void, Struct, Union
};