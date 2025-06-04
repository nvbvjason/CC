#include "Parser.hpp"
#include "Parsing/Parser.hpp"

#include "gtest/gtest.h"
#include <vector>

namespace {
using Token = Lexing::Token;
using TokenType = Lexing::Token::Type;
}

std::vector<Token> generateTokens(const std::vector<TokenType>& tokenTypes)
{
    std::vector<Token> tokens;
    tokens.reserve(tokenTypes.size() + 1);
    for (const auto& tokenType : tokenTypes)
        tokens.emplace_back(1, 1, tokenType, "");
    tokens.emplace_back(1, 1, TokenType::EndOfFile, "");
    return tokens;
}

Parsing::Parser createParser(const std::vector<TokenType>& tokenTypes)
{
    const std::vector<Token> tokens = generateTokens(tokenTypes);
    return Parsing::Parser(tokens);
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
    const std::vector tokenTypes{TokenType::IntKeyword, TokenType::Identifier, TokenType::Equal, TokenType::IntegerLiteral, TokenType::Semicolon};
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