#include <Lexer.hpp>
#include "Frontend/Lexing/Lexer.hpp"
#include "Frontend/Lexing/Token.hpp"
#include <gtest/gtest.h>

namespace {

using TokenType = Lexing::Token::Type;

const std::string BASIC_PROGRAM =
    "int main(void) {\n"
    "// int main return\n"
    "    return 100;\n"
    "}";

const std::string MULTILINE_COMMENT_PROGRAM =
    "/*int main(void) {\n"
    "// int main return\n"
    "    return 100;\n"
    "}*/";

TokenStore runLexerTest(const std::string& input)
{
    TokenStore tokenStore;
    Lexing::Lexer lexer(input, tokenStore);
    lexer.getLexemes();
    return tokenStore;
}

void TestSingleTokenLexing(const std::string& input, const TokenType expectedType)
{
    constexpr i32 endOfFile = 1;
    const TokenStore tokenStore = runLexerTest(input);
    ASSERT_EQ(tokenStore.size(), 1 + endOfFile) << "Expected exactly one token for input: " << input;
    const Lexing::Token expected{
        1, 1, expectedType, input
    };
    EXPECT_EQ(tokenStore.getToken(0), expected) << "Token mismatch for input: " << input;
}

void TestSingleTokenLexing(const TestCaseLexer& testCase)
{
    constexpr i32 endOfFile = 1;
    const TokenStore tokenStore = runLexerTest(testCase.input);
    ASSERT_EQ(tokenStore.size(), 1 + endOfFile) << "Expected exactly one token for input: " << testCase.input;
    const Lexing::Token expected{
        1, 1, testCase.expectedType, ""
    };
    EXPECT_EQ(tokenStore.getToken(0), expected) << "Token mismatch for input: " << testCase.input;
}

}

TEST(LexerTests, GetTokens)
{
    const TokenStore tokenStore = runLexerTest(BASIC_PROGRAM);

    EXPECT_EQ(tokenStore.size(), 11);

    const Lexing::Token expected[] = {
        {1, 1, TokenType::IntKeyword, ""},
        {1, 5, TokenType::Identifier, "main"},
        {1, 9, TokenType::OpenParen, ""},
        {1, 10, TokenType::VoidKeyword, ""},
        {1, 14, TokenType::CloseParen, ""},
        {1, 16, TokenType::OpenBrace, ""},
        {3, 5, TokenType::Return, ""},
        {3, 12, TokenType::IntegerLiteral, "100"},
        {3, 15, TokenType::Semicolon, ""},
        {4, 1, TokenType::CloseBrace, ""},
        {4, 2, TokenType::EndOfFile, ""}
    };

    for(size_t i = 0; i < tokenStore.size(); ++i)
        EXPECT_EQ(tokenStore.getToken(i), expected[i]) << "Mismatch at token " << i;
}

TEST(LexerTests, Tokens)
{
    const std::vector<TestCaseLexer> testCases = {
        {"return", TokenType::Return},
        {"int", TokenType::IntKeyword},
        {"void", TokenType::VoidKeyword},
        {"if", TokenType::If},
        {"else", TokenType::Else},
        {"do", TokenType::Do},
        {"while", TokenType::While},
        {"for", TokenType::For},
        {"break", TokenType::Break},
        {"continue", TokenType::Continue},
        {"goto", TokenType::Goto},
        {"switch", TokenType::Switch},
        {"case", TokenType::Case},
        {"default", TokenType::Default},
        {"static", TokenType::Static},
        {"extern", TokenType::Extern},
        {"signed", TokenType::Signed},
        {"unsigned", TokenType::Unsigned},
        {"double", TokenType::DoubleKeyword},
        {"struct", TokenType::StructKeyword},
        {"%", TokenType::Percent},
        {"+", TokenType::Plus},
        {"*", TokenType::Asterisk},
        {"/", TokenType::ForwardSlash},
        {"|", TokenType::Pipe},
        {"&", TokenType::Ampersand},
        {"<<", TokenType::LeftShift},
        {">>", TokenType::RightShift},
        {"^", TokenType::Circumflex},
        {"--", TokenType::Decrement},
        {"++", TokenType::Increment},
        {"&&", TokenType::LogicalAnd},
        {"||", TokenType::LogicalOr},
        {"!=", TokenType::LogicalNotEqual},
        {"==", TokenType::LogicalEqual},
        {"<", TokenType::Less},
        {"<=", TokenType::LessOrEqual},
        {">", TokenType::Greater},
        {">=", TokenType::GreaterOrEqual},
        {"!", TokenType::ExclamationMark},
        {"=", TokenType::Equal},
        {"+=", TokenType::PlusAssign},
        {"-=", TokenType::MinusAssign},
        {"*=", TokenType::MultiplyAssign},
        {"/=", TokenType::DivideAssign},
        {"%=", TokenType::ModuloAssign},
        {"&=", TokenType::BitwiseAndAssign},
        {"|=", TokenType::BitwiseOrAssign},
        {"^=", TokenType::BitwiseXorAssign},
        {"<<=", TokenType::LeftShiftAssign},
        {">>=", TokenType::RightShiftAssign},
        {"?", TokenType::QuestionMark},
        {",", TokenType::Comma},
        {":", TokenType::Colon},
        {"[", TokenType::OpenSqBracket},
        {"]", TokenType::CloseSqBracket},
        {"{", TokenType::OpenBrace},
        {"}", TokenType::CloseBrace},
        {"(", TokenType::OpenParen},
        {")", TokenType::CloseParen},
        {".", TokenType::Period},
        {"->", TokenType::Arrow},
    };
    for (const TestCaseLexer& testCase : testCases)
        TestSingleTokenLexing(testCase);
}

TEST(LexerTests, IntegerLiteral)
{
    TestSingleTokenLexing("34875", TokenType::IntegerLiteral);
}

TEST(LexerTests, LongLiteralOverflowFromInteger)
{
    TestSingleTokenLexing("2147483648", TokenType::LongLiteral);
}

TEST(LexerTests, LongLiteralUpperCase)
{
    TestSingleTokenLexing("2309L", TokenType::LongLiteral);
}

TEST(LexerTests, LongLiteralLowerCase)
{
    TestSingleTokenLexing("2309l", TokenType::LongLiteral);
}

TEST(LexerTests, UnsignedIntegerLiteral)
{
    TestSingleTokenLexing("2147483648u", TokenType::UnsignedIntegerLiteral);
}

TEST(LexerTests, UnsignedIntegerLiteralLowerCase)
{
    TestSingleTokenLexing("214u", TokenType::UnsignedIntegerLiteral);
}

TEST(LexerTests, UnsignedIntegerLiteralUpperCase)
{
    TestSingleTokenLexing("2147U", TokenType::UnsignedIntegerLiteral);
}

TEST(LexerTests, UnsignedLongLiteralOverflowFromInteger)
{
    TestSingleTokenLexing("4294967296u", TokenType::UnsignedLongLiteral);
}

TEST(LexerTests, UnsignedLongLiteralUpperCase)
{
    TestSingleTokenLexing("2147UL", TokenType::UnsignedLongLiteral);
}

TEST(LexerTests, UnsignedLongLiteralLowerCase)
{
    TestSingleTokenLexing("2147ul", TokenType::UnsignedLongLiteral);
}

TEST(LexerTests, MultiLineComment)
{
    const auto tokens = runLexerTest(MULTILINE_COMMENT_PROGRAM);
    EXPECT_TRUE(tokens.size() == 1) << " " << tokens.size();
}