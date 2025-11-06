#include "Assembly.hpp"
#include "AsmAST.hpp"
#include "AsmPrinter.hpp"
#include "CodeGen/Assembly.hpp"
#include "ASTIr.hpp"

#include <gtest/gtest.h>
#include <utility>

namespace {
using AsmType = CodeGen::AsmType;
using CondCode = CodeGen::BinaryInst::CondCode;
using RegKind = CodeGen::Operand::RegKind;
using BinaryOper = CodeGen::BinaryInst::Operator;
using UnaryOper = CodeGen::UnaryInst::Operator;
using PseudoOperand = CodeGen::PseudoOperand;
using Iden = CodeGen::Identifier;
using DataOperand = CodeGen::DataOperand;
using ImmOperand = CodeGen::ImmOperand;
using RegisterOperand = CodeGen::RegisterOperand;
using MemoryOperand = CodeGen::MemoryOperand;
using std::make_shared;
}

TEST(AssemblyTests, addType)
{
    struct TestDataAddType {
        const std::string expected;
        const AsmType type;
        TestDataAddType(std::string expected, const AsmType type)
            : expected(std::move(expected)), type(type) {}
    };
    const std::vector<TestDataAddType> tests = {
        {"addl", CodeGen::LongWordType()},
        {"addq", CodeGen::QuadWordType()},
        {"addsd", CodeGen::DoubleType()},
        {"add not set addType", CodeGen::ByteType()},
    };
    const std::string start = "add";
    for (const TestDataAddType& test : tests) {
        const std::string withType = CodeGen::addType(start, test.type);
        EXPECT_EQ(withType, test.expected) << "Instruction mismatch for input: " << CodeGen::to_string(test.type) << '\n';
    }
}

TEST(AssemblyTests, condCode)
{
    struct TestDataCondCode {
        const std::string expected;
        const CondCode condCode;
        TestDataCondCode(std::string expected, const CondCode condCode)
            : expected(std::move(expected)), condCode(condCode) {}
    };
    const std::vector<TestDataCondCode> tests = {
        {"e", CondCode::E},
        {"ne", CondCode::NE},
        {"l", CondCode::L},
        {"le", CondCode::LE},
        {"g", CondCode::G},
        {"ge", CondCode::GE},
        {"a", CondCode::A},
        {"ae", CondCode::AE},
        {"b", CondCode::B},
        {"be", CondCode::BE},
        {"p", CondCode::PF},
    };
    for (const TestDataCondCode& test : tests) {
        const std::string withType = CodeGen::condCode(test.condCode);
        EXPECT_EQ(withType, test.expected) << "Instruction mismatch for input: "
                                           << CodeGen::to_string(test.condCode) << '\n';
    }
}

