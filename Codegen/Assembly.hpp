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
    Assembly(Parsing::ProgramNode program)
        : c_program(std::move(program)) {}
    void getOutput(std::string& output) const;

    Assembly() = delete;
    Assembly(Assembly&&) = delete;
    Assembly& operator=(Assembly&&) = delete;
    Assembly(const Assembly&) = delete;
    Assembly& operator=(const Assembly&) = delete;
private:
    [[nodiscard]] static std::string getFunction(const Parsing::FunctionNode& functionNode);
    [[nodiscard]] static std::string getInstruction(const Parsing::StatementNode& returnNode);
    [[nodiscard]] static std::string getOperand(const Parsing::ExpressionNode& constantNode);
};

}

#endif // AST_TO_ASSEMBLY_HPP