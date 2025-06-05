#include <gtest/gtest.h>

#include "GenerateIr.hpp"
#include "IR/GenerateIr.hpp"
#include "ASTIr.hpp"
#include "Frontend/AST/ASTParser.hpp"
#include "Frontend/AST/ASTTypes.hpp"

TEST(GenerateIrTests, VarExprToIrPreservesType)
{
    auto varExpr = std::make_unique<Parsing::VarExpr>("v");
    varExpr->type = std::make_unique<Parsing::VarType>(Type::I32);
    const auto expected = std::make_shared<Ir::ValueVar>(Ir::Identifier("v"), Type::I32);
    const std::shared_ptr<Ir::Value> result = Ir::GenerateIr::generateVarInst(*varExpr);
    EXPECT_EQ(expected->kind, result->kind);
}

TEST(GenerateIrTests, ConstExprToIrPreservesType)
{
    const auto constExpr = std::make_unique<Parsing::ConstExpr>(
        5, std::make_unique<Parsing::VarType>(Type::I32)
        );
    const auto expected = std::make_shared<Ir::ValueConst>(5);
    const std::shared_ptr<Ir::Value> result = Ir::GenerateIr::generateConstInst(*constExpr);
    EXPECT_EQ(expected->kind, result->kind);
}