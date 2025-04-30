#include "Lexing/Lexer.hpp"
#include "Lexing/Token.hpp"
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

    std::vector<Lexing::Token> runLexerTest(const std::string& input) {
        Lexing::Lexer lexer(input);
        std::vector<Lexing::Token> tokens;
        lexer.getLexemes(tokens);
        return tokens;
    }
    void TestSingleTokenLexing(const std::string& input, Lexing::Token::Type expectedType)
    {
        constexpr i32 endOfFile = 1;
        const auto tokens = runLexerTest(input);
        ASSERT_EQ(tokens.size(), 1 + endOfFile) << "Expected exactly one token for input: " << input;
        const Lexing::Token expected{
            1, 1, expectedType, input
        };
        EXPECT_EQ(tokens[0], expected) << "Token mismatch for input: " << input;
    }
}

TEST(LexerTests, GetTokens) {
    auto tokens = runLexerTest(BASIC_PROGRAM);

    ASSERT_EQ(tokens.size(), 11);

    const Lexing::Token expected[] = {
        {1, 1, Lexing::Token::Type::IntKeyword, "int"},
        {1, 5, Lexing::Token::Type::Identifier, "main"},
        {1, 9, Lexing::Token::Type::OpenParen, "("},
        {1, 10, Lexing::Token::Type::Void, "void"},
        {1, 14, Lexing::Token::Type::CloseParen, ")"},
        {1, 16, Lexing::Token::Type::OpenBrace, "{"},
        {3, 5, Lexing::Token::Type::Return, "return"},
        {3, 12, Lexing::Token::Type::Integer, "100"},
        {3, 15, Lexing::Token::Type::Semicolon, ";"},
        {4, 1, Lexing::Token::Type::CloseBrace, "}"},
        {4, 2, Lexing::Token::Type::EndOfFile, ""}
    };

    for(size_t i = 0; i < tokens.size(); ++i) {
        EXPECT_EQ(tokens[i], expected[i]) << "Mismatch at token " << i;
    }
}

TEST(LexerTests, Percent)
{
    TestSingleTokenLexing("%", TokenType::Percent);
}

TEST(LexerTests, Plus)
{
    TestSingleTokenLexing("+", TokenType::Plus);
}

TEST(LexerTests, Asterisk)
{
    TestSingleTokenLexing("*", TokenType::Asterisk);
}

TEST(LexerTests, ForwardSlash)
{
    TestSingleTokenLexing("/", TokenType::ForwardSlash);
}

TEST(LexerTests, Pipe)
{
    TestSingleTokenLexing("|", TokenType::Pipe);
}

TEST(LexerTests, Ampersand)
{
    TestSingleTokenLexing("&", TokenType::Ampersand);
}

TEST(LexerTests, LeftShift)
{
    TestSingleTokenLexing("<<", TokenType::LeftShift);
}

TEST(LexerTests, RightShift)
{
    TestSingleTokenLexing(">>", TokenType::RightShift);
}

TEST(LexerTests, Circumflex)
{
    TestSingleTokenLexing("^", TokenType::Circumflex);
}

TEST(LexerTests, LogicalAnd)
{
    TestSingleTokenLexing("&&", TokenType::LogicalAnd);
}

TEST(LexerTests, LogicalOr)
{
    TestSingleTokenLexing("||", TokenType::LogicalOr);
}

TEST(LexerTests, LogicalNotEqual)
{
    TestSingleTokenLexing("!=", TokenType::LogicalNotEqual);
}

TEST(LexerTests, LogicalEqual)
{
    TestSingleTokenLexing("==", TokenType::LogicalEqual);
}

TEST(LexerTests, Less)
{
    TestSingleTokenLexing("<", TokenType::Less);
}

TEST(LexerTests, LessEqual)
{
    TestSingleTokenLexing("<=", TokenType::LessOrEqual);
}

TEST(LexerTests, Greater)
{
    TestSingleTokenLexing(">", TokenType::Greater);
}

TEST(LexerTests, GreaterEqual)
{
    TestSingleTokenLexing(">=", TokenType::GreaterOrEqual);
}

TEST(LexerTests, ExclamationMark)
{
    TestSingleTokenLexing("!", TokenType::ExclamationMark);
}

TEST(LexerTests, MultiLineComment)
{
    const auto tokens = runLexerTest(MULTILINE_COMMENT_PROGRAM);
    EXPECT_TRUE(tokens.size() == 1) << " " << tokens.size();
}

TEST(LexerTests, InvalidInput) {
    Lexing::Lexer lexer("\\");
    std::vector<Lexing::Token> tokens;
    ASSERT_NE(lexer.getLexemes(tokens), 0);
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0].m_type, Lexing::Token::Type::Invalid);
}