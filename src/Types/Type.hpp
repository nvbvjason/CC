#pragma once

#include "../ShortTypes.hpp"

enum class ReferingTo : u8 {
    Static, Extern, Arg, Local
};

enum class Type {
    Invalid, I32, I64, U32, U64, Double, Function, Pointer, Array
};