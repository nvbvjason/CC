#pragma once

#ifndef CC_FRONTEND_DRIVER_HPP
#define CC_FRONTEND_DRIVER_HPP

#include "AST/ASTParser.hpp"

#include <string>

class FrontendDriver {
    std::string m_arg;
public:
    FrontendDriver() = delete;
    FrontendDriver(const FrontendDriver& other) = delete;
    explicit FrontendDriver(std::string arg)
        : m_arg(std::move(arg)) {}

    [[nodiscard]] std::tuple<Parsing::Program, i32> run() const;
};

#endif // CC_FRONTEND_DRIVER_HPP