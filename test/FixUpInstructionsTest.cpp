#include "FixUpInstructionsTest.hpp"

#include "ASTIr.hpp"
#include "DynCast.hpp"
#include "FixUpInstructions.hpp"

std::array stackOperands{OperKind::Data, OperKind::Memory};
std::array stackOperandsAndImm{OperKind::Data, OperKind::Memory, OperKind::Imm};
std::array binaryShiftOpers{BinaryOper::LeftShiftSigned, BinaryOper::RightShiftSigned,
                            BinaryOper::RightShiftUnsigned, BinaryOper::LeftShiftUnsigned};
std::array binaryOtherOpers{BinaryOper::Add, BinaryOper::Sub,
                            BinaryOper::BitwiseAnd, BinaryOper::BitwiseOr, BinaryOper::BitwiseXor};

void FixUpInstructionsTest::addMove(const OperKind srcKind, const OperKind dstKind, const AsmType asmType)
{
    insts.push_back(factory.create(InstKind::Move, srcKind, dstKind, asmType));
}

void FixUpInstructionsTest::addMoveZero(const OperKind srcKind, const OperKind dstKind, const AsmType asmType)
{
    insts.push_back(factory.create(InstKind::MoveZeroExtend, srcKind, dstKind, asmType));
}

void FixUpInstructionsTest::addMoveSX(const OperKind srcKind, const OperKind dstKind)
{
    insts.push_back(factory.create(InstKind::MoveSX, srcKind, dstKind));
}

void FixUpInstructionsTest::addLea(const OperKind srcKind, const OperKind dstKind)
{
    insts.push_back(factory.create(InstKind::Lea, srcKind, dstKind));
}

void FixUpInstructionsTest::addDiv(const OperKind srcKind, const OperKind dstKind)
{
    insts.push_back(factory.create(InstKind::Div, srcKind, dstKind));
}

void FixUpInstructionsTest::addIdiv(const OperKind srcKind, const OperKind dstKind)
{
    insts.push_back(factory.create(InstKind::Idiv, srcKind, dstKind));
}

void FixUpInstructionsTest::addCvttsd2si(OperKind srcKind, OperKind dstKind)
{
    insts.push_back(factory.create(InstKind::Cvttsd2si, srcKind, dstKind));
}

void FixUpInstructionsTest::addCvtsi2sd(OperKind srcKind, OperKind dstKind)
{
    insts.push_back(factory.create(InstKind::Cvtsi2sd, srcKind, dstKind));
}

void FixUpInstructionsTest::addCmp(const OperKind srcKind, const OperKind dstKind, const AsmType asmType)
{
    insts.push_back(factory.create(InstKind::Cmp, srcKind, dstKind, asmType));
}

void FixUpInstructionsTest::addBinary(CodeGen::BinaryInst::Operator oper, AsmType asmType, OperKind srcKind, OperKind dstKind)
{
    insts.push_back(factory.createBinary(oper, asmType, srcKind, dstKind));
}

void FixUpInstructionsTest::run()
{
    CodeGen::FixUpInstructions fixUpInstructions(insts, 0);
    fixUpInstructions.fixUp();
}

void FixUpInstructionsTest::run(const i32 stackAlloc)
{
    CodeGen::FixUpInstructions fixUpInstructions(insts, stackAlloc);
    fixUpInstructions.fixUp();
}

TEST_F(FixUpInstructionsTest, _doNothing)
{
    run(0);
    EXPECT_TRUE(insts.empty());
}

TEST_F(FixUpInstructionsTest, fixStackAlignment_doNothing)
{
    run(0);
    EXPECT_TRUE(insts.empty());
}

TEST_F(FixUpInstructionsTest, fixStackAlignment_fixAlignment)
{
    run(-8);
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Binary);
    const auto binary = dynCast<CodeGen::BinaryInst>(insts[0].get());
    EXPECT_EQ(binary->lhs->kind, OperKind::Imm);
    EXPECT_EQ(binary->rhs->kind, OperKind::Register);
    EXPECT_EQ(binary->oper, BinaryOper::Sub);
    EXPECT_EQ(binary->type.kind, CodeGen::QuadWordType().kind);
    const auto imm = dynCast<CodeGen::ImmOperand>(binary->lhs.get());
    EXPECT_EQ(imm->value, 16);
}

TEST_F(FixUpInstructionsTest, fixMove_expandSrcOnStack)
{
    for (const OperKind stackOperand : stackOperands) {
        addMove(stackOperand, OperKind::Memory, CodeGen::DoubleType());
        run();
        EXPECT_EQ(insts.size(), 2);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        EXPECT_EQ(insts[1]->kind, InstKind::Move);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixMove_doNothing)
{
    addMove(OperKind::Imm, OperKind::Memory, CodeGen::DoubleType());
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Move);
}

TEST_F(FixUpInstructionsTest, fixMoveSX_doNothing)
{
    addMoveSX(OperKind::Register, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::MoveSX);
}

