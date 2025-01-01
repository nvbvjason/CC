#pragma once

#ifndef AST_TO_ASSEMBLY_HPP
#define AST_TO_ASSEMBLY_HPP
#include <utility>

#include "../Parser/Parser.hpp"

namespace Codegen {

class AstToAssembly {
    Parsing::ProgramNode c_program;
public:
    explicit AstToAssembly(Parsing::ProgramNode program)
        : c_program(std::move(program)) {}
    void writeToFile(const std::string& fileName) const;

    AstToAssembly() = delete;
    AstToAssembly(AstToAssembly&&) = delete;
    AstToAssembly& operator=(AstToAssembly&&) = delete;
    AstToAssembly(const AstToAssembly&) = delete;
    AstToAssembly& operator=(const AstToAssembly&) = delete;
};

}

#endif // AST_TO_ASSEMBLY_HPP