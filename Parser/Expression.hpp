#pragma once

#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <cstdint>

using i32 = int32_t;

namespace Parsing {

struct Expression {
    i32 constant;
};

}

#endif //EXPRESSION_HPP