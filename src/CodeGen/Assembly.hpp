#pragma once

#ifndef CC_CODEGEN_AST_TO_ASSEMBLY_HPP
#define CC_CODEGEN_AST_TO_ASSEMBLY_HPP

#include "Parsing/ConcreteTree.hpp"

namespace Codegen {
class Assembly {
    struct ProgramNode;
    struct FunctionNode;
    struct InstructionNode;
    struct OperandNode;

    const Parsing::Program* c_program;
public:
    explicit Assembly(const Parsing::Program* program)
        : c_program(program) {}
    void getOutput(std::string& output) const;

    Assembly() = delete;
    Assembly(Assembly&&) = delete;
    Assembly& operator=(Assembly&&) = delete;
    Assembly(const Assembly&) = delete;
    Assembly& operator=(const Assembly&) = delete;
private:
    [[nodiscard]] static std::string getFunction(const std::unique_ptr<Parsing::Function>& functionNode);
    [[nodiscard]] static std::string getInstruction(const std::unique_ptr<Parsing::Statement>& returnNode);
    [[nodiscard]] static std::string getOperand(const std::unique_ptr<Parsing::Expr>& constantNode);
};

}

#endif // CC_CODEGEN_AST_TO_ASSEMBLY_HPP