#include <gtest/gtest.h>

#include "Token.hpp"
#include "Parsing/Parser.hpp"

using TokenType = Lexing::Token::Type;

TEST(ParserOperatorsTest, unaryOperator)
{
    using UnaryOper = Parsing::UnaryExpr::Operator;
    struct TestcaseUnary {
        const TokenType tokenType;
        const UnaryOper unaryOper;
        TestcaseUnary(const TokenType tokenType, const UnaryOper unaryOper)
            : tokenType(tokenType), unaryOper(unaryOper) {  }
    };
    const std::vector<TestcaseUnary> testcases{
            {TokenType::Plus,            UnaryOper::Plus},
            {TokenType::Minus,           UnaryOper::Negate},
            {TokenType::Tilde,           UnaryOper::Complement},
            {TokenType::ExclamationMark, UnaryOper::Not},
            {TokenType::Increment,       UnaryOper::PrefixIncrement},
            {TokenType::Decrement,       UnaryOper::PrefixDecrement},
    };
    for (const TestcaseUnary& testcase : testcases) {
        EXPECT_EQ(Parsing::Operators::unaryOperator(testcase.tokenType), testcase.unaryOper);
    }
}

TEST(ParserOperatorsTest, binaryOperator)
{
    using BinaryOper = Parsing::BinaryExpr::Operator;
    struct TestcaseBinary {
        const TokenType tokenType;
        const BinaryOper binaryOper;
        TestcaseBinary(const TokenType tokenType, const BinaryOper unaryOper)
            : tokenType(tokenType), binaryOper(unaryOper) {  }
    };
    const std::vector<TestcaseBinary> testcases{
                {TokenType::Plus,            BinaryOper::Add},
                {TokenType::Minus,           BinaryOper::Subtract},
                {TokenType::Asterisk,        BinaryOper::Multiply},
                {TokenType::ForwardSlash,    BinaryOper::Divide},
                {TokenType::Percent,         BinaryOper::Modulo},

                {TokenType::Ampersand,       BinaryOper::BitwiseAnd},
                {TokenType::Pipe,            BinaryOper::BitwiseOr},
                {TokenType::Circumflex,      BinaryOper::BitwiseXor},
                {TokenType::LeftShift,       BinaryOper::LeftShift},
                {TokenType::RightShift,      BinaryOper::RightShift},

                {TokenType::LogicalAnd,      BinaryOper::And},
                {TokenType::LogicalOr,       BinaryOper::Or},
                {TokenType::LogicalEqual,    BinaryOper::Equal},
                {TokenType::LogicalNotEqual, BinaryOper::NotEqual},
                {TokenType::Greater,         BinaryOper::GreaterThan},
                {TokenType::GreaterOrEqual,  BinaryOper::GreaterOrEqual},
                {TokenType::Less,            BinaryOper::LessThan},
                {TokenType::LessOrEqual,     BinaryOper::LessOrEqual},
        };
    for (const TestcaseBinary& testcase : testcases) {
        EXPECT_EQ(Parsing::Operators::binaryOperator(testcase.tokenType), testcase.binaryOper);
    }
}

TEST(ParserOperatorsTest, assignOperator)
{
    using AssignOper = Parsing::AssignmentExpr::Operator;
    struct TestcaseBinary {
        const TokenType tokenType;
        const AssignOper assignOper;
        TestcaseBinary(const TokenType tokenType, const AssignOper unaryOper)
            : tokenType(tokenType), assignOper(unaryOper) {  }
    };
    const std::vector<TestcaseBinary> testcases{
                    {TokenType::Equal,              AssignOper::Assign},
                    {TokenType::PlusAssign,         AssignOper::PlusAssign},
                    {TokenType::MinusAssign,        AssignOper::MinusAssign},
                    {TokenType::MultiplyAssign,     AssignOper::MultiplyAssign},
                    {TokenType::DivideAssign,       AssignOper::DivideAssign},
                    {TokenType::ModuloAssign,       AssignOper::ModuloAssign},
                    {TokenType::BitwiseAndAssign,   AssignOper::BitwiseAndAssign},
                    {TokenType::BitwiseOrAssign,    AssignOper::BitwiseOrAssign},
                    {TokenType::BitwiseXorAssign,   AssignOper::BitwiseXorAssign},
                    {TokenType::LeftShiftAssign,    AssignOper::LeftShiftAssign},
                    {TokenType::RightShiftAssign,   AssignOper::RightShiftAssign},
            };
    for (const TestcaseBinary& testcase : testcases) {
        EXPECT_EQ(Parsing::Operators::assignOperator(testcase.tokenType), testcase.assignOper);
    }
}

TEST(ParserOperatorsTest, isUnaryOperator)
{
    constexpr std::array tokens{
        TokenType::Minus,
        TokenType::Plus,
        TokenType::Tilde,
        TokenType::ExclamationMark,
        TokenType::Increment,
        TokenType::Decrement,
        TokenType::Ampersand,
        TokenType::Asterisk,
    };
    for (const TokenType& tokenType : tokens) {
        EXPECT_TRUE(Parsing::Operators::isUnaryOperator(tokenType));
    }
}

