#pragma once

#ifndef CC_CompilerDriver_HPP
#define CC_CompilerDriver_HPP

#include "Frontend/Parsing/Parser.hpp"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "ErrorCodes.hpp"

class CompilerDriver {
    std::vector<std::string> args;
    std::string m_outputFileName;
public:
    CompilerDriver() = delete;
    CompilerDriver(const CompilerDriver& other) = delete;
    explicit CompilerDriver(std::vector<std::string> args)
        : args(std::move(args)) {}

    bool validateCommandLineArguments(std::string& argument, ErrorCode& value1) const;
    [[nodiscard]] ErrorCode wrappedRun();
    [[nodiscard]] i32 run();
private:
    std::string getInputFolder() const;
    void writeAssmFile(const std::string& inputFile, const std::string& output, const std::string& argument);
};

std::string getSourceCode(const std::string& inputFile);
std::string astPrinter(const Parsing::Program &program);

#endif // CC_CompilerDriver_HPP