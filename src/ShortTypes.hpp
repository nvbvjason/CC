#pragma once

#include <cstdint>

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

static_assert(sizeof(i8) == 1,  "Non-8-bit char platform?");
static_assert(sizeof(u8) == 1,  "Non-8-bit char platform?");