TEST(ParserOperatorsTest, isBinaryOperator)
{
    constexpr std::array tokens{
        TokenType::Plus,
        TokenType::Minus,
        TokenType::ForwardSlash,
        TokenType::Percent,
        TokenType::Asterisk,
        TokenType::LeftShift,
        TokenType::RightShift,
        TokenType::Ampersand,
        TokenType::Pipe,
        TokenType::Circumflex,

        TokenType::LogicalAnd,
        TokenType::LogicalOr,
        TokenType::LogicalEqual,
        TokenType::LogicalNotEqual,
        TokenType::Greater,
        TokenType::Less,
        TokenType::LessOrEqual,
        TokenType::GreaterOrEqual,
    };
    for (const TokenType& tokenType : tokens) {
        EXPECT_TRUE(Parsing::Operators::isBinaryOperator(tokenType));
    }
}

TEST(ParserOperatorsTest, unaryPrecedenceLevels)
{
    using UnaryOper = Parsing::UnaryExpr::Operator;
    struct TestcaseUnaryPrecedenceLevel {
        const UnaryOper unaryType;
        const i32 level;
        TestcaseUnaryPrecedenceLevel(const UnaryOper unaryType, const i32 level)
            : unaryType(unaryType), level(level) {  }
    };
    const std::vector<TestcaseUnaryPrecedenceLevel> testCases{
        {UnaryOper::PostFixIncrement, 1},
        {UnaryOper::PostFixDecrement, 1},

        {UnaryOper::PrefixIncrement, 2},
        {UnaryOper::PrefixDecrement, 2},
        {UnaryOper::Complement, 2},
        {UnaryOper::Negate, 2},
        {UnaryOper::Not, 2},
    };
    for (const TestcaseUnaryPrecedenceLevel& testCase : testCases) {
        EXPECT_EQ(Parsing::Operators::getPrecedenceLevel(testCase.unaryType), testCase.level);
    }
}

TEST(ParserOperatorsTest, binaryPrecedenceLevels)
{
    using BinaryOper = Parsing::BinaryExpr::Operator;
    struct TestcaseUnaryPrecedenceLevel {
        const BinaryOper binaryType;
        const i32 level;
        TestcaseUnaryPrecedenceLevel(const BinaryOper binaryType, const i32 level)
            : binaryType(binaryType), level(level) {  }
    };
    const std::vector<TestcaseUnaryPrecedenceLevel> testCases{
            {BinaryOper::Multiply, 3},
            {BinaryOper::Divide, 3},
            {BinaryOper::Modulo, 3},

            {BinaryOper::Add, 4},
            {BinaryOper::Subtract, 4},

            {BinaryOper::LeftShift, 5},
            {BinaryOper::RightShift, 5},

            {BinaryOper::LessThan, 6},
            {BinaryOper::LessOrEqual, 6},
            {BinaryOper::GreaterThan, 6},
            {BinaryOper::GreaterOrEqual, 6},

            {BinaryOper::Equal, 7},
            {BinaryOper::NotEqual, 7},
            {BinaryOper::NotEqual, 7},

            {BinaryOper::BitwiseAnd, 8},
            {BinaryOper::BitwiseXor, 9},
            {BinaryOper::BitwiseOr, 10},
            {BinaryOper::And, 11},
            {BinaryOper::Or, 12},
        };
    for (const TestcaseUnaryPrecedenceLevel& testCase : testCases) {
        EXPECT_EQ(Parsing::Operators::getPrecedenceLevel(testCase.binaryType), testCase.level);
    }
}

TEST(ParserOperatorsTest, getBinaryOperatorFromAssign)
{
    using Assign = Parsing::AssignmentExpr::Operator;
    using Binary = Parsing::BinaryExpr::Operator;
    struct TestcaseBinaryFromAssign {
        const Assign assign;
        const Binary binary;
        TestcaseBinaryFromAssign(const Assign assign, const Binary binary)
            : assign(assign), binary(binary) {  }
    };
    const std::vector<TestcaseBinaryFromAssign> testCases{
                {Assign::PlusAssign, Binary::Add},
                {Assign::MinusAssign, Binary::Subtract},
                {Assign::MultiplyAssign, Binary::Multiply},
                {Assign::DivideAssign, Binary::Divide},
                {Assign::ModuloAssign, Binary::Modulo},
                {Assign::BitwiseAndAssign, Binary::BitwiseAnd},
                {Assign::BitwiseOrAssign, Binary::BitwiseOr},
                {Assign::BitwiseXorAssign, Binary::BitwiseXor},
                {Assign::LeftShiftAssign, Binary::LeftShift},
                {Assign::RightShiftAssign, Binary::RightShift},
            };
    for (const TestcaseBinaryFromAssign& testCase : testCases) {
        EXPECT_EQ(Parsing::Operators::getBinaryOperator(testCase.assign), testCase.binary);
    }
}