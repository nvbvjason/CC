#include "Parser.hpp"

#include <gtest/gtest.h>

namespace {
using Type = Lexing::Token::Type;
}

TEST(ParserStmtTests, ReturnStmtSuccess)
{
    const std::vector tokenTypes{Type::Return, Type::Integer, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.returnStmtParse());
}

TEST(ParserStmtTests, ReturnStmtMissingReturn)
{
    const std::vector tokenTypes{Type::Integer, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.returnStmtParse());
}

TEST(ParserStmtTests, ReturnStmtMissingExpr)
{
    const std::vector tokenTypes{Type::Return, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.returnStmtParse());
}

TEST(ParserStmtTests, ReturnStmtMissingSemicolon)
{
    const std::vector tokenTypes{Type::Return, Type::Integer};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.returnStmtParse());
}

TEST(ParserStmtTests, ExprStmtSuccess)
{
    const std::vector tokenTypes{Type::Integer, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.exprStmtParse());
}

TEST(ParserStmtTests, ExprStmtMissingSemicolon)
{
    const std::vector tokenTypes{Type::Integer};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.exprStmtParse());
}

TEST(ParserStmtTests, IfStmtWrongBracketingSymbols)
{
    const std::vector tokenTypes{Type::If, Type::OpenBrace, Type::Integer, Type::CloseBrace, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.ifStmtParse());
}

TEST(ParserStmtTests, IfStmtMissingIf)
{
    const std::vector tokenTypes{Type::OpenParen, Type::Integer, Type::CloseParen, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.ifStmtParse());
}

TEST(ParserStmtTests, IfStmtMissingOpenParen)
{
    const std::vector tokenTypes{Type::If, Type::Integer, Type::CloseParen, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.ifStmtParse());
}

TEST(ParserStmtTests, IfStmtMissingExpr)
{
    const std::vector tokenTypes{Type::If, Type::OpenParen, Type::CloseParen, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.ifStmtParse());
}

TEST(ParserStmtTests, IfStmtMissingCloseParen)
{
    const std::vector tokenTypes{Type::If, Type::OpenParen, Type::Integer, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.ifStmtParse());
}

TEST(ParserStmtTests, IfStmtMissingBody)
{
    const std::vector tokenTypes{Type::If, Type::OpenParen, Type::Integer, Type::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.ifStmtParse());
}

TEST(ParserStmtTests, IfElseStmtSuccess)
{
    const std::vector tokenTypes{Type::If, Type::OpenParen, Type::Integer, Type::CloseParen, Type::Semicolon,
                                       Type::Else, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.ifStmtParse());
}

TEST(ParserStmtTests, IfElseStmtMissingBody)
{
    const std::vector tokenTypes{Type::If, Type::OpenParen, Type::Integer, Type::CloseParen, Type::Semicolon,
                                       Type::Else};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.ifStmtParse());
}

TEST(ParserStmtTests, GotoStmtSuccess)
{
    const std::vector tokenTypes{Type::Goto, Type::Identifier, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.gotoStmtParse());
}

TEST(ParserStmtTests, GotoStmtMissingGoto)
{
    const std::vector tokenTypes{Type::Identifier, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.gotoStmtParse());
}

TEST(ParserStmtTests, GotoStmtMissingIdentifier)
{
    const std::vector tokenTypes{Type::Goto, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.gotoStmtParse());
}

TEST(ParserStmtTests, GotoStmtMissingSemicolon)
{
    const std::vector tokenTypes{Type::Goto, Type::Identifier};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.gotoStmtParse());
}

TEST(ParserStmtTests, BreakStmtSuccess)
{
    const std::vector tokenTypes{Type::Break, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.breakStmtParse());
}

TEST(ParserStmtTests, BreakStmtMissingBreak)
{
    const std::vector tokenTypes{Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.breakStmtParse());
}

TEST(ParserStmtTests, BreakStmtMissingSemicolon)
{
    const std::vector tokenTypes{Type::Break};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.breakStmtParse());
}

