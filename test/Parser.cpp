#include "Parser.hpp"
#include "Parsing/Parser.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace {
using Type = Lexing::Token::Type;
using Token = Lexing::Token;
std::vector<Token> generateTokens(const std::vector<Type>& tokenTypes)
{
    std::vector<Lexing::Token> tokens;
    tokens.reserve(tokenTypes.size() + 1);
    for (const auto& tokenType : tokenTypes)
        tokens.emplace_back(1, 1, tokenType, "");
    tokens.emplace_back(1, 1, Type::EndOfFile, "");
    return tokens;
}

Parsing::Parser createParser(const std::vector<Type>& tokenTypes)
{
    const std::vector<Lexing::Token> tokens = generateTokens(tokenTypes);
    return Parsing::Parser(tokens);
}

Parsing::Parser createParser(std::vector<Token>& tokens)
{
    tokens.emplace_back(1, 1, Type::EndOfFile, "");
    return Parsing::Parser(tokens);
}
}

TEST(ParserTests, CaseStmtSuccess)
{
    const std::vector<Type> tokenTypes{Type::Case, Type::Integer, Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.caseStmtParse());
}

TEST(ParserTests, CaseStmtMissingCase)
{
    const std::vector<Type> tokenTypes{Type::Integer, Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.caseStmtParse());
}

TEST(ParserTests, CaseStmtMissingColon)
{
    const std::vector<Type> tokenTypes{Type::Integer, Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.caseStmtParse());
}

TEST(ParserTests, DefaultStmtSuccess)
{
    const std::vector<Type> tokenTypes{Type::Default, Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.defaultStmtParse());
}

TEST(ParserTests, DefaultStmtMissingDefault)
{
    const std::vector<Type> tokenTypes{Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.defaultStmtParse());
}

TEST(ParserTests, DefaultStmtMissingColon)
{
    const std::vector<Type> tokenTypes{Type::Default,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.defaultStmtParse());
}

TEST(ParserTests, DefaultStmtMissingBody)
{
    const std::vector<Type> tokenTypes{Type::Default, Type::Colon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.defaultStmtParse());
}

TEST(ParserTests, WhileStmtSuccess)
{
    const std::vector<Type> tokenTypes{Type::While, Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.whileStmtParse());
}

TEST(ParserTests, WhileStmtMissingWhile)
{
    const std::vector<Type> tokenTypes{Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.whileStmtParse());
}

TEST(ParserTests, WhileStmtMissingOpenParen)
{
    const std::vector<Type> tokenTypes{Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.whileStmtParse());
}

TEST(ParserTests, WhileStmtMissingCloseParen)
{
    const std::vector<Type> tokenTypes{Type::While,
        Type::OpenParen, Type::Integer,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.whileStmtParse());
}

TEST(ParserTests, WhileStmtMissingBody)
{
    const std::vector<Type> tokenTypes{Type::While,
        Type::OpenParen, Type::Integer};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.whileStmtParse());
}

TEST(ParserTests, doWhileStmtSuccess)
{
    const std::vector<Type> tokenTypes{Type::Do,
        Type::Semicolon, Type::While, Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.doWhileStmtParse());
}

TEST(ParserTests, doWhileStmtMissingWhile)
{
    const std::vector<Type> tokenTypes{Type::Do,
        Type::Semicolon, Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.doWhileStmtParse());
}

TEST(ParserTests, doWhileStmtMissingSemicolon)
{
    const std::vector<Type> tokenTypes{Type::Do,
        Type::Semicolon, Type::While, Type::OpenParen, Type::Integer, Type::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.doWhileStmtParse());
}

TEST(ParserTests, doWhileStmtMissingParen)
{
    const std::vector<Type> tokenTypes{Type::Do,
        Type::Semicolon, Type::While, Type::OpenParen, Type::Integer,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.doWhileStmtParse());
}

TEST(ParserTests, forStmtSuccess)
{
    const std::vector<Type> tokenTypes{Type::For,
        Type::OpenParen, Type::Semicolon, Type::Semicolon, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.forStmtParse());
}

TEST(ParserTests, forStmtWrongBracketing)
{
    const std::vector<Type> tokenTypes{Type::For,
        Type::OpenBrace, Type::Semicolon, Type::Semicolon, Type::CloseBrace,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.forStmtParse());
}

TEST(ParserTests, forStmtMissingSemicolon)
{
    const std::vector<Type> tokenTypes{Type::For,
        Type::OpenParen, Type::Semicolon, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.forStmtParse());
}

TEST(ParserTests, forStmtMissingBrace)
{
    const std::vector<Type> tokenTypes{Type::For,
        Type::OpenParen, Type::Semicolon, Type::Semicolon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.forStmtParse());
}

TEST(ParserTests, switchStmtSuccess)
{
    const std::vector<Type> tokenTypes{Type::Switch,
        Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.switchStmtParse());
}

TEST(ParserTests, switchStmtWrongBracketing)
{
    const std::vector<Type> tokenTypes{Type::Switch,
        Type::OpenBrace, Type::Integer, Type::OpenBrace,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.switchStmtParse());
}

TEST(ParserTests, switchStmtMissingSwitch)
{
    const std::vector<Type> tokenTypes{
        Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.switchStmtParse());
}

TEST(ParserTests, switchStmtMissingParen)
{
    const std::vector<Type> tokenTypes{Type::Switch,
        Type::Integer, Type::OpenBrace,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.switchStmtParse());
}

TEST(ParserTests, NullStmtSuccess)
{
    const std::vector<Type> tokens{Type::Semicolon};
    Parsing::Parser parser = createParser(tokens);
    EXPECT_NE(nullptr, parser.nullStmtParse());
}

TEST(ParserTests, NullStmtFailure)
{
    const std::vector<Type> tokens{Type::OpenBrace};
    Parsing::Parser parser = createParser(tokens);
    EXPECT_EQ(nullptr, parser.nullStmtParse());
}