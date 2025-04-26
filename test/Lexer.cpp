#include "Lexing/Lexer.hpp"
#include "Lexing/Token.hpp"
#include <gtest/gtest.h>

namespace {
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
    const std::string input = "%";
    const auto tokens = runLexerTest(input);
    const Lexing::Token expected[] = {
        {1, 1, Lexing::Token::Type::Percent, "%"}
    };
    EXPECT_EQ(tokens[0], expected[0]) << " Percent mismatch";
}

TEST(LexerTests, Plus)
{
    const std::string input = "+";
    const auto tokens = runLexerTest(input);
    const Lexing::Token expected[] = {
        {1, 1, Lexing::Token::Type::Plus, "+"}
    };
    EXPECT_EQ(tokens[0], expected[0]) << " Plus mismatch";
}

TEST(LexerTests, Asterisk)
{
    const std::string input = "*";
    const auto tokens = runLexerTest(input);
    const Lexing::Token expected[] = {
        {1, 1, Lexing::Token::Type::Asterisk, "*"}
    };
    EXPECT_EQ(tokens[0], expected[0]) << " Asterisk mismatch";
}

TEST(LexerTests, ForwardSlash)
{
    const std::string input = "/";
    const auto tokens = runLexerTest(input);
    const Lexing::Token expected[] = {
        {1, 1, Lexing::Token::Type::ForwardSlash, "/"}
    };
    EXPECT_EQ(tokens[0], expected[0]) << " ForwardSlash mismatch";
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