TEST(ParserStmtTests, ContinueStmtSuccess)
{
    const std::vector tokenTypes{Type::Continue, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.continueStmtParse());
}

TEST(ParserStmtTests, ContinueStmtMissingContinue)
{
    const std::vector tokenTypes{Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.continueStmtParse());
}

TEST(ParserStmtTests, ContinueStmtMissingSemicolon)
{
    const std::vector tokenTypes{Type::Continue};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.continueStmtParse());
}

TEST(ParserStmtTests, LabelStmtSuccess)
{
    const std::vector tokenTypes{Type::Identifier, Type::Colon, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.labelStmtParse());
}

TEST(ParserStmtTests, LabelStmtMissingIdentifier)
{
    const std::vector tokenTypes{Type::Colon, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.labelStmtParse());
}

TEST(ParserStmtTests, LabelStmtMissingColon)
{
    const std::vector tokenTypes{Type::Identifier, Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.labelStmtParse());
}

TEST(ParserStmtTests, LabelStmtMissingBody)
{
    const std::vector tokenTypes{Type::Identifier, Type::Colon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.labelStmtParse());
}

TEST(ParserTests, CaseStmtSuccess)
{
    const std::vector tokenTypes{Type::Case, Type::Integer, Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.caseStmtParse());
}

TEST(ParserStmtTests, CaseStmtMissingCase)
{
    const std::vector tokenTypes{Type::Integer, Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.caseStmtParse());
}

TEST(ParserStmtTests, CaseStmtMissingColon)
{
    const std::vector tokenTypes{Type::Case, Type::Integer,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.caseStmtParse());
}

TEST(ParserStmtTests, CaseStmtMissingBody)
{
    const std::vector tokenTypes{Type::Integer, Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.caseStmtParse());
}

TEST(ParserStmtTests, DefaultStmtSuccess)
{
    const std::vector tokenTypes{Type::Default, Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.defaultStmtParse());
}

TEST(ParserStmtTests, DefaultStmtMissingDefault)
{
    const std::vector tokenTypes{Type::Colon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.defaultStmtParse());
}

TEST(ParserStmtTests, DefaultStmtMissingColon)
{
    const std::vector tokenTypes{Type::Default,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.defaultStmtParse());
}

TEST(ParserStmtTests, DefaultStmtMissingBody)
{
    const std::vector tokenTypes{Type::Default, Type::Colon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.defaultStmtParse());
}

TEST(ParserStmtTests, WhileStmtSuccess)
{
    const std::vector tokenTypes{Type::While, Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.whileStmtParse());
}

TEST(ParserStmtTests, WhileStmtMissingWhile)
{
    const std::vector tokenTypes{Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.whileStmtParse());
}

TEST(ParserStmtTests, WhileStmtMissingOpenParen)
{
    const std::vector tokenTypes{Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.whileStmtParse());
}

TEST(ParserStmtTests, WhileStmtMissingCloseParen)
{
    const std::vector tokenTypes{Type::While,
        Type::OpenParen, Type::Integer,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.whileStmtParse());
}

TEST(ParserStmtTests, WhileStmtMissingBody)
{
    const std::vector tokenTypes{Type::While,
        Type::OpenParen, Type::Integer};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.whileStmtParse());
}

TEST(ParserStmtTests, doWhileStmtSuccess)
{
    const std::vector tokenTypes{Type::Do,
        Type::Semicolon, Type::While, Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.doWhileStmtParse());
}

TEST(ParserStmtTests, doWhileStmtMissingDo)
{
    const std::vector tokenTypes{
        Type::Semicolon, Type::While, Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.doWhileStmtParse());
}

TEST(ParserStmtTests, doWhileStmtMissingBody)
{
    const std::vector tokenTypes{Type::Do,
        Type::CloseParen, Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.doWhileStmtParse());
}

TEST(ParserStmtTests, doWhileStmtMissingWhile)
{
    const std::vector tokenTypes{Type::Do,
        Type::Semicolon, Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.doWhileStmtParse());
}

