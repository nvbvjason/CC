#include "Parser.hpp"

#include <gtest/gtest.h>

namespace {
using enum Lexing::Token::Type;
}

TEST_F(ParserStmtTest, ReturnStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"ValidReturn", true,
            {Return, Integer, Semicolon}},
        {"MissingReturn", false,
            {Integer, Semicolon}},
        {"MissingExpr", false,
            {Return, Semicolon}},
        {"MissingSemicolon", false,
            {Return, Integer}}
    };
    RunTestCases("ReturnStmt", &Parsing::Parser::returnStmtParse, cases);
}

TEST_F(ParserStmtTest, ExprStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"ValidExpr", true,
            {Integer, Semicolon}},
        {"MissingExpr", false,
            {Semicolon}},
        {"MissingSemicolon", false,
            {Integer}},
    };
    RunTestCases("ExprStmt", &Parsing::Parser::exprStmtParse, cases);
}

TEST_F(ParserStmtTest, IfStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid If", true,
            {If, OpenParen, Integer, CloseParen, Semicolon}},
        {"Wrong Bracketing Symbols", false,
            {If, OpenBrace, Integer, CloseBrace, Semicolon}},
        {"Missing If", false,
            {OpenParen, Integer, CloseParen, Semicolon}},
        {"Missing OpenParen", false,
        {If, Integer, CloseParen, Semicolon}},
        {"Missing Expr", false,
        {If, OpenParen, CloseParen, Semicolon}},
        {"Missing CloseParen", false,
        {If, OpenParen, Integer, Semicolon}},
        {"Missing Body", false,
        {If, OpenParen, Integer, CloseParen}},
        {"If Else Success", true,
        {If, OpenParen, Integer, CloseParen, Semicolon, Else, Semicolon}},
        {"If Else Missing Body", false,
        {If, OpenParen, Integer, CloseParen, Semicolon, Else}},
    };
    RunTestCases("IfStmt", &Parsing::Parser::ifStmtParse, cases);
}

TEST_F(ParserStmtTest, GotoStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid Goto", true,
            {Goto, Identifier, Semicolon}},
        {"Missing Goto", false,
            {Identifier, Semicolon}},
        {"Missing Identifier", false,
            {Goto, Semicolon}},
        {"Missing Semicolon", false,
        {Goto, Identifier}},
    };
    RunTestCases("Goto Stmt", &Parsing::Parser::gotoStmtParse, cases);
}

TEST_F(ParserStmtTest, BreakStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid Break", true,
            {Break, Semicolon}},
        {"Missing Break", false,
            {Semicolon}},
        {"Missing Semicolon", false,
            {Break}},
    };
    RunTestCases("Break Stmt", &Parsing::Parser::breakStmtParse, cases);
}

TEST_F(ParserStmtTest, ContinueStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid Continue", true,
            {Continue, Semicolon}},
        {"Missing Continue", false,
            {Semicolon}},
        {"Missing Semicolon", false,
            {Continue}},
    };
    RunTestCases("Continue Stmt", &Parsing::Parser::continueStmtParse, cases);
}

TEST_F(ParserStmtTest, LabelStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid Label", true,
            {Identifier, Colon, Semicolon}},
        {"Missing Identifier", false,
        {Colon, Semicolon}},
        {"Missing Colon", false,
            {Identifier, Semicolon}},
        {"Missing Body", false,
        {Identifier, Colon}},
    };
    RunTestCases("Label Stmt", &Parsing::Parser::labelStmtParse, cases);
}

TEST_F(ParserStmtTest, CaseStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid Case", true,
            {Case, Integer, Colon, Semicolon}},
        {"Missing Case", false,
            {Integer, Colon, Semicolon}},
        {"Missing Condition", false,
        {Case, Colon, Semicolon}},
        {"Missing Colon", false,
        {Case, Integer, Semicolon}},
        {"Missing Body", false,
        {Case, Integer, Colon}},
    };
    RunTestCases("Case Stmt", &Parsing::Parser::caseStmtParse, cases);
}

