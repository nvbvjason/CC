#include "Parser.hpp"

#include <gtest/gtest.h>

namespace {
    using TokenType = Lexing::Token::Type;
    using Kind = Parsing::Expr::Kind;
}

TEST(ParserExprTests, UnarySuccess)
{
    const std::vector tokenTypes{TokenType::Plus, TokenType::IntegerLiteral};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.unaryExprParse());
}

TEST(ParserExprTests, UnaryCorrectType)
{
    const std::vector tokenTypes{TokenType::Plus, TokenType::IntegerLiteral};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Unary, parser.unaryExprParse()->kind);
}

TEST(ParserExprTests, UnaryFailure)
{
    const std::vector tokenTypes{TokenType::Plus, TokenType::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.unaryExprParse());
}

TEST(ParserExprTests, PostfixIncrementSuccess)
{
    const std::vector tokenTypes{TokenType::IntegerLiteral, TokenType::Increment};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Unary, parser.exprPostfix()->kind);
}

TEST(ParserExprTests, PostfixDecrementSuccess)
{
    const std::vector tokenTypes{TokenType::IntegerLiteral, TokenType::Decrement};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Unary, parser.exprPostfix()->kind);
}

TEST(ParserExprTests, PostfixIntegerLiteranl)
{
    const std::vector tokenTypes{TokenType::IntegerLiteral};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Constant, parser.exprPostfix()->kind);
}

TEST(ParserExprTests, FactorIntgerLiteralSuccess)
{
    const std::vector tokenTypes{TokenType::IntegerLiteral};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorIntgerLiteralCorrectType)
{
    const std::vector tokenTypes{TokenType::IntegerLiteral};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Constant, parser.factorParse()->kind);
}

TEST(ParserExprTests, FactorIdentifierVarSuccess)
{
    const std::vector tokenTypes{TokenType::Identifier};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorIdentifierVarCorrectType)
{
    const std::vector tokenTypes{TokenType::Identifier};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Var, parser.factorParse()->kind);
}

TEST(ParserExprTests, FactorIdentifierFunCallSuccess)
{
    const std::vector tokenTypes{TokenType::Identifier, TokenType::OpenParen, TokenType::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorIdentifierFunCallCorrectType)
{
    const std::vector tokenTypes{TokenType::Identifier, TokenType::OpenParen, TokenType::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::FunctionCall, parser.factorParse()->kind);
}

TEST(ParserExprTests, FactorBracedSuccess)
{
    const std::vector tokenTypes{TokenType::OpenParen ,TokenType::IntegerLiteral, TokenType::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorBracedMissingInner)
{
    const std::vector tokenTypes{TokenType::OpenParen, TokenType::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorBracedMissingClose)
{
    const std::vector tokenTypes{TokenType::OpenParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorBracedWrongTokenStart)
{
    const std::vector tokenTypes{TokenType::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.factorParse());
}