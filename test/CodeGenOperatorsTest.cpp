#include "CodeGen/Operators.hpp"
#include "CodeGen/AsmAST.hpp"
#include "IR/ASTIr.hpp"

#include <gtest/gtest.h>

TEST(CodeGenOperatorsTest, unaryOperator)
{
    using IrOper = Ir::UnaryInst::Operation;
    using AsmOper = CodeGen::UnaryInst::Operator;
    struct TestcaseUnary {
        const IrOper irOper;
        const AsmOper asmOper;
        TestcaseUnary(const IrOper irOp, const AsmOper asmOp)
            : irOper(irOp), asmOper(asmOp) {  }
    };
    const std::vector<TestcaseUnary> testcases{
        {IrOper::Complement, AsmOper::Not},
        {IrOper::Negate, AsmOper::Neg}
    };
    for (const TestcaseUnary& testcase : testcases) {
        EXPECT_EQ(CodeGen::Operators::unaryOperator(testcase.irOper), testcase.asmOper);
    }
}

TEST(CodeGenOperatorsTest, binaryOperator)
{
    using IrOper = Ir::BinaryInst::Operation;
    using AsmOper = CodeGen::BinaryInst::Operator;
    struct TestcaseBinary {
        const IrOper irOper;
        const AsmOper asmOper;
        TestcaseBinary(const IrOper irOp, const AsmOper asmOp)
            : irOper(irOp), asmOper(asmOp) {  }
    };
    const std::vector<TestcaseBinary> testcases{
            {IrOper::Add, AsmOper::Add},
            {IrOper::Subtract, AsmOper::Sub},
            {IrOper::Multiply, AsmOper::Mul},
            {IrOper::BitwiseAnd, AsmOper::BitwiseAnd},
            {IrOper::BitwiseOr, AsmOper::BitwiseOr},
            {IrOper::BitwiseXor, AsmOper::BitwiseXor},
    };
    for (const TestcaseBinary& testcase : testcases) {
        EXPECT_EQ(CodeGen::Operators::binaryOperator(testcase.irOper), testcase.asmOper);
    }
}

TEST(CodeGenOperatorsTest, shiftOperators)
{
    using IrOper = Ir::BinaryInst::Operation;
    using AsmOper = CodeGen::BinaryInst::Operator;
    struct TestcaseCond {
        const IrOper irOper;
        const AsmOper asmOper;
        const bool isSigned;
        TestcaseCond(const IrOper irOp, const AsmOper asmOper, const bool isSigned)
            : irOper(irOp), asmOper(asmOper), isSigned(isSigned) {  }
    };
    const std::vector<TestcaseCond> testcases{
                    {IrOper::LeftShift, AsmOper::LeftShiftSigned, true},
                    {IrOper::RightShift, AsmOper::RightShiftSigned, true},
                    {IrOper::LeftShift, AsmOper::LeftShiftUnsigned, false},
                    {IrOper::RightShift, AsmOper::RightShiftUnsigned, false},
            };
    for (const TestcaseCond& testcase : testcases) {
        EXPECT_EQ(CodeGen::Operators::getShiftOperator(testcase.irOper, testcase.isSigned), testcase.asmOper);
    }
}

TEST(CodeGenOperatorsTest, condCode)
{
    using IrOper = Ir::BinaryInst::Operation;
    using BinCond = CodeGen::BinaryInst::CondCode;
    struct TestcaseCond {
        const IrOper irOper;
        const BinCond cond;
        const bool isSigned;
        TestcaseCond(const IrOper irOp, const BinCond cond, const bool isSigned)
            : irOper(irOp), cond(cond), isSigned(isSigned) {  }
    };
    const std::vector<TestcaseCond> testcases{
                {IrOper::Equal, BinCond::E, true},
                {IrOper::NotEqual, BinCond::NE, true},
                {IrOper::LessThan, BinCond::L, true},
                {IrOper::LessOrEqual, BinCond::LE, true},
                {IrOper::GreaterThan, BinCond::G, true},
                {IrOper::GreaterOrEqual, BinCond::GE, true},

                {IrOper::Equal, BinCond::E, false},
                {IrOper::NotEqual, BinCond::NE, false},
                {IrOper::LessThan, BinCond::B, false},
                {IrOper::LessOrEqual, BinCond::BE, false},
                {IrOper::GreaterThan, BinCond::A, false},
                {IrOper::GreaterOrEqual, BinCond::AE, false},
        };
    for (const TestcaseCond& testcase : testcases) {
        EXPECT_EQ(CodeGen::Operators::condCode(testcase.irOper, testcase.isSigned), testcase.cond);
    }
}

TEST(CodeGenOperatorsTest, getAsmType)
{
    using AsmType = CodeGen::AsmType;
    struct TestcaseAsmType {
        const Type type;
        const AsmType asmType;
        TestcaseAsmType(const Type type, const AsmType asmType)
            : type(type), asmType(asmType) {  }
    };
    const std::vector<TestcaseAsmType> testcases{
                {Type::I32, CodeGen::LongWordType()},
                {Type::U32, CodeGen::LongWordType()},
                {Type::I64, CodeGen::QuadWordType()},
                {Type::U64, CodeGen::QuadWordType()},
                {Type::Pointer, CodeGen::QuadWordType()},
                {Type::Double, CodeGen::DoubleType()},
            };
    for (const TestcaseAsmType& testcase : testcases) {
        EXPECT_EQ(CodeGen::Operators::getAsmType(testcase.type).kind, testcase.asmType.kind);
    }
}