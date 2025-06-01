#include <gtest/gtest.h>

#include "GenerateIr.hpp"
#include "IR/GenerateIr.hpp"

#include "ASTIr.hpp"
#include "Frontend/AST/ASTParser.hpp"

TEST(GenerateIrTests, VarExprToIrPreservesType)
{
    const auto varExpr = std::make_unique<Parsing::VarExpr>("v");
    const auto expected = std::make_shared<Ir::ValueVar>(Ir::Identifier("v"));
    const std::shared_ptr<Ir::Value> result = Ir::GenerateIr::generateVarInst(*varExpr);
    EXPECT_EQ(expected->type, result->type);
}

TEST(GenerateIrTests, ConstExprToIrPreservesType)
{
    const auto constExpr = std::make_unique<Parsing::ConstExpr>(5, Parsing::VarType(Parsing::VarType::Kind::Int));
    const auto expected = std::make_shared<Ir::ValueConst>(5);
    const std::shared_ptr<Ir::Value> result = Ir::GenerateIr::generateConstInst(*constExpr);
    EXPECT_EQ(expected->type, result->type);
}