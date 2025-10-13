#pragma once

#include "ASTIr.hpp"
#include "StateCode.hpp"
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
    FrontendDriver(std::string arg, std::filesystem::path inputFile)
        : m_arg(std::move(arg)), m_inputFile(std::move(inputFile)) {}

    [[nodiscard]] std::tuple<std::optional<Ir::Program>, StateCode> run() const;
};

StateCode validateSemantics(Parsing::Program& program, SymbolTable& symbolTable);