TEST_F(FixUpInstructionsTest, fixMoveSX_moveSrcImm)
{
    addMoveSX(OperKind::Imm, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 2);
    EXPECT_EQ(insts[0]->kind, InstKind::Move);
    EXPECT_EQ(insts[1]->kind, InstKind::MoveSX);
}

TEST_F(FixUpInstructionsTest, fixMoveSX_expandDstOnStack)
{
    for (const OperKind stackOperand : stackOperands) {
        addMoveSX(OperKind::Register, stackOperand);
        run();
        EXPECT_EQ(insts.size(), 2);
        EXPECT_EQ(insts[0]->kind, InstKind::MoveSX);
        EXPECT_EQ(insts[1]->kind, InstKind::Move);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixMoveZero_replaceWithMove)
{
    addMoveZero(OperKind::Register, OperKind::Register, CodeGen::DoubleType());
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Move);
}

TEST_F(FixUpInstructionsTest, fixMoveZero_replaceRegister)
{
    for (const OperKind stackOperand : stackOperands) {
        addMoveZero(OperKind::Register, stackOperand, CodeGen::DoubleType());
        run();
        EXPECT_EQ(insts.size(), 2);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixLea_doNothing)
{
    addLea(OperKind::Register, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Lea);
}

TEST_F(FixUpInstructionsTest, fixLea_fixStackDst)
{
    for (const OperKind stackOperand : stackOperands) {
        addLea(OperKind::Register, stackOperand);
        run();
        EXPECT_EQ(insts.size(), 2);
        EXPECT_EQ(insts[0]->kind, InstKind::Lea);
        EXPECT_EQ(insts[1]->kind, InstKind::Move);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixBinaryShift_replaceWithByteMove)
{
    for (const CodeGen::BinaryInst::Operator oper : binaryShiftOpers) {
        addBinary(oper, CodeGen::LongWordType(), OperKind::Register, OperKind::Register);
        run();
        EXPECT_EQ(insts.size(), 2);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        EXPECT_EQ(insts[1]->kind, InstKind::Binary);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixBinaryMulLong_doNothing)
{
    addBinary(BinaryOper::Mul, CodeGen::LongWordType(), OperKind::Register, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Binary);
}

TEST_F(FixUpInstructionsTest, fixBinaryMulLong_fixUpStackRhsOperand)
{
    for (const CodeGen::Operand::Kind oper : stackOperands) {
        addBinary(BinaryOper::Mul, CodeGen::LongWordType(), OperKind::Register, oper);
        run();
        EXPECT_EQ(insts.size(), 3);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        EXPECT_EQ(insts[1]->kind, InstKind::Binary);
        EXPECT_EQ(insts[2]->kind, InstKind::Move);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixBinaryMulDouble_doNothing)
{
    addBinary(BinaryOper::Mul, CodeGen::DoubleType(), OperKind::Register, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Binary);
}

TEST_F(FixUpInstructionsTest, fixBinaryMulDouble_fixUpStackRhsOperand)
{
    addBinary(BinaryOper::Mul, CodeGen::DoubleType(), OperKind::Register, OperKind::Memory);
    run();
    EXPECT_EQ(insts.size(), 3);
    EXPECT_EQ(insts[0]->kind, InstKind::Move);
    EXPECT_EQ(insts[1]->kind, InstKind::Binary);
    EXPECT_EQ(insts[2]->kind, InstKind::Move);
}

TEST_F(FixUpInstructionsTest, fixBinaryOtherLong_fixUpBothOnStack)
{
    for (const BinaryOper oper : binaryOtherOpers) {
        addBinary(oper, CodeGen::LongWordType(), OperKind::Memory, OperKind::Memory);
        run();
        EXPECT_EQ(insts.size(), 2);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        EXPECT_EQ(insts[1]->kind, InstKind::Binary);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixBinaryOtherLong_doNothing)
{
    for (const BinaryOper oper : binaryOtherOpers) {
        addBinary(oper, CodeGen::LongWordType(), OperKind::Register, OperKind::Memory);
        run();
        EXPECT_EQ(insts.size(), 1);
        EXPECT_EQ(insts[0]->kind, InstKind::Binary);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixBinaryDouble_fixUpBothOnStack)
{
    for (const BinaryOper oper : binaryOtherOpers) {
        addBinary(oper, CodeGen::DoubleType(), OperKind::Memory, OperKind::Memory);
        run();
        EXPECT_EQ(insts.size(), 3);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        EXPECT_EQ(insts[1]->kind, InstKind::Binary);
        EXPECT_EQ(insts[2]->kind, InstKind::Move);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixBinaryDouble_doNothing)
{
    for (const BinaryOper oper : binaryOtherOpers) {
        addBinary(oper, CodeGen::DoubleType(), OperKind::Memory, OperKind::Register);
        run();
        EXPECT_EQ(insts.size(), 1);
        EXPECT_EQ(insts[0]->kind, InstKind::Binary);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixCmp_doNothing)
{
    addCmp(OperKind::Memory, OperKind::Register, CodeGen::LongWordType());
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Cmp);
}

TEST_F(FixUpInstructionsTest, fixCmp_rhsRegisterAndTypeDouble)
{
    addCmp(OperKind::Register, OperKind::Memory, CodeGen::DoubleType());
    run();
    EXPECT_EQ(insts.size(), 2);
    EXPECT_EQ(insts[0]->kind, InstKind::Move);
    EXPECT_EQ(insts[1]->kind, InstKind::Cmp);
}

TEST_F(FixUpInstructionsTest, fixCmp_fixRhsImm)
{
    addCmp(OperKind::Memory, OperKind::Imm, CodeGen::LongWordType());
    run();
    EXPECT_EQ(insts.size(), 2);
    EXPECT_EQ(insts[0]->kind, InstKind::Move);
    EXPECT_EQ(insts[1]->kind, InstKind::Cmp);
}

TEST_F(FixUpInstructionsTest, fixCmp_fixUpBothOnStack)
{
    for (const OperKind oper : stackOperands) {
        addCmp(oper, oper, CodeGen::LongWordType());
        run();
        EXPECT_EQ(insts.size(), 2);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        EXPECT_EQ(insts[1]->kind, InstKind::Cmp);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixIdiv_doNothing)
{
    addIdiv(OperKind::Register, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Idiv);
}

TEST_F(FixUpInstructionsTest, fixIdiv_fixStackAndImmOperands)
{
    for (const OperKind stackOperand : stackOperands) {
        addIdiv(stackOperand, OperKind::Register);
        run();
        EXPECT_EQ(insts.size(), 2);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        EXPECT_EQ(insts[1]->kind, InstKind::Idiv);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixDiv_doNothing)
{
    addDiv(OperKind::Register, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Div);
}

TEST_F(FixUpInstructionsTest, fixDiv_fixStackAndImmOperands)
{
    for (const OperKind stackOperand : stackOperands) {
        addDiv(stackOperand, OperKind::Register);
        run();
        EXPECT_EQ(insts.size(), 2);
        EXPECT_EQ(insts[0]->kind, InstKind::Move);
        EXPECT_EQ(insts[1]->kind, InstKind::Div);
        TearDown();
    }
}

TEST_F(FixUpInstructionsTest, fixCvttsd2si_doNothing)
{
    addCvttsd2si(OperKind::Register, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Cvttsd2si);
}

TEST_F(FixUpInstructionsTest, fixCvttsd2si_fixDstNotRegister)
{
    addCvttsd2si(OperKind::Register, OperKind::Memory);
    run();
    EXPECT_EQ(insts.size(), 2);
    EXPECT_EQ(insts[0]->kind, InstKind::Cvttsd2si);
    EXPECT_EQ(insts[1]->kind, InstKind::Move);
}

TEST_F(FixUpInstructionsTest, fixCvtsi2sd_doNothing)
{
    addCvtsi2sd(OperKind::Register, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 1);
    EXPECT_EQ(insts[0]->kind, InstKind::Cvtsi2sd);
}

TEST_F(FixUpInstructionsTest, fixCvtsi2sd_fixDstNotRegister)
{
    addCvtsi2sd(OperKind::Register, OperKind::Memory);
    run();
    EXPECT_EQ(insts.size(), 2);
    EXPECT_EQ(insts[0]->kind, InstKind::Cvtsi2sd);
    EXPECT_EQ(insts[1]->kind, InstKind::Move);
}

TEST_F(FixUpInstructionsTest, fixCvtsi2sd_fixSrcImm)
{
    addCvtsi2sd(OperKind::Imm, OperKind::Register);
    run();
    EXPECT_EQ(insts.size(), 2);
    EXPECT_EQ(insts[0]->kind, InstKind::Move);
    EXPECT_EQ(insts[1]->kind, InstKind::Cvtsi2sd);
}

TEST_F(FixUpInstructionsTest, genSrcOperand_Double)
{
    const auto expected = make_shared<RegisterOperand>(RegType::XMM14, CodeGen::DoubleType());
    const auto actual = CodeGen::FixUpInstructions::genSrcOperand(CodeGen::DoubleType());
    EXPECT_EQ(expected->regKind, actual->regKind);
}

TEST_F(FixUpInstructionsTest, genSrcOperand_Long)
{
    const auto expected = make_shared<RegisterOperand>(RegType::R10, CodeGen::LongWordType());
    const auto actual = CodeGen::FixUpInstructions::genSrcOperand(CodeGen::LongWordType());
    EXPECT_EQ(expected->regKind, actual->regKind);
}

TEST_F(FixUpInstructionsTest, genDstOperand_Double)
{
    const auto expected = make_shared<RegisterOperand>(RegType::XMM15, CodeGen::DoubleType());
    const auto actual = CodeGen::FixUpInstructions::genDstOperand(CodeGen::DoubleType());
    EXPECT_EQ(expected->regKind, actual->regKind);
}

TEST_F(FixUpInstructionsTest, genDstOperand_Long)
{
    const auto expected = make_shared<RegisterOperand>(RegType::R11, CodeGen::LongWordType());
    const auto actual = CodeGen::FixUpInstructions::genDstOperand(CodeGen::LongWordType());
    EXPECT_EQ(expected->regKind, actual->regKind);
}