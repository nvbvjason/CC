#pragma once

#ifndef AST_TO_ASSEMBLY_HPP
#define AST_TO_ASSEMBLY_HPP

#include "../Parsing/ConcreteTree.hpp"

namespace Codegen {


class Assembly {
    using i32 = int32_t;
    struct ProgramNode;
    struct FunctionNode;
    struct InstructionNode;
    struct OperandNode;

    const Parsing::ProgramNode* c_program;
public:
    Assembly(const Parsing::ProgramNode* program)
        : c_program(program) {}
    [[nodiscard]] i32 getOutput(std::string& output) const;

    Assembly() = delete;
    Assembly(Assembly&&) = delete;
    Assembly& operator=(Assembly&&) = delete;
    Assembly(const Assembly&) = delete;
    Assembly& operator=(const Assembly&) = delete;
private:
    [[nodiscard]] static std::string getFunction(const std::unique_ptr<Parsing::FunctionNode> &functionNode);
    [[nodiscard]] static std::string getInstruction(const std::unique_ptr<Parsing::StatementNode>& returnNode);
    [[nodiscard]] static std::string getOperand(const std::unique_ptr<Parsing::ExpressionNode>& constantNode);
};

}

#endif // AST_TO_ASSEMBLY_HPP