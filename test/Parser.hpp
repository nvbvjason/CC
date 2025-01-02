#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include <gtest/gtest.h>

#include "../Parsing/Parser.hpp"

namespace GTest {

// #include "../Lexing/Lexer.hpp"
// #include "../Parsing/Parser.hpp"
//
// TEST(Lexing, invalidNameForMain)
// {
//     const std::string testProgram =
//     "int master(void) {\n"
//     "    return 0;\n"
//     "}";
//     Lexing::Lexer lexer(testProgram);
//     std::vector<Lexing::Token> tokens = lexer.tokenize();
//     Parsing::Parser parser(tokens);
//     Parsing::ProgramNode program;
//     ASSERT_NE(0, parser.parse());
// }
}

#endif //PARSER_HPP
