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
        {"addb", asmByte},
        {"addl", asmLongWord},
        {"addq", asmQuadWord},
        {"addsd",asmDouble},
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
        {"%rbp", CodeGen::asmDouble, RegKind::BP},

        {"%xmm0", CodeGen::asmDouble, RegKind::XMM0},
        {"%xmm1", CodeGen::asmDouble, RegKind::XMM1},
        {"%xmm2", CodeGen::asmDouble, RegKind::XMM2},
        {"%xmm3", CodeGen::asmDouble, RegKind::XMM3},
        {"%xmm4", CodeGen::asmDouble, RegKind::XMM4},
        {"%xmm5", CodeGen::asmDouble, RegKind::XMM5},
        {"%xmm6", CodeGen::asmDouble, RegKind::XMM6},
        {"%xmm7", CodeGen::asmDouble, RegKind::XMM7},
        {"%xmm14", CodeGen::asmDouble, RegKind::XMM14},
        {"%xmm15", CodeGen::asmDouble, RegKind::XMM15},

        {"%al", CodeGen::asmByte, RegKind::AX},
        {"%ax", CodeGen::AsmType::Word, RegKind::AX},
        {"%eax", CodeGen::asmLongWord, RegKind::AX},
        {"%rax", CodeGen::asmQuadWord, RegKind::AX},

        {"%cl", CodeGen::asmByte, RegKind::CX},
        {"%cx", CodeGen::AsmType::Word, RegKind::CX},
        {"%ecx", CodeGen::asmLongWord, RegKind::CX},
        {"%rcx", CodeGen::asmQuadWord, RegKind::CX},

        {"%dl", CodeGen::asmByte, RegKind::DX},
        {"%dx", CodeGen::AsmType::Word, RegKind::DX},
        {"%edx", CodeGen::asmLongWord, RegKind::DX},
        {"%rdx", CodeGen::asmQuadWord, RegKind::DX},

        {"%dil", CodeGen::asmByte, RegKind::DI},
        {"%di", CodeGen::AsmType::Word, RegKind::DI},
        {"%edi", CodeGen::asmLongWord, RegKind::DI},
        {"%rdi", CodeGen::asmQuadWord, RegKind::DI},

        {"%sil", CodeGen::asmByte, RegKind::SI},
        {"%si", CodeGen::AsmType::Word, RegKind::SI},
        {"%esi", CodeGen::asmLongWord, RegKind::SI},
        {"%rsi", CodeGen::asmQuadWord, RegKind::SI},
        {"invalid_size", CodeGen::asmDouble, RegKind::SI},

        {"%r8b", CodeGen::asmByte, RegKind::R8},
        {"%r8w", CodeGen::AsmType::Word, RegKind::R8},
        {"%r8d", CodeGen::asmLongWord, RegKind::R8},
        {"%r8", CodeGen::asmQuadWord, RegKind::R8},

        {"%r9b", CodeGen::asmByte, RegKind::R9},
        {"%r9w", CodeGen::AsmType::Word, RegKind::R9},
        {"%r9d", CodeGen::asmLongWord, RegKind::R9},
        {"%r9", CodeGen::asmQuadWord, RegKind::R9},

        {"%r10b", CodeGen::asmByte, RegKind::R10},
        {"%r10w", CodeGen::AsmType::Word, RegKind::R10},
        {"%r10d", CodeGen::asmLongWord, RegKind::R10},
        {"%r10", CodeGen::asmQuadWord, RegKind::R10},

        {"%r11b", CodeGen::asmByte, RegKind::R11},
        {"%r11w", CodeGen::AsmType::Word, RegKind::R11},
        {"%r11d", CodeGen::asmLongWord, RegKind::R11},
        {"%r11", CodeGen::asmQuadWord, RegKind::R11},

        {"%rsp", CodeGen::asmByte, RegKind::SP},
        {"%rsp", CodeGen::AsmType::Word, RegKind::SP},
        {"%rsp", CodeGen::asmLongWord, RegKind::SP},
        {"%rsp", CodeGen::asmQuadWord, RegKind::SP},
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
        const std::string operString = CodeGen::asmUnaryOperator(test.oper, CodeGen::asmLongWord);
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
        {"xorpd", BinaryOper::BitwiseXor, CodeGen::asmDouble},
        {"mulsd", BinaryOper::Mul, CodeGen::asmDouble},
        {"divsd", BinaryOper::DivDouble, CodeGen::asmDouble},
        {"imull", BinaryOper::Mul, CodeGen::asmLongWord},
        {"addl", BinaryOper::Add, CodeGen::asmLongWord},
        {"subl", BinaryOper::Sub, CodeGen::asmLongWord},
        {"andl", BinaryOper::BitwiseAnd, CodeGen::asmLongWord},
        {"orl", BinaryOper::BitwiseOr, CodeGen::asmLongWord},
        {"xorl", BinaryOper::BitwiseXor, CodeGen::asmLongWord},
        {"shll", BinaryOper::LeftShiftSigned, CodeGen::asmLongWord},
        {"sall", BinaryOper::LeftShiftUnsigned, CodeGen::asmLongWord},
        {"sarl", BinaryOper::RightShiftSigned, CodeGen::asmLongWord},
        {"shrl", BinaryOper::RightShiftUnsigned, CodeGen::asmLongWord},
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
        {"invalid pseudo", make_shared<PseudoOperand>(Iden(""), ReferingTo::Local, CodeGen::asmLongWord, true)},
        {"(%rip)", make_shared<DataOperand>(Iden(""), CodeGen::asmLongWord, true)},
        {".L(%rip)", make_shared<DataOperand>(Iden(""), CodeGen::asmDouble, true)},
        {"$0", make_shared<ImmOperand>(0l, CodeGen::asmQuadWord)},
        {"%rax", make_shared<RegisterOperand>(RegKind::AX, CodeGen::asmQuadWord)},
        {"10(%rcx)", make_shared<MemoryOperand>(RegKind::CX, 10, CodeGen::asmQuadWord)},
        {"(%rcx)", make_shared<MemoryOperand>(RegKind::CX, 0, CodeGen::asmQuadWord)},
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