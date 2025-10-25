#include "Parser.hpp"
#include "Parsing/Parser.hpp"

#include "gtest/gtest.h"
#include <vector>

TokenStore g_tokenStore;

namespace {
using Token = Lexing::Token;
using TokenType = Lexing::Token::Type;
}

TokenStore generateTokens(const std::vector<Lexing::Token::Type>& tokenTypes)
{
    std::vector<Token> tokens;
    g_tokenStore.clear();
    g_tokenStore.reserve(tokenTypes.size() + 1);
    for (const TokenType tokenType : tokenTypes) {
        if (tokenType == TokenType::Identifier)
            g_tokenStore.emplaceBack(0, 1, 1, tokenType, "x");
        else
            g_tokenStore.emplaceBack(0, 1, 1, tokenType, "");
    }
    g_tokenStore.emplaceBack(0, 1, 1, TokenType::EndOfFile, "");
    return g_tokenStore;
}

Parsing::Parser createParser(const std::vector<TokenType>& tokenTypes)
{
    const TokenStore tokenStore = generateTokens(tokenTypes);
    return Parsing::Parser(g_tokenStore);
}

TEST(ParserTests, BlockParseSuccesEmpty)
{
    const std::vector tokenTypes{TokenType::OpenParen, TokenType::CloseBrace};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto ptr= parser.blockParse();
    EXPECT_EQ(nullptr, ptr);
}

TEST(ParserTests, BlockParseSuccesWithBody)
{
    const std::vector tokenTypes{TokenType::OpenParen, TokenType::Semicolon, TokenType::CloseBrace};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto ptr= parser.blockParse();
    EXPECT_EQ(nullptr, ptr);
}


TEST(ParserTests, BlockParseMissingOpenBrace)
{
    const std::vector tokenTypes{TokenType::IntKeyword, TokenType::CloseBrace};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto ptr= parser.blockParse();
    EXPECT_EQ(nullptr, ptr);
}

TEST(ParserTests, forInitParseSuccesInit)
{
    const std::vector tokenTypes{
        TokenType::IntKeyword, TokenType::Identifier, TokenType::Equal, TokenType::IntegerLiteral, TokenType::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto [ptr, err] = parser.forInitParse();
    EXPECT_FALSE(err);
}

TEST(ParserTests, forInitParseSuccesExpr)
{
    const std::vector tokenTypes{TokenType::IntegerLiteral, TokenType::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto [ptr, err] = parser.forInitParse();
    EXPECT_FALSE(err);
}

TEST(ParserTests, forInitParseMissingSemicolon)
{
    const std::vector tokenTypes{TokenType::IntegerLiteral};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto [ptr, err] = parser.forInitParse();
    EXPECT_TRUE(err);
}