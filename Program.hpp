#pragma once

#ifndef CC_PROGRAM_H
#define CC_PROGRAM_H

#include "Parser/FunctionDefinition.hpp"

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

static bool fileExists(const std::string &name);
static bool isCommandLineArgumentValid(const std::string &argument);
static void astPrinter(const Parsing::FunctionDefinition &program);

#endif // CC_PROGRAM_H