#pragma once

#ifndef CC_PROGRAM_H
#define CC_PROGRAM_H

#include "Parsing/ConcreteTree.hpp"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

class CompilerDriver {
    std::vector<std::string> args;
    std::filesystem::path m_inputFile;
public:
    CompilerDriver() = delete;
    CompilerDriver(const CompilerDriver& other) = delete;
    explicit CompilerDriver(std::vector<std::string> args)
        : args(std::move(args)) {}
    [[nodiscard]] i32 run() const;
};

std::string getSourceCode(const std::string& inputFile);
std::string astPrinter(const Parsing::Program &program);

#endif // CC_PROGRAM_H