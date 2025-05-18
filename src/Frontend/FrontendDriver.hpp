#pragma once

#ifndef CC_FRONTEND_DRIVER_HPP
#define CC_FRONTEND_DRIVER_HPP

#include "ASTIr.hpp"
#include "ErrorCodes.hpp"
#include "ASTParser.hpp"
#include "SymbolTable.hpp"

#include <filesystem>
#include <string>

class FrontendDriver {
    std::string m_arg;
    std::filesystem::path m_inputFile;
public:
    FrontendDriver() = delete;
    FrontendDriver(const FrontendDriver& other) = delete;
    explicit FrontendDriver(std::string arg, std::filesystem::path inputFile)
        : m_arg(std::move(arg)), m_inputFile(std::move(inputFile)) {}

    [[nodiscard]] std::tuple<std::unique_ptr<Ir::Program>, ErrorCode> run() const;
};

ErrorCode validateSemantics(Parsing::Program& programNode, SymbolTable& symbolTable);

#endif // CC_FRONTEND_DRIVER_HPP