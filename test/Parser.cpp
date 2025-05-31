#include "Parser.hpp"
#include "Parsing/Parser.hpp"

#include "gtest/gtest.h"
#include <vector>

namespace {
using Token = Lexing::Token;
using Type = Lexing::Token::Type;
}

std::vector<Token> generateTokens(const std::vector<Type>& tokenTypes)
{
    std::vector<Token> tokens;
    tokens.reserve(tokenTypes.size() + 1);
    for (const auto& tokenType : tokenTypes)
        tokens.emplace_back(1, 1, tokenType, "");
    tokens.emplace_back(1, 1, Type::EndOfFile, "");
    return tokens;
}

Parsing::Parser createParser(const std::vector<Type>& tokenTypes)
{
    const std::vector<Token> tokens = generateTokens(tokenTypes);
    return Parsing::Parser(tokens);
}

TEST(ParserTests, BlockParseSuccesEmpty)
{
    const std::vector tokenTypes{Type::OpenParen, Type::CloseBrace};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto ptr= parser.blockParse();
    EXPECT_EQ(nullptr, ptr);
}

TEST(ParserTests, BlockParseSuccesWithBody)
{
    const std::vector tokenTypes{Type::OpenParen, Type::Semicolon, Type::CloseBrace};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto ptr= parser.blockParse();
    EXPECT_EQ(nullptr, ptr);
}


TEST(ParserTests, BlockParseMissingOpenBrace)
{
    const std::vector tokenTypes{Type::IntKeyword, Type::CloseBrace};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto ptr= parser.blockParse();
    EXPECT_EQ(nullptr, ptr);
}

TEST(ParserTests, forInitParseSuccesInit)
{
    const std::vector tokenTypes{Type::IntKeyword, Type::Identifier, Type::Equal, Type::IntegerLiteral, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto [ptr, err] = parser.forInitParse();
    EXPECT_FALSE(err);
}

TEST(ParserTests, forInitParseSuccesExpr)
{
    const std::vector tokenTypes{Type::IntegerLiteral, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto [ptr, err] = parser.forInitParse();
    EXPECT_FALSE(err);
}

TEST(ParserTests, forInitParseMissingSemicolon)
{
    const std::vector tokenTypes{Type::IntegerLiteral};
    Parsing::Parser parser = createParser(tokenTypes);
    const auto [ptr, err] = parser.forInitParse();
    EXPECT_TRUE(err);
}