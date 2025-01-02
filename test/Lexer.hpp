#pragma once

#ifndef LEXER_HPP
#define LEXER_HPP

#include "../Lexing/Lexer.hpp"
#include "../Lexing/Token.hpp"

#include <gtest/gtest.h>

TEST(Lexing, getTokens)
{
    const std::string testProgram =
    "int main(void) {\n"
    "// int main return\n"
    "    return 100;\n"
    "}";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Token> tokens = lexer.tokenize();
    Lexing::Token intKeywordToken(1, 1, Lexing::TokenType::INT_KEYWORD, "int");
    ASSERT_EQ(tokens[0], intKeywordToken);
    Lexing::Token mainToken(1, 5, Lexing::TokenType::IDENTIFIER, "main");
    ASSERT_EQ(tokens[1], mainToken);
    Lexing::Token openParenToken(1, 9, Lexing::TokenType::OPEN_PAREN, "(");
    ASSERT_EQ(tokens[2], openParenToken);
    Lexing::Token voidToken(1, 10, Lexing::TokenType::VOID, "void");
    ASSERT_EQ(tokens[3], voidToken);
    Lexing::Token closeParenToken(1, 14, Lexing::TokenType::CLOSE_PAREN, ")");
    ASSERT_EQ(tokens[4], closeParenToken);
    Lexing::Token openBraceToken(1, 16, Lexing::TokenType::OPEN_BRACE, "{");
    ASSERT_EQ(tokens[5], openBraceToken);
    Lexing::Token returnToken(3, 5, Lexing::TokenType::RETURN, "return");
    ASSERT_EQ(tokens[6], returnToken);
    Lexing::Token intToken(3, 12, Lexing::TokenType::INTEGER, "100");
    ASSERT_EQ(tokens[7], intToken);
    Lexing::Token semicolonToken(3, 15, Lexing::TokenType::SEMICOLON, ";");
    ASSERT_EQ(tokens[8], semicolonToken);
    Lexing::Token closeBraceToken(4, 1, Lexing::TokenType::CLOSE_BRACE, "}");
    ASSERT_EQ(tokens[9], closeBraceToken);
}

TEST(Lexing, multiLineComment)
{
    const std::string testProgram =
    "/*int main(void) {\n"
    "// int main return\n"
    "    return 100;\n"
    "}*/";
    Lexing::Lexer lexer(testProgram);
    const std::vector<Lexing::Token> tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 0);
}

TEST(Lexing, whiteSpace)
{
    const std::string testProgram =
    "\t\r";
    Lexing::Lexer lexer(testProgram);
    const std::vector<Lexing::Token> tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 0);
}

TEST(Lexing, invalid)
{
    const std::string testProgram =
    "\\";
    Lexing::Lexer lexer(testProgram);
    const std::vector<Lexing::Token> tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 1);
    Lexing::Token invalidToken(1, 1, Lexing::TokenType::INVALID, "\\");
    ASSERT_EQ(tokens[0], invalidToken);
}

TEST(Lexing, invalidMultiline)
{
    const std::string testProgram =
    "/*  ";
    Lexing::Lexer lexer(testProgram);
    const std::vector<Lexing::Token> tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 0);
}

TEST(Lexing, testTokenToString)
{
    std::vector<Lexing::Token> tokens;
    tokens.emplace_back(1, 1, Lexing::TokenType::INT_KEYWORD, "int");
    EXPECT_EQ(tokens[0].getTypeName(), "Int Keyword");
    tokens.emplace_back(1, 5, Lexing::TokenType::IDENTIFIER, "main");
    EXPECT_EQ(tokens[1].getTypeName(), "Identifier");
    tokens.emplace_back(1, 9, Lexing::TokenType::OPEN_PAREN, "(");
    EXPECT_EQ(tokens[2].getTypeName(), "Open Paren");
    tokens.emplace_back(1, 10, Lexing::TokenType::VOID, "void");
    EXPECT_EQ(tokens[3].getTypeName(), "Void");
    tokens.emplace_back(1, 14, Lexing::TokenType::CLOSE_PAREN, ")");
    EXPECT_EQ(tokens[4].getTypeName(), "Close Paren");
    tokens.emplace_back(1, 16, Lexing::TokenType::OPEN_BRACE, "{");
    EXPECT_EQ(tokens[5].getTypeName(), "Open Brace");
    tokens.emplace_back(3, 5, Lexing::TokenType::RETURN, "return");
    EXPECT_EQ(tokens[6].getTypeName(), "Return");
    tokens.emplace_back(3, 12, Lexing::TokenType::INTEGER, "100");
    EXPECT_EQ(tokens[7].getTypeName(), "Integer");
    tokens.emplace_back(3, 15, Lexing::TokenType::SEMICOLON, ";");
    EXPECT_EQ(tokens[8].getTypeName(), "Semicolon");
    tokens.emplace_back(4, 1, Lexing::TokenType::CLOSE_BRACE, "}");
    EXPECT_EQ(tokens[9].getTypeName(), "Close Brace");
}

TEST(Lexing, decrement)
{
    const std::string testProgram =
    "--";
    Lexing::Lexer lexer(testProgram);
    const std::vector<Lexing::Token> tokens = lexer.tokenize();
    Lexing::Token decrementToken(1, 1, Lexing::TokenType::DECREMENT, "--");
    ASSERT_EQ(tokens[0], decrementToken);
    ASSERT_EQ(tokens.size(), 1);
}

TEST(Lexing, minusMinusInBraces)
{
    const std::string testProgram =
    "-(-)";
    Lexing::Lexer lexer(testProgram);
    const std::vector<Lexing::Token> tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 4);
    Lexing::Token firstMinus(1, 1, Lexing::TokenType::MINUS, "-");
    ASSERT_EQ(tokens[0], firstMinus);
    Lexing::Token openParen(1, 2, Lexing::TokenType::OPEN_PAREN, "(");
    ASSERT_EQ(tokens[1], openParen);
    Lexing::Token secondMinus(1, 3, Lexing::TokenType::MINUS, "-");
    ASSERT_EQ(tokens[2], secondMinus);
    Lexing::Token closeParen(1, 4, Lexing::TokenType::CLOSE_PAREN, ")");
    ASSERT_EQ(tokens[3], closeParen);
}

TEST(Lexing, Tilde)
{
    const std::string testProgram =
    "~";
    Lexing::Lexer lexer(testProgram);
    const std::vector<Lexing::Token> tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 1);
    Lexing::Token firstMinus(1, 1, Lexing::TokenType::TILDE, "~");
}

#endif //LEXER_HPP