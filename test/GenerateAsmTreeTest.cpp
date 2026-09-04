#include "GenerateAsmTreeTest.hpp"

#include "AsmPrinter.hpp"
#include "DynCast.hpp"
#include "IrType.hpp"
#include "IrPrinter.hpp"

#include <ostream>

namespace CodeGen {

GenerateAsmTree GenerateAsmTreeTest::genGenerateAsmTree()
{
    return GenerateAsmTree(program);
}

void GenerateAsmTreeTest::actGenFunctionPushOntoStack(
    const std::vector<bool>& argsPushedOnStack,
    std::vector<Ir::Identifier>&& args,
    std::vector<IrType>&& argTypes)
{
    GenerateAsmTree generateAsmTree = genGenerateAsmTree();
    const Ir::Function function = genIrFunction(std::move(args), std::move(argTypes));
    generateAsmTree.genFunctionPushOntoStack(function, argsPushedOnStack);
    insts = std::move(generateAsmTree.insts);
    generateAsmTree.insts = std::vector<std::unique_ptr<Inst>>();
}

void GenerateAsmTreeTest::actGenFunctionPushOntoStackSimple(std::vector<IrType>&& argTypes)
{
    std::vector<bool> argsPushedOnStack(argTypes.size(), false);
    std::vector<Ir::Identifier> args;
    args.reserve(argTypes.size());

    for (int i = 0; i < argTypes.size(); ++i)
        args.emplace_back(std::to_string(i));

    actGenFunctionPushOntoStack(argsPushedOnStack, std::move(args), std::move(argTypes));
}

static std::vector<MoveInst> castStackMoves(const std::vector<std::unique_ptr<Inst>>& stackMoves)
{
    std::vector<MoveInst> result;
    result.reserve(stackMoves.size());

    for (const std::unique_ptr<Inst>& inst : stackMoves)
        result.emplace_back(*dynCast<MoveInst>(inst.get()));

    return result;
}

static std::vector<MemoryOperand> castMemoryOperands(const std::vector<MoveInst>& stackMoves)
{
    std::vector<MemoryOperand> result;
    result.reserve(stackMoves.size());

    for (const MoveInst& inst : stackMoves) {
        const auto* memOper = dynCast<const MemoryOperand>(inst.src);
        result.emplace_back(memOper->regKind, memOper->value, memOper->type, memOper->offset);
    }

    return result;
}

Ir::Function genIrFunction(std::vector<Ir::Identifier>&& args, std::vector<IrType>&& argTypes)
{
    Ir::Function irFunction("function", true);
    irFunction.args = std::move(args);
    irFunction.argTypes = std::move(argTypes);
    return irFunction;
}

TEST_F(GenerateAsmTreeTest, genFunctionPushOntoStack_ignoreAgrsPushedOnStack)
{
    const std::vector argsPushedOnStack(1, true);
    std::vector args{
        Ir::Identifier("first")
    };
    std::vector argTypes{
        IrType(1)
    };

    actGenFunctionPushOntoStack(
        argsPushedOnStack,
        std::move(args),
        std::move(argTypes)
    );

    EXPECT_TRUE(insts.empty());
}

namespace {
    struct GenerateAsmTreeTestSingleArgTestCase_type {
        IrType irType;
        AsmType::Kind expected;
    };

    struct GenerateAsmTreeTestSingleArgTestCase_alignment {
        IrType irType;
        i64 expected;
    };
}

TEST_F(GenerateAsmTreeTest, fixStackAlignment_singleArgument_type)
{
    std::vector<GenerateAsmTreeTestSingleArgTestCase_type> testCases{
        {Ir::i8Type, AsmType::Kind::Byte},
        {Ir::u8Type, AsmType::Kind::Byte},
        {Ir::charType, AsmType::Kind::Byte},
        {Ir::i32Type, AsmType::Kind::LongWord},
        {Ir::u32Type, AsmType::Kind::LongWord},
        {Ir::i64Type, AsmType::Kind::QuadWord},
        {Ir::u64Type, AsmType::Kind::QuadWord},
        {Ir::pointerType, AsmType::Kind::QuadWord},
        {Ir::doubleType, AsmType::Kind::Double},
    };

    for (const GenerateAsmTreeTestSingleArgTestCase_type& testCase : testCases) {
        std::vector irTypes{testCase.irType};

        actGenFunctionPushOntoStackSimple(std::move(irTypes));

        const std::vector<MoveInst> stackMoves = castStackMoves(insts);

        EXPECT_EQ(stackMoves.front().type.kind, testCase.expected);
    }
}

TEST_F(GenerateAsmTreeTest, fixStackAlignment_singleArgument_stackAlignment)
{
    std::vector<GenerateAsmTreeTestSingleArgTestCase_alignment> testCases{
        {Ir::i8Type, 8},
        {Ir::u8Type, 8},
        {Ir::charType, 8},
        {Ir::i32Type, 8},
        {Ir::u32Type, 8},
        {Ir::i64Type, 8},
        {Ir::u64Type, 8},
        {Ir::pointerType, 8},
        {Ir::doubleType, 8},
    };

    for (const GenerateAsmTreeTestSingleArgTestCase_alignment& testCase : testCases) {
        std::vector irTypes{testCase.irType};

        actGenFunctionPushOntoStackSimple(std::move(irTypes));

        const std::vector<MoveInst> stackMoves = castStackMoves(insts);
        const std::vector<MemoryOperand> memoryOperands = castMemoryOperands(stackMoves);

        EXPECT_EQ(memoryOperands.front().value - 8, testCase.expected) << Ir::to_string(testCase.irType);
    }
}

}
