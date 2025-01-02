#pragma once

#ifndef AST_TO_ASSEMBLY_HPP
#define AST_TO_ASSEMBLY_HPP

#include "../Parsing/Parser.hpp"

#include <utility>

namespace Codegen {


class Assembly {
    using i32 = int32_t;
    struct ProgramNode;
    struct FunctionNode;
    struct InstructionNode;
    struct OperandNode;

    Parsing::ProgramNode c_program;
public:
    explicit Assembly(Parsing::ProgramNode program)
        : c_program(std::move(program)) {}
    void writeToFile(const std::string& fileName) const;

    Assembly() = delete;
    Assembly(Assembly&&) = delete;
    Assembly& operator=(Assembly&&) = delete;
    Assembly(const Assembly&) = delete;
    Assembly& operator=(const Assembly&) = delete;
private:
    [[nodiscard]] std::string getFunction(const Parsing::FunctionNode& functionNode) const;
    [[nodiscard]] std::string getInstruction() const;
    [[nodiscard]] std::string getOperand() const;
};

}

#endif // AST_TO_ASSEMBLY_HPP