TEST(AssemblyTests, asmRegister)
{
    struct TestDataAsmRegister {
        const std::string expected;
        const AsmType type;
        const RegKind reg;
        TestDataAsmRegister(std::string expected, const AsmType type, const RegKind reg)
            : expected(std::move(expected)), type(type), reg(reg) {}
    };
    const std::vector<TestDataAsmRegister> tests = {
        {"%rbp", CodeGen::DoubleType(), RegKind::BP},

        {"%xmm0", CodeGen::DoubleType(), RegKind::XMM0},
        {"%xmm1", CodeGen::DoubleType(), RegKind::XMM1},
        {"%xmm2", CodeGen::DoubleType(), RegKind::XMM2},
        {"%xmm3", CodeGen::DoubleType(), RegKind::XMM3},
        {"%xmm4", CodeGen::DoubleType(), RegKind::XMM4},
        {"%xmm5", CodeGen::DoubleType(), RegKind::XMM5},
        {"%xmm6", CodeGen::DoubleType(), RegKind::XMM6},
        {"%xmm7", CodeGen::DoubleType(), RegKind::XMM7},
        {"%xmm14", CodeGen::DoubleType(), RegKind::XMM14},
        {"%xmm15", CodeGen::DoubleType(), RegKind::XMM15},

        {"%al", CodeGen::ByteType(), RegKind::AX},
        {"%ax", CodeGen::WordType(), RegKind::AX},
        {"%eax", CodeGen::LongWordType(), RegKind::AX},
        {"%rax", CodeGen::QuadWordType(), RegKind::AX},

        {"%cl", CodeGen::ByteType(), RegKind::CX},
        {"%cx", CodeGen::WordType(), RegKind::CX},
        {"%ecx", CodeGen::LongWordType(), RegKind::CX},
        {"%rcx", CodeGen::QuadWordType(), RegKind::CX},

        {"%dl", CodeGen::ByteType(), RegKind::DX},
        {"%dx", CodeGen::WordType(), RegKind::DX},
        {"%edx", CodeGen::LongWordType(), RegKind::DX},
        {"%rdx", CodeGen::QuadWordType(), RegKind::DX},

        {"%dil", CodeGen::ByteType(), RegKind::DI},
        {"%di", CodeGen::WordType(), RegKind::DI},
        {"%edi", CodeGen::LongWordType(), RegKind::DI},
        {"%rdi", CodeGen::QuadWordType(), RegKind::DI},

        {"%sil", CodeGen::ByteType(), RegKind::SI},
        {"%si", CodeGen::WordType(), RegKind::SI},
        {"%esi", CodeGen::LongWordType(), RegKind::SI},
        {"%rsi", CodeGen::QuadWordType(), RegKind::SI},
        {"invalid_size", CodeGen::DoubleType(), RegKind::SI},

        {"%r8b", CodeGen::ByteType(), RegKind::R8},
        {"%r8w", CodeGen::WordType(), RegKind::R8},
        {"%r8d", CodeGen::LongWordType(), RegKind::R8},
        {"%r8", CodeGen::QuadWordType(), RegKind::R8},

        {"%r9b", CodeGen::ByteType(), RegKind::R9},
        {"%r9w", CodeGen::WordType(), RegKind::R9},
        {"%r9d", CodeGen::LongWordType(), RegKind::R9},
        {"%r9", CodeGen::QuadWordType(), RegKind::R9},

        {"%r10b", CodeGen::ByteType(), RegKind::R10},
        {"%r10w", CodeGen::WordType(), RegKind::R10},
        {"%r10d", CodeGen::LongWordType(), RegKind::R10},
        {"%r10", CodeGen::QuadWordType(), RegKind::R10},

        {"%r11b", CodeGen::ByteType(), RegKind::R11},
        {"%r11w", CodeGen::WordType(), RegKind::R11},
        {"%r11d", CodeGen::LongWordType(), RegKind::R11},
        {"%r11", CodeGen::QuadWordType(), RegKind::R11},

        {"%rsp", CodeGen::ByteType(), RegKind::SP},
        {"%rsp", CodeGen::WordType(), RegKind::SP},
        {"%rsp", CodeGen::LongWordType(), RegKind::SP},
        {"%rsp", CodeGen::QuadWordType(), RegKind::SP},
    };
    for (const TestDataAsmRegister& test : tests) {
        const std::string withType = CodeGen::asmRegister(test.type, test.reg);
        EXPECT_EQ(withType, test.expected) << "Instruction mismatch for input: "
                                           << CodeGen::to_string(test.type) << ' '
                                           << CodeGen::to_string(test.reg) << '\n';
    }
}

TEST(AssemblyTests, asmUnaryOperator)
{
    struct TestDataUnaryOperator {
        const std::string expected;
        const UnaryOper oper;
        TestDataUnaryOperator(std::string expected, const UnaryOper oper)
            : expected(std::move(expected)), oper(oper) {}
    };
    const std::vector<TestDataUnaryOperator> tests = {
        {"negl", UnaryOper::Neg},
        {"notl", UnaryOper::Not},
        {"shrl", UnaryOper::Shr},
    };
    for (const TestDataUnaryOperator& test : tests) {
        const std::string operString = CodeGen::asmUnaryOperator(test.oper, CodeGen::LongWordType());
        EXPECT_EQ(operString, test.expected) << "Instruction mismatch for input: "
                                   << CodeGen::to_string(test.oper)<< '\n';
    }
}

