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
        {"addb", AsmType::Byte},
        {"addl", AsmType::LongWord},
        {"addq", AsmType::QuadWord},
        {"addsd",AsmType::Double},
        {"add not set addType", AsmType::Word},
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
        {"%rbp", CodeGen::AsmType::Double, RegKind::BP},

        {"%xmm0", CodeGen::AsmType::Double, RegKind::XMM0},
        {"%xmm1", CodeGen::AsmType::Double, RegKind::XMM1},
        {"%xmm2", CodeGen::AsmType::Double, RegKind::XMM2},
        {"%xmm3", CodeGen::AsmType::Double, RegKind::XMM3},
        {"%xmm4", CodeGen::AsmType::Double, RegKind::XMM4},
        {"%xmm5", CodeGen::AsmType::Double, RegKind::XMM5},
        {"%xmm6", CodeGen::AsmType::Double, RegKind::XMM6},
        {"%xmm7", CodeGen::AsmType::Double, RegKind::XMM7},
        {"%xmm14", CodeGen::AsmType::Double, RegKind::XMM14},
        {"%xmm15", CodeGen::AsmType::Double, RegKind::XMM15},

        {"%al", CodeGen::AsmType::Byte, RegKind::AX},
        {"%ax", CodeGen::AsmType::Word, RegKind::AX},
        {"%eax", CodeGen::AsmType::LongWord, RegKind::AX},
        {"%rax", CodeGen::AsmType::QuadWord, RegKind::AX},

        {"%cl", CodeGen::AsmType::Byte, RegKind::CX},
        {"%cx", CodeGen::AsmType::Word, RegKind::CX},
        {"%ecx", CodeGen::AsmType::LongWord, RegKind::CX},
        {"%rcx", CodeGen::AsmType::QuadWord, RegKind::CX},

        {"%dl", CodeGen::AsmType::Byte, RegKind::DX},
        {"%dx", CodeGen::AsmType::Word, RegKind::DX},
        {"%edx", CodeGen::AsmType::LongWord, RegKind::DX},
        {"%rdx", CodeGen::AsmType::QuadWord, RegKind::DX},

        {"%dil", CodeGen::AsmType::Byte, RegKind::DI},
        {"%di", CodeGen::AsmType::Word, RegKind::DI},
        {"%edi", CodeGen::AsmType::LongWord, RegKind::DI},
        {"%rdi", CodeGen::AsmType::QuadWord, RegKind::DI},

        {"%sil", CodeGen::AsmType::Byte, RegKind::SI},
        {"%si", CodeGen::AsmType::Word, RegKind::SI},
        {"%esi", CodeGen::AsmType::LongWord, RegKind::SI},
        {"%rsi", CodeGen::AsmType::QuadWord, RegKind::SI},
        {"invalid_size", CodeGen::AsmType::Double, RegKind::SI},

        {"%r8b", CodeGen::AsmType::Byte, RegKind::R8},
        {"%r8w", CodeGen::AsmType::Word, RegKind::R8},
        {"%r8d", CodeGen::AsmType::LongWord, RegKind::R8},
        {"%r8", CodeGen::AsmType::QuadWord, RegKind::R8},

        {"%r9b", CodeGen::AsmType::Byte, RegKind::R9},
        {"%r9w", CodeGen::AsmType::Word, RegKind::R9},
        {"%r9d", CodeGen::AsmType::LongWord, RegKind::R9},
        {"%r9", CodeGen::AsmType::QuadWord, RegKind::R9},

        {"%r10b", CodeGen::AsmType::Byte, RegKind::R10},
        {"%r10w", CodeGen::AsmType::Word, RegKind::R10},
        {"%r10d", CodeGen::AsmType::LongWord, RegKind::R10},
        {"%r10", CodeGen::AsmType::QuadWord, RegKind::R10},

        {"%r11b", CodeGen::AsmType::Byte, RegKind::R11},
        {"%r11w", CodeGen::AsmType::Word, RegKind::R11},
        {"%r11d", CodeGen::AsmType::LongWord, RegKind::R11},
        {"%r11", CodeGen::AsmType::QuadWord, RegKind::R11},

        {"%rsp", CodeGen::AsmType::Byte, RegKind::SP},
        {"%rsp", CodeGen::AsmType::Word, RegKind::SP},
        {"%rsp", CodeGen::AsmType::LongWord, RegKind::SP},
        {"%rsp", CodeGen::AsmType::QuadWord, RegKind::SP},
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
        const std::string operString = CodeGen::asmUnaryOperator(test.oper, CodeGen::AsmType::LongWord);
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
        {"xorpd", BinaryOper::BitwiseXor, CodeGen::AsmType::Double},
        {"mulsd", BinaryOper::Mul, CodeGen::AsmType::Double},
        {"divsd", BinaryOper::DivDouble, CodeGen::AsmType::Double},
        {"imull", BinaryOper::Mul, CodeGen::AsmType::LongWord},
        {"addl", BinaryOper::Add, CodeGen::AsmType::LongWord},
        {"subl", BinaryOper::Sub, CodeGen::AsmType::LongWord},
        {"andl", BinaryOper::BitwiseAnd, CodeGen::AsmType::LongWord},
        {"orl", BinaryOper::BitwiseOr, CodeGen::AsmType::LongWord},
        {"xorl", BinaryOper::BitwiseXor, CodeGen::AsmType::LongWord},
        {"shll", BinaryOper::LeftShiftSigned, CodeGen::AsmType::LongWord},
        {"sall", BinaryOper::LeftShiftUnsigned, CodeGen::AsmType::LongWord},
        {"sarl", BinaryOper::RightShiftSigned, CodeGen::AsmType::LongWord},
        {"shrl", BinaryOper::RightShiftUnsigned, CodeGen::AsmType::LongWord},
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
        {"invalid pseudo", make_shared<PseudoOperand>(Iden(""), ReferingTo::Local, CodeGen::AsmType::LongWord, true)},
        {"(%rip)", make_shared<DataOperand>(Iden(""), CodeGen::AsmType::LongWord, true)},
        {".L(%rip)", make_shared<DataOperand>(Iden(""), CodeGen::AsmType::Double, true)},
        {"$0", make_shared<ImmOperand>(0l, CodeGen::AsmType::QuadWord)},
        {"%rax", make_shared<RegisterOperand>(RegKind::AX, CodeGen::AsmType::QuadWord)},
        {"10(%rcx)", make_shared<MemoryOperand>(RegKind::CX, 10, CodeGen::AsmType::QuadWord)},
        {"(%rcx)", make_shared<MemoryOperand>(RegKind::CX, 0, CodeGen::AsmType::QuadWord)},
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