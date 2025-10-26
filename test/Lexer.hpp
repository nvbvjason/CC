#pragma once

#include "Frontend/Lexing/Token.hpp"

#include <string>

struct TestCaseLexer {
    const std::string input;
    const Lexing::Token::Type expectedType;
    TestCaseLexer(std::string input, const Lexing::Token::Type token)
        : input(std::move(input)), expectedType(token) {}
};