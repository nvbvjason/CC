#pragma once

#include "AsmAST.hpp"

#include <memory>
#include <vector>
#include <gtest/gtest.h>

#include "GenerateAsmTree.hpp"

namespace CodeGen {

using RegType = Operand::RegKind;
using OperKind = Operand::Kind;
using InstKind = Inst::Kind;
using AsmType = AsmType;
using BinaryOper = BinaryInst::Operator;
using IrType = Ir::IrType;

class GenerateAsmTreeTest : public testing::Test {
    Program program;

public:
    std::vector<std::unique_ptr<Inst>> insts;

    GenerateAsmTree genGenerateAsmTree();

    void actGenFunctionPushOntoStack(
        const std::vector<bool>& argsPushedOnStack,
        std::vector<Ir::Identifier>&& args,
        std::vector<IrType>&& argTypes
    );
    void actGenFunctionPushOntoStackSimple(std::vector<IrType>&& argTypes);
protected:
    void TearDown() override
    {
        program.operands.clear();
        program.topLevels.clear();
    }

};

static Ir::Function genIrFunction(std::vector<Ir::Identifier>&& args, std::vector<IrType>&& argTypes);
}