TEST(ParserStmtTests, doWhileStmtMissingOpenParen)
{
    const std::vector tokenTypes{Type::Do,
        Type::Semicolon, Type::While, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.doWhileStmtParse());
}

TEST(ParserStmtTests, doWhileStmtMissingSemicolon)
{
    const std::vector tokenTypes{Type::Do,
        Type::Semicolon, Type::While, Type::OpenParen, Type::Integer, Type::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.doWhileStmtParse());
}

TEST(ParserStmtTests, doWhileStmtMissingCloseParen)
{
    const std::vector tokenTypes{Type::Do,
        Type::Semicolon, Type::While, Type::OpenParen, Type::Integer,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.doWhileStmtParse());
}

TEST(ParserStmtTests, forStmtSuccess)
{
    const std::vector tokenTypes{Type::For,
        Type::OpenParen, Type::Semicolon, Type::Semicolon, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.forStmtParse());
}

TEST(ParserStmtTests, forStmtWrongBracketing)
{
    const std::vector tokenTypes{Type::For,
        Type::OpenBrace, Type::Semicolon, Type::Semicolon, Type::CloseBrace,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.forStmtParse());
}

TEST(ParserStmtTests, forStmtMissingFor)
{
    const std::vector tokenTypes{
        Type::OpenBrace, Type::Semicolon, Type::Semicolon, Type::CloseBrace,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.forStmtParse());
}

TEST(ParserStmtTests, forStmtMissingSemicolon)
{
    const std::vector tokenTypes{Type::For,
        Type::OpenParen, Type::Semicolon, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.forStmtParse());
}

TEST(ParserStmtTests, forStmtMissingCloseParen)
{
    const std::vector tokenTypes{Type::For,
        Type::OpenParen, Type::Semicolon, Type::Semicolon,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.forStmtParse());
}

TEST(ParserStmtTests, forStmtMissingBody)
{
    const std::vector tokenTypes{Type::For,
        Type::OpenParen, Type::Semicolon, Type::Semicolon, Type::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.forStmtParse());
}

TEST(ParserStmtTests, switchStmtSuccess)
{
    const std::vector tokenTypes{Type::Switch,
        Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_NE(nullptr, parser.switchStmtParse());
}

TEST(ParserStmtTests, switchStmtWrongBracketing)
{
    const std::vector tokenTypes{Type::Switch,
        Type::OpenBrace, Type::Integer, Type::OpenBrace,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.switchStmtParse());
}

TEST(ParserStmtTests, switchStmtMissingSwitch)
{
    const std::vector tokenTypes{
        Type::OpenParen, Type::Integer, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.switchStmtParse());
}

TEST(ParserStmtTests, switchStmtMissingOpenParen)
{
    const std::vector tokenTypes{Type::Switch,
        Type::OpenBrace, Type::Integer,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.switchStmtParse());
}

TEST(ParserStmtTests, switchStmtMissingCondition)
{
    const std::vector tokenTypes{Type::Switch,
        Type::OpenParen, Type::CloseParen,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.switchStmtParse());
}

TEST(ParserStmtTests, switchStmtMissingCloseParen)
{
    const std::vector tokenTypes{Type::Switch,
        Type::OpenParen, Type::Integer,
        Type::Semicolon};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.switchStmtParse());
}

TEST(ParserStmtTests, switchStmtMissingBody)
{
    const std::vector tokenTypes{Type::Switch,
        Type::OpenParen, Type::Integer, Type::CloseParen};
    Parsing::Parser parser = createParser(tokenTypes);
    EXPECT_EQ(nullptr, parser.switchStmtParse());
}

TEST(ParserStmtTests, NullStmtSuccess)
{
    const std::vector tokens{Type::Semicolon};
    Parsing::Parser parser = createParser(tokens);
    EXPECT_NE(nullptr, parser.nullStmtParse());
}

TEST(ParserStmtTests, NullStmtFailure)
{
    const std::vector tokens{Type::OpenBrace};
    Parsing::Parser parser = createParser(tokens);
    EXPECT_EQ(nullptr, parser.nullStmtParse());
}