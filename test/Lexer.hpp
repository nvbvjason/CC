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
    std::vector<Lexing::Token> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    Lexing::Token intKeywordToken(1, 1, Lexing::TokenType::IntKeyword, "int");
    ASSERT_EQ(lexemes[0], intKeywordToken);
    Lexing::Token mainToken(1, 5, Lexing::TokenType::Identifier, "main");
    ASSERT_EQ(lexemes[1], mainToken);
    Lexing::Token openParenToken(1, 9, Lexing::TokenType::OpenParen, "(");
    ASSERT_EQ(lexemes[2], openParenToken);
    Lexing::Token voidToken(1, 10, Lexing::TokenType::Void, "void");
    ASSERT_EQ(lexemes[3], voidToken);
    Lexing::Token closeParenToken(1, 14, Lexing::TokenType::CloseParen, ")");
    ASSERT_EQ(lexemes[4], closeParenToken);
    Lexing::Token openBraceToken(1, 16, Lexing::TokenType::OpenBrace, "{");
    ASSERT_EQ(lexemes[5], openBraceToken);
    Lexing::Token returnToken(3, 5, Lexing::TokenType::Return, "return");
    ASSERT_EQ(lexemes[6], returnToken);
    Lexing::Token intToken(3, 12, Lexing::TokenType::Integer, "100");
    ASSERT_EQ(lexemes[7], intToken);
    Lexing::Token semicolonToken(3, 15, Lexing::TokenType::Semicolon, ";");
    ASSERT_EQ(lexemes[8], semicolonToken);
    Lexing::Token closeBraceToken(4, 1, Lexing::TokenType::CloseBrace, "}");
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
    std::vector<Lexing::Token> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 1);
}

TEST(Lexing, whiteSpace)
{
    const std::string testProgram =
    "\t\r";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Token> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 1);
}

TEST(Lexing, invalid)
{
    const std::string testProgram =
    "\\";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Token> lexemes;
    ASSERT_NE(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 1);
    const Lexing::Token invalidToken(1, 1, Lexing::TokenType::Invalid, "\\");
    ASSERT_EQ(lexemes[0], invalidToken);
}

TEST(Lexing, invalidMultiline)
{
    const std::string testProgram =
    "/*  ";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Token> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 1);
}

TEST(Lexing, testTokenToString)
{
    std::vector<Lexing::Token> tokens;
    tokens.emplace_back(1, 1, Lexing::TokenType::IntKeyword, "int");
    EXPECT_EQ(tokens[0].getTypeName(), "Int Keyword");
    tokens.emplace_back(1, 5, Lexing::TokenType::Identifier, "main");
    EXPECT_EQ(tokens[1].getTypeName(), "Identifier");
    tokens.emplace_back(1, 9, Lexing::TokenType::OpenParen, "(");
    EXPECT_EQ(tokens[2].getTypeName(), "Open Paren");
    tokens.emplace_back(1, 10, Lexing::TokenType::Void, "void");
    EXPECT_EQ(tokens[3].getTypeName(), "Void");
    tokens.emplace_back(1, 14, Lexing::TokenType::CloseParen, ")");
    EXPECT_EQ(tokens[4].getTypeName(), "Close Paren");
    tokens.emplace_back(1, 16, Lexing::TokenType::OpenBrace, "{");
    EXPECT_EQ(tokens[5].getTypeName(), "Open Brace");
    tokens.emplace_back(3, 5, Lexing::TokenType::Return, "return");
    EXPECT_EQ(tokens[6].getTypeName(), "Return");
    tokens.emplace_back(3, 12, Lexing::TokenType::Integer, "100");
    EXPECT_EQ(tokens[7].getTypeName(), "Integer");
    tokens.emplace_back(3, 15, Lexing::TokenType::Semicolon, ";");
    EXPECT_EQ(tokens[8].getTypeName(), "Semicolon");
    tokens.emplace_back(4, 1, Lexing::TokenType::CloseBrace, "}");
    EXPECT_EQ(tokens[9].getTypeName(), "Close Brace");
    tokens.emplace_back(4, 1, Lexing::TokenType::Invalid, "}");
    EXPECT_EQ(tokens[10].getTypeName(), "Invalid Token");
    tokens.emplace_back(4, 1, Lexing::TokenType::EndOfFile, "}");
    EXPECT_EQ(tokens[11].getTypeName(), "End of File");
}

TEST(Lexing, decrement)
{
    const std::string testProgram =
    "--";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Token> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    const Lexing::Token decrementToken(1, 1, Lexing::TokenType::Decrement, "--");
    ASSERT_EQ(lexemes[0], decrementToken);
    ASSERT_EQ(lexemes.size(), 2);
}

TEST(Lexing, minusMinusInBraces)
{
    const std::string testProgram =
    "-(-)";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Token> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 5);
    Lexing::Token firstMinus(1, 1, Lexing::TokenType::Minus, "-");
    ASSERT_EQ(lexemes[0], firstMinus);
    Lexing::Token openParen(1, 2, Lexing::TokenType::OpenParen, "(");
    ASSERT_EQ(lexemes[1], openParen);
    Lexing::Token secondMinus(1, 3, Lexing::TokenType::Minus, "-");
    ASSERT_EQ(lexemes[2], secondMinus);
    Lexing::Token closeParen(1, 4, Lexing::TokenType::CloseParen, ")");
    ASSERT_EQ(lexemes[3], closeParen);
}

TEST(Lexing, Tilde)
{
    const std::string testProgram =
    "~";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Token> lexemes;
    ASSERT_EQ(0, lexer.getLexems(lexemes));
    ASSERT_EQ(lexemes.size(), 2);
    Lexing::Token firstMinus(1, 1, Lexing::TokenType::Tilde, "~");
}

#endif //LEXER_HPP