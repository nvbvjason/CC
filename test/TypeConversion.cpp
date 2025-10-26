#include "TypeConversion.hpp"
#include "Types/TypeConversion.hpp"

TEST(TypeConversion, isInteger_acceptsValidTypes)
{
    const std::vector types{Type::I32, Type::I64, Type::U32, Type::U64};
    for (const Type t : types)
        EXPECT_TRUE(isInteger(t));
}

TEST(TypeConversion, isInteger_rejectsInvalidTypes)
{
    const std::vector types{Type::Invalid, Type::Double, Type::Function, Type::Pointer};
    for (const Type t : types)
        EXPECT_FALSE(isInteger(t));
}

TEST(TypeConversion, isArithmetic_acceptsValidTypes)
{
    const std::vector types{Type::I32, Type::I64, Type::U32, Type::U64, Type::Double};
    for (const Type t : types)
        EXPECT_TRUE(isArithmetic(t));
}

TEST(TypeConversion, isArithmetic_rejectsInvalidTypes)
{
    const std::vector types{Type::Invalid, Type::Function, Type::Pointer};
    for (const Type t : types)
        EXPECT_FALSE(isArithmetic(t));
}