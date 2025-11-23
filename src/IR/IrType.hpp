#pragma once

#include "ShortTypes.hpp"

namespace Ir {

struct IrType final {
    enum class Kind : u8 {
        Char, I8, U8, I32, U32, I64, U64, Pointer, Void, Double, ByteArray
    };
    const i64 size;
    const Kind kind;

    constexpr explicit IrType(const i64 size)
        : size(size), kind(Kind::ByteArray) {}
    constexpr explicit IrType(const Kind kind, const i64 size)
        : size(size), kind(kind) {}

    IrType() = delete;
};

inline bool operator==(const IrType a, const IrType b)
{
    return a.kind == b.kind && a.size == b.size;
}

inline bool isSigned(const IrType type)
{
    return type.kind == IrType::Kind::Char ||
           type.kind == IrType::Kind::I8 ||
           type.kind == IrType::Kind::I32 ||
           type.kind == IrType::Kind::I64;
}

constexpr auto i8Type = IrType(IrType::Kind::I8, 1);
constexpr auto u8Type = IrType(IrType::Kind::U8, 1);
constexpr auto charType = IrType(IrType::Kind::Char, 1);
constexpr auto i32Type = IrType(IrType::Kind::I32, 4);
constexpr auto u32Type = IrType(IrType::Kind::U32, 4);
constexpr auto i64Type = IrType(IrType::Kind::I64, 8);
constexpr auto u64Type = IrType(IrType::Kind::U64, 8);
constexpr auto pointerType = IrType(IrType::Kind::Pointer, 8);
constexpr auto doubleType = IrType(IrType::Kind::Double ,8);
constexpr auto voidType = IrType(IrType::Kind::Void, 0);

} // Ir