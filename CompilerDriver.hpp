#pragma once

#ifndef CC_PROGRAM_H
#define CC_PROGRAM_H

#include "Parsing/Parser.hpp"

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
    [[nodiscard]] static i32 runTwoArgs(const std::string& inputFile);
    [[nodiscard]] i32 runThreeArgs(const std::string& inputFile) const;
};

static i32 lex(std::vector<Lexing::Lexeme>& lexemes, const std::string& inputFile);
static i32 parse(const std::vector<Lexing::Lexeme>& tokens, Parsing::ProgramNode& programNode);
static i32 codegen(const Parsing::ProgramNode& programNode, std::string& output);
static bool fileExists(const std::string &name);
static bool isCommandLineArgumentValid(const std::string &argument);
static std::string preProcess(const std::string &file);
static i32 assemble(const std::string& asmFile);
std::string getSourceCode(const std::string& inputFile);
std::string astPrinter(const Parsing::ProgramNode &program);

#endif // CC_PROGRAM_H