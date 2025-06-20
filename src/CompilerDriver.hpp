#pragma once

#ifndef CC_CompilerDriver_HPP
#define CC_CompilerDriver_HPP

#include "Frontend/Parsing/Parser.hpp"

#include <filesystem>
#include <string>
#include <vector>

#include "StateCode.hpp"

class CompilerDriver {
    std::vector<std::string> m_args;
    std::string m_outputFileName;
public:
    CompilerDriver() = delete;
    CompilerDriver(const CompilerDriver& other) = delete;
    CompilerDriver(const int argc, char *argv[])
        : m_args(std::vector<std::string>(argv, argv + argc)) {}

    StateCode validateAndSetArg(std::string& argument) const;
    [[nodiscard]] i32 run();
private:
    [[nodiscard]] StateCode wrappedRun();
    StateCode writeAssmFile(const std::string& inputFile, const std::string& output, const std::string& argument);
};

std::string getSourceCode(const std::string& inputFile);
std::string astPrinter(const Parsing::Program &program);

#endif // CC_CompilerDriver_HPP