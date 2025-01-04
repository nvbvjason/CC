#pragma once

#ifndef LEXER_HPP
#define LEXER_HPP

#include "../Lexing/Lexer.hpp"
#include "../Lexing/Lexeme.hpp"

#include <gtest/gtest.h>

TEST(Lexing, getTokens)
{
    const std::string testProgram =
    "int main(void) {\n"
    "// int main return\n"
    "    return 100;\n"
    "}";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Lexeme> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    Lexing::Lexeme intKeywordToken(1, 1, Lexing::LexemeType::IntKeyword, "int");
    ASSERT_EQ(lexemes[0], intKeywordToken);
    Lexing::Lexeme mainToken(1, 5, Lexing::LexemeType::Identifier, "main");
    ASSERT_EQ(lexemes[1], mainToken);
    Lexing::Lexeme openParenToken(1, 9, Lexing::LexemeType::OpenParen, "(");
    ASSERT_EQ(lexemes[2], openParenToken);
    Lexing::Lexeme voidToken(1, 10, Lexing::LexemeType::Void, "void");
    ASSERT_EQ(lexemes[3], voidToken);
    Lexing::Lexeme closeParenToken(1, 14, Lexing::LexemeType::CloseParen, ")");
    ASSERT_EQ(lexemes[4], closeParenToken);
    Lexing::Lexeme openBraceToken(1, 16, Lexing::LexemeType::OpenBrace, "{");
    ASSERT_EQ(lexemes[5], openBraceToken);
    Lexing::Lexeme returnToken(3, 5, Lexing::LexemeType::Return, "return");
    ASSERT_EQ(lexemes[6], returnToken);
    Lexing::Lexeme intToken(3, 12, Lexing::LexemeType::Integer, "100");
    ASSERT_EQ(lexemes[7], intToken);
    Lexing::Lexeme semicolonToken(3, 15, Lexing::LexemeType::Semicolon, ";");
    ASSERT_EQ(lexemes[8], semicolonToken);
    Lexing::Lexeme closeBraceToken(4, 1, Lexing::LexemeType::CloseBrace, "}");
    ASSERT_EQ(lexemes[9], closeBraceToken);
}

TEST(Lexing, multiLineComment)
{
    const std::string testProgram =
    "/*int main(void) {\n"
    "// int main return\n"
    "    return 100;\n"
    "}*/";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Lexeme> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 0);
}

TEST(Lexing, whiteSpace)
{
    const std::string testProgram =
    "\t\r";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Lexeme> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 0);
}

TEST(Lexing, invalid)
{
    const std::string testProgram =
    "\\";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Lexeme> lexemes;
    ASSERT_NE(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 1);
    const Lexing::Lexeme invalidToken(1, 1, Lexing::LexemeType::Invalid, "\\");
    ASSERT_EQ(lexemes[0], invalidToken);
}

TEST(Lexing, invalidMultiline)
{
    const std::string testProgram =
    "/*  ";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Lexeme> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 0);
}

TEST(Lexing, testTokenToString)
{
    std::vector<Lexing::Lexeme> tokens;
    tokens.emplace_back(1, 1, Lexing::LexemeType::IntKeyword, "int");
    EXPECT_EQ(tokens[0].getTypeName(), "Int Keyword");
    tokens.emplace_back(1, 5, Lexing::LexemeType::Identifier, "main");
    EXPECT_EQ(tokens[1].getTypeName(), "Identifier");
    tokens.emplace_back(1, 9, Lexing::LexemeType::OpenParen, "(");
    EXPECT_EQ(tokens[2].getTypeName(), "Open Paren");
    tokens.emplace_back(1, 10, Lexing::LexemeType::Void, "void");
    EXPECT_EQ(tokens[3].getTypeName(), "Void");
    tokens.emplace_back(1, 14, Lexing::LexemeType::CloseParen, ")");
    EXPECT_EQ(tokens[4].getTypeName(), "Close Paren");
    tokens.emplace_back(1, 16, Lexing::LexemeType::OpenBrace, "{");
    EXPECT_EQ(tokens[5].getTypeName(), "Open Brace");
    tokens.emplace_back(3, 5, Lexing::LexemeType::Return, "return");
    EXPECT_EQ(tokens[6].getTypeName(), "Return");
    tokens.emplace_back(3, 12, Lexing::LexemeType::Integer, "100");
    EXPECT_EQ(tokens[7].getTypeName(), "Integer");
    tokens.emplace_back(3, 15, Lexing::LexemeType::Semicolon, ";");
    EXPECT_EQ(tokens[8].getTypeName(), "Semicolon");
    tokens.emplace_back(4, 1, Lexing::LexemeType::CloseBrace, "}");
    EXPECT_EQ(tokens[9].getTypeName(), "Close Brace");
    tokens.emplace_back(4, 1, Lexing::LexemeType::Invalid, "}");
    EXPECT_EQ(tokens[10].getTypeName(), "Invalid Token");
    tokens.emplace_back(4, 1, Lexing::LexemeType::EndOfFile, "}");
    EXPECT_EQ(tokens[11].getTypeName(), "End of File");
}

TEST(Lexing, decrement)
{
    const std::string testProgram =
    "--";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Lexeme> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    const Lexing::Lexeme decrementToken(1, 1, Lexing::LexemeType::Decrement, "--");
    ASSERT_EQ(lexemes[0], decrementToken);
    ASSERT_EQ(lexemes.size(), 1);
}

TEST(Lexing, minusMinusInBraces)
{
    const std::string testProgram =
    "-(-)";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Lexeme> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 4);
    Lexing::Lexeme firstMinus(1, 1, Lexing::LexemeType::Minus, "-");
    ASSERT_EQ(lexemes[0], firstMinus);
    Lexing::Lexeme openParen(1, 2, Lexing::LexemeType::OpenParen, "(");
    ASSERT_EQ(lexemes[1], openParen);
    Lexing::Lexeme secondMinus(1, 3, Lexing::LexemeType::Minus, "-");
    ASSERT_EQ(lexemes[2], secondMinus);
    Lexing::Lexeme closeParen(1, 4, Lexing::LexemeType::CloseParen, ")");
    ASSERT_EQ(lexemes[3], closeParen);
}

TEST(Lexing, Tilde)
{
    const std::string testProgram =
    "~";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Lexeme> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 1);
    Lexing::Lexeme firstMinus(1, 1, Lexing::LexemeType::Tilde, "~");
}

#endif //LEXER_HPP