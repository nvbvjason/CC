#pragma once

#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <gtest/gtest.h>

#include "../Parsing/Parser.hpp"
#include "../Lexing/Lexer.hpp"
#include "../Lexing/Lexeme.hpp"

TEST(Lexing, invalidNameForMain)
{
    const std::string testProgram =
    "int master(void) {\n"
    "    return 0;\n"
    "}";
    Lexing::Lexer lexer(testProgram);
    std::vector<Lexing::Lexeme> tokens = lexer.tokenize();
    Parsing::Parser parser(tokens);
    Parsing::ProgramNode program;
    ASSERT_NE(0, parser.parseProgram(program));
}

#endif //CODEGEN_HPP
