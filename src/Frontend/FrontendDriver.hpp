#pragma once

#include "ASTIr.hpp"
#include "StateCode.hpp"
#include "ASTParser.hpp"
#include "Error.hpp"
#include "SymbolTable.hpp"
#include "TokenStore.hpp"

#include <filesystem>
#include <string>

class FrontendDriver {
    std::string m_arg;
    std::filesystem::path m_inputFile;
    TokenStore m_tokenStore;
public:
    FrontendDriver() = delete;
    FrontendDriver(const FrontendDriver& other) = delete;
    FrontendDriver(std::string arg, std::filesystem::path inputFile)
        : m_arg(std::move(arg)), m_inputFile(std::move(inputFile)) {}

    [[nodiscard]] std::tuple<std::optional<Ir::Program>, StateCode> run();
};

std::pair<StateCode, std::vector<Error>> validateSemantics(Parsing::Program& program, SymbolTable& symbolTable);
void reportErrors(const std::vector<Error>& errors, const TokenStore& tokenStore);
void reportError(const Error& error, const TokenStore& tokenStore);