TEST(AssemblyTests, asmBinaryOperator)
{
    struct TestDataBinaryOperator {
        const std::string expected;
        const BinaryOper oper;
        const AsmType type;
        TestDataBinaryOperator(std::string expected, const BinaryOper oper, const AsmType type)
            : expected(std::move(expected)), oper(oper), type(type) {}
    };
    const std::vector<TestDataBinaryOperator> tests = {
        {"xorpd", BinaryOper::BitwiseXor, CodeGen::DoubleType()},
        {"mulsd", BinaryOper::Mul, CodeGen::DoubleType()},
        {"divsd", BinaryOper::DivDouble, CodeGen::DoubleType()},
        {"imull", BinaryOper::Mul, CodeGen::LongWordType()},
        {"addl", BinaryOper::Add, CodeGen::LongWordType()},
        {"subl", BinaryOper::Sub, CodeGen::LongWordType()},
        {"andl", BinaryOper::BitwiseAnd, CodeGen::LongWordType()},
        {"orl", BinaryOper::BitwiseOr, CodeGen::LongWordType()},
        {"xorl", BinaryOper::BitwiseXor, CodeGen::LongWordType()},
        {"shll", BinaryOper::LeftShiftSigned, CodeGen::LongWordType()},
        {"sall", BinaryOper::LeftShiftUnsigned, CodeGen::LongWordType()},
        {"sarl", BinaryOper::RightShiftSigned, CodeGen::LongWordType()},
        {"shrl", BinaryOper::RightShiftUnsigned, CodeGen::LongWordType()},
    };
    for (const TestDataBinaryOperator& test : tests) {
        const std::string operString = CodeGen::asmBinaryOperator(test.oper, test.type);
        EXPECT_EQ(operString, test.expected) << "Instruction mismatch for input: "
                                   << CodeGen::to_string(test.oper) << ' '
                                   << CodeGen::to_string(test.type) << '\n';
    }
}

TEST(AssemblyTests, createLabel)
{
    const std::string expected = ".Lname";
    const std::string name = "name";
    const std::string result = CodeGen::createLabel(name);
    EXPECT_EQ(result, expected) << "Expected " << ".L" << name << " for " << name;
}

TEST(AssemblyTests, asmOperand)
{
    struct TestDataOperand {
        const std::string expected;
        const std::shared_ptr<CodeGen::Operand> operand;
        TestDataOperand(std::string expected, const std::shared_ptr<CodeGen::Operand>& operand)
            : expected(std::move(expected)), operand(operand) {}
    };

    const std::vector<TestDataOperand> tests = {
        {"invalid pseudo", make_shared<PseudoOperand>(Iden(""), ReferingTo::Local, CodeGen::LongWordType(), true)},
        {"(%rip)", make_shared<DataOperand>(Iden(""), CodeGen::LongWordType(), true)},
        {".L(%rip)", make_shared<DataOperand>(Iden(""), CodeGen::DoubleType(), true)},
        {"$0", make_shared<ImmOperand>(0l, CodeGen::QuadWordType())},
        {"%rax", make_shared<RegisterOperand>(RegKind::AX, CodeGen::QuadWordType())},
        {"10(%rcx)", make_shared<MemoryOperand>(RegKind::CX, 10, CodeGen::QuadWordType())},
        {"(%rcx)", make_shared<MemoryOperand>(RegKind::CX, 0, CodeGen::QuadWordType())},
    };
    for (const TestDataOperand& test : tests) {
        const std::string operString = CodeGen::asmOperand(test.operand);
        EXPECT_EQ(operString, test.expected);
    }
}

TEST(AssemblyTests, asmFormatLabel)
{
    const std::string expected = "name:\n";
    const std::string name = "name";
    const std::string result = CodeGen::asmFormatLabel(name);
    EXPECT_EQ(result, expected);
}