#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include "Token.hpp"
#include "Parsing/Parser.hpp"

#include <vector>
#include <gtest/gtest.h>

struct TestCase {
    std::string name;
    bool shouldSucceed;
    std::vector<Lexing::Token::Type> tokens;
};

TokenStore generateTokens(const std::vector<Lexing::Token::Type>& tokenTypes);
Parsing::Parser createParser(const std::vector<Lexing::Token::Type>& tokenTypes);

class ParserStmtTest : public ::testing::Test {
protected:
    static Parsing::Parser createParser(const std::vector<Lexing::Token::Type>& tokenTypes) {
        const TokenStore tokenStore = generateTokens(tokenTypes);
        return Parsing::Parser(tokenStore);
    }
};


template <typename ParseFunc>
void RunTestCase(const std::string& stmtName, Parsing::Parser& parser, ParseFunc parseFunc,
                 const TestCase& testCase)
{
    auto result = (parser.*parseFunc)();
    if (testCase.shouldSucceed)
        EXPECT_NE(nullptr, result) << stmtName << " - " << testCase.name;
    else
        EXPECT_EQ(nullptr, result) << stmtName << " - " << testCase.name;
}

template <typename ParseFunc>
void RunTestCases(const std::string& stmtName, ParseFunc parseFunc,
                  const std::vector<TestCase>& testCases)
{
    for (const auto& test : testCases) {
        Parsing::Parser parser = createParser(test.tokens);
        RunTestCase(stmtName, parser, parseFunc, test);
    }
}
#endif //PARSER_HPP