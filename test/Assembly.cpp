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
        {"addl", AsmType::LongWord},
        {"addq", AsmType::QuadWord},
        {"addsd", AsmType::Double},
        {"add not set addType", AsmType::Byte},
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
        {"%rbp", AsmType::Double, RegKind::BP},

        {"%xmm0", AsmType::Double, RegKind::XMM0},
        {"%xmm1", AsmType::Double, RegKind::XMM1},
        {"%xmm2", AsmType::Double, RegKind::XMM2},
        {"%xmm3", AsmType::Double, RegKind::XMM3},
        {"%xmm4", AsmType::Double, RegKind::XMM4},
        {"%xmm5", AsmType::Double, RegKind::XMM5},
        {"%xmm6", AsmType::Double, RegKind::XMM6},
        {"%xmm7", AsmType::Double, RegKind::XMM7},
        {"%xmm14", AsmType::Double, RegKind::XMM14},
        {"%xmm15", AsmType::Double, RegKind::XMM15},

        {"%al", AsmType::Byte, RegKind::AX},
        {"%ax", AsmType::Word, RegKind::AX},
        {"%eax", AsmType::LongWord, RegKind::AX},
        {"%rax", AsmType::QuadWord, RegKind::AX},

        {"%cl", AsmType::Byte, RegKind::CX},
        {"%cx", AsmType::Word, RegKind::CX},
        {"%ecx", AsmType::LongWord, RegKind::CX},
        {"%rcx", AsmType::QuadWord, RegKind::CX},

        {"%dl", AsmType::Byte, RegKind::DX},
        {"%dx", AsmType::Word, RegKind::DX},
        {"%edx", AsmType::LongWord, RegKind::DX},
        {"%rdx", AsmType::QuadWord, RegKind::DX},

        {"%dil", AsmType::Byte, RegKind::DI},
        {"%di", AsmType::Word, RegKind::DI},
        {"%edi", AsmType::LongWord, RegKind::DI},
        {"%rdi", AsmType::QuadWord, RegKind::DI},

        {"%sil", AsmType::Byte, RegKind::SI},
        {"%si", AsmType::Word, RegKind::SI},
        {"%esi", AsmType::LongWord, RegKind::SI},
        {"%rsi", AsmType::QuadWord, RegKind::SI},
        {"invalid_size", AsmType::Double, RegKind::SI},

        {"%r8b", AsmType::Byte, RegKind::R8},
        {"%r8w", AsmType::Word, RegKind::R8},
        {"%r8d", AsmType::LongWord, RegKind::R8},
        {"%r8", AsmType::QuadWord, RegKind::R8},

        {"%r9b", AsmType::Byte, RegKind::R9},
        {"%r9w", AsmType::Word, RegKind::R9},
        {"%r9d", AsmType::LongWord, RegKind::R9},
        {"%r9", AsmType::QuadWord, RegKind::R9},

        {"%r10b", AsmType::Byte, RegKind::R10},
        {"%r10w", AsmType::Word, RegKind::R10},
        {"%r10d", AsmType::LongWord, RegKind::R10},
        {"%r10", AsmType::QuadWord, RegKind::R10},

        {"%r11b", AsmType::Byte, RegKind::R11},
        {"%r11w", AsmType::Word, RegKind::R11},
        {"%r11d", AsmType::LongWord, RegKind::R11},
        {"%r11", AsmType::QuadWord, RegKind::R11},

        {"%rsp", AsmType::Byte, RegKind::SP},
        {"%rsp", AsmType::Word, RegKind::SP},
        {"%rsp", AsmType::LongWord, RegKind::SP},
        {"%rsp", AsmType::QuadWord, RegKind::SP},
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
        const std::string operString = CodeGen::asmUnaryOperator(test.oper, AsmType::LongWord);
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
        {"xorpd", BinaryOper::BitwiseXor, AsmType::Double},
        {"mulsd", BinaryOper::Mul, AsmType::Double},
        {"divsd", BinaryOper::DivDouble, AsmType::Double},
        {"imull", BinaryOper::Mul, AsmType::LongWord},
        {"addl", BinaryOper::Add, AsmType::LongWord},
        {"subl", BinaryOper::Sub, AsmType::LongWord},
        {"andl", BinaryOper::BitwiseAnd, AsmType::LongWord},
        {"orl", BinaryOper::BitwiseOr, AsmType::LongWord},
        {"xorl", BinaryOper::BitwiseXor, AsmType::LongWord},
        {"shll", BinaryOper::LeftShiftSigned, AsmType::LongWord},
        {"sall", BinaryOper::LeftShiftUnsigned, AsmType::LongWord},
        {"sarl", BinaryOper::RightShiftSigned, AsmType::LongWord},
        {"shrl", BinaryOper::RightShiftUnsigned, AsmType::LongWord},
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
        {"invalid pseudo", make_shared<PseudoOperand>(Iden(""), ReferingTo::Local, AsmType::LongWord, true)},
        {"(%rip)", make_shared<DataOperand>(Iden(""), AsmType::LongWord, true)},
        {".L(%rip)", make_shared<DataOperand>(Iden(""), AsmType::Double, true)},
        {"$0", make_shared<ImmOperand>(0l, AsmType::QuadWord)},
        {"%rax", make_shared<RegisterOperand>(RegKind::AX, AsmType::QuadWord)},
        {"10(%rcx)", make_shared<MemoryOperand>(RegKind::CX, 10, AsmType::QuadWord)},
        {"(%rcx)", make_shared<MemoryOperand>(RegKind::CX, 0, AsmType::QuadWord)},
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