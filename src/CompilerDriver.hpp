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
    CompilerDriver(const int argc, char *argv[])
        : args(std::vector<std::string>(argv, argv + argc)) {}

    bool validateCommandLineArguments(std::string& argument, ErrorCode& value1) const;
    [[nodiscard]] ErrorCode wrappedRun();
    [[nodiscard]] i32 run();
private:
    void writeAssmFile(const std::string& inputFile, const std::string& output, const std::string& argument);
};

std::string getSourceCode(const std::string& inputFile);
std::string astPrinter(const Parsing::Program &program);

#endif // CC_CompilerDriver_HPP