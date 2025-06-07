#pragma once

#ifndef CC_TYPS_HPP
#define CC_TYPS_HPP

#include "../ShortTypes.hpp"

enum class ReferingTo : u8 {
    Static, Extern, Arg, Local
};

enum class Type {
    I32, I64, Function
};

#endif // CC_TYPS_HPP
