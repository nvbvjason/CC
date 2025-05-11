#include "Parser.hpp"

#include <gtest/gtest.h>

namespace {
    using Type = Lexing::Token::Type;
    using Kind = Parsing::Expr::Kind;
}

TEST(ParserExprTests, UnarySuccess)
{
    const std::vector tokenTypes{Type::Plus, Type::Integer};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.unaryExprParse());
}

TEST(ParserExprTests, UnaryCorrectType)
{
    const std::vector tokenTypes{Type::Plus, Type::Integer};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Unary, parser.unaryExprParse()->kind);
}

TEST(ParserExprTests, UnaryFailure)
{
    const std::vector tokenTypes{Type::Plus, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.unaryExprParse());
}

TEST(ParserExprTests, PostfixIncrementSuccess)
{
    const std::vector tokenTypes{Type::Integer, Type::Increment};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Unary, parser.exprPostfix()->kind);
}

TEST(ParserExprTests, PostfixDecrementSuccess)
{
    const std::vector tokenTypes{Type::Integer, Type::Decrement};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Unary, parser.exprPostfix()->kind);
}

TEST(ParserExprTests, PostfixIntegerLiteranl)
{
    const std::vector tokenTypes{Type::Integer};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Constant, parser.exprPostfix()->kind);
}

TEST(ParserExprTests, FactorIntgerLiteralSuccess)
{
    const std::vector tokenTypes{Type::Integer};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorIntgerLiteralCorrectType)
{
    const std::vector tokenTypes{Type::Integer};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Constant, parser.factorParse()->kind);
}

TEST(ParserExprTests, FactorIdentifierVarSuccess)
{
    const std::vector tokenTypes{Type::Identifier};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorIdentifierVarCorrectType)
{
    const std::vector tokenTypes{Type::Identifier};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::Var, parser.factorParse()->kind);
}

TEST(ParserExprTests, FactorIdentifierFunCallSuccess)
{
    const std::vector tokenTypes{Type::Identifier, Type::OpenParen, Type::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorIdentifierFunCallCorrectType)
{
    const std::vector tokenTypes{Type::Identifier, Type::OpenParen, Type::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(Kind::FunctionCall, parser.factorParse()->kind);
}

TEST(ParserExprTests, FactorBracedSuccess)
{
    const std::vector tokenTypes{Type::OpenParen ,Type::Integer, Type::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorBracedMissingInner)
{
    const std::vector tokenTypes{Type::OpenParen, Type::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorBracedMissingClose)
{
    const std::vector tokenTypes{Type::OpenParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.factorParse());
}

TEST(ParserExprTests, FactorBracedWrongTokenStart)
{
    const std::vector tokenTypes{Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.factorParse());
}