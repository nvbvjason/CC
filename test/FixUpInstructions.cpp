#include "FixUpInstructions.hpp"
#include "CodeGen/FixUpInstructions.hpp"
#include "AsmAST.hpp"

#include <memory>
#include <vector>
#include <gtest/gtest.h>

namespace {
    using RegType = CodeGen::Operand::RegKind;
    using AsmType = CodeGen::AsmType;
    using RegisterOperand = CodeGen::RegisterOperand;
    using std::make_shared;
}

struct TestData {
    std::vector<std::unique_ptr<CodeGen::Inst>> insts;
    std::vector<std::unique_ptr<CodeGen::Inst>> copy;
};

TEST(FixUpInstructions, genSrcOperand_Double)
{
    const auto expected = make_shared<RegisterOperand>(RegType::XMM14, AsmType::Double);
    const auto actual = CodeGen::FixUpInstructions::genSrcOperand(AsmType::Double);
    EXPECT_EQ(expected->regKind, actual->regKind);
}

TEST(FixUpInstructions, genSrcOperand_Long)
{
    const auto expected = make_shared<RegisterOperand>(RegType::R10, AsmType::LongWord);
    const auto actual = CodeGen::FixUpInstructions::genSrcOperand(AsmType::LongWord);
    EXPECT_EQ(expected->regKind, actual->regKind);
}

TEST(FixUpInstructions, genDstOperand_Double)
{
    const auto expected = make_shared<RegisterOperand>(RegType::XMM15, AsmType::Double);
    const auto actual = CodeGen::FixUpInstructions::genDstOperand(AsmType::Double);
    EXPECT_EQ(expected->regKind, actual->regKind);
}

TEST(FixUpInstructions, genDstOperand_Long)
{
    const auto expected = make_shared<RegisterOperand>(RegType::R11, AsmType::LongWord);
    const auto actual = CodeGen::FixUpInstructions::genDstOperand(AsmType::LongWord);
    EXPECT_EQ(expected->regKind, actual->regKind);
}