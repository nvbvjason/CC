#pragma once

#ifndef CC_PROGRAM_H
#define CC_PROGRAM_H

#include "Parsing/Parser.hpp"

#include <string>
#include <utility>
#include <vector>

class Program {
    std::vector<std::string> args;
public:
    Program() = delete;
    Program(const Program& other) = delete;
    explicit Program(std::vector<std::string> args)
        : args(std::move(args)) {}
    [[nodiscard]] int run() const;
};

static i32 lex(std::vector<Lexing::Token>& tokens, const std::string& inputFile);
static i32 parse(const std::vector<Lexing::Token>& tokens, Parsing::ProgramNode& programNode);
static i32 codegen(const Parsing::ProgramNode& programNode, std::string& output);
static bool fileExists(const std::string &name);
static bool isCommandLineArgumentValid(const std::string &argument);
static void astPrinter(const Parsing::FunctionNode &program);

#endif // CC_PROGRAM_H