TEST_F(ParserStmtTest, DefaultStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid Default", true,
            {Default, Colon, Semicolon}},
        {"Missing Default", false,
        {Colon, Semicolon}},
        {"Missing Default", false,
        {Default, Colon}},
        {"Missing Colon", false,
        {Default, Semicolon}}
    };
    RunTestCases("Default Stmt", &Parsing::Parser::defaultStmtParse, cases);
}

TEST_F(ParserStmtTest, WhileStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid While", true,
        {While, OpenParen, Integer, CloseParen, Semicolon}},
        {"Missing While", false,
        {OpenParen, Integer, CloseParen, Semicolon}},
        {"Missing Open Paren", false,
    {While, Integer, CloseParen, Semicolon}},
        {"Missing Condition", false,
        {While, OpenParen, CloseParen, Semicolon}},
        {"Missing Close Paren", false,
        {While, OpenParen, Integer, Semicolon}},
        {"Missing Missing Body", false,
        {While, OpenParen, Integer, CloseParen}},
    };
    RunTestCases("While Stmt", &Parsing::Parser::whileStmtParse, cases);
}

TEST_F(ParserStmtTest, DoWhileStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid Do While", true,
{Do, Semicolon, While, OpenParen, Integer, CloseParen, Semicolon}},
        {"Missing Do While", false,
        {Semicolon, While, OpenParen, Integer, CloseParen, Semicolon}},
        {"Missing Body", false,
    {Do, Integer, While, OpenParen, Integer, CloseParen, Semicolon}},
        {"Missing While", false,
        {Do, Semicolon, OpenParen, Integer, CloseParen, Semicolon}},
    {"Missing Open Paren", false,
    {Do, Semicolon, While, Integer, CloseParen, Semicolon}},
        {"Missing Close Paren", false,
        {Do, Semicolon, While, OpenParen, Integer, Semicolon}},
        {"Missing Missing Semicolon at end", false,
        {Do, Semicolon, While, OpenParen, Integer, CloseParen}},
    };
    RunTestCases("Do While Stmt", &Parsing::Parser::doWhileStmtParse, cases);
}

TEST_F(ParserStmtTest, ForStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid For", true,
{For, OpenParen, Semicolon, Semicolon, CloseParen, Semicolon}},
        {"Wrong Bracketing Symbols", false,
        {For, OpenBrace, Semicolon, Semicolon, CloseBrace, Semicolon}},
        {"Missing For", false,
        {OpenParen, Semicolon, Semicolon, CloseParen, Semicolon}},
        {"Missing Open Paren", false,
    {For, Semicolon, Semicolon, CloseParen, Semicolon}},
        {"Missing Semicolon in for", false,
        {For, OpenParen, Semicolon, CloseParen, Semicolon}},
        {"Missing Close Paren", false,
        {For, OpenParen, Semicolon, Semicolon, Semicolon}},
        {"Missing Missing Body", false,
        {For, OpenParen, Semicolon, Semicolon, CloseParen}},
    };
    RunTestCases("For Stmt", &Parsing::Parser::forStmtParse, cases);
}

TEST_F(ParserStmtTest, SwitchStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid Switch", true,
{Switch, OpenParen, Integer, CloseParen, Semicolon}},
        {"Wrong Bracketing Symbols", false,
        {Switch, OpenBrace, Integer, CloseBrace, Semicolon}},
        {"Missing Switch", false,
        {OpenParen, Integer, CloseParen, Semicolon}},
        {"Missing Open Paren", false,
    {Switch, Integer, CloseParen, Semicolon}},
        {"Missing Condition", false,
        {Switch, OpenParen, CloseParen, Semicolon}},
        {"Missing Close Paren", false,
        {Switch, OpenParen, Integer, Semicolon}},
        {"Missing Missing Body", false,
        {Switch, OpenParen, Integer, CloseParen}},
    };
    RunTestCases("Switch Stmt", &Parsing::Parser::switchStmtParse, cases);
}

TEST_F(ParserStmtTest, NullStmtVariations)
{
    const std::vector<TestCase> cases = {
        {"Valid Null", true,
{Semicolon}},
        {"Missing Semicolon", false,
        {}},
    };
    RunTestCases("Null Stmt", &Parsing::Parser::nullStmtParse, cases);
}