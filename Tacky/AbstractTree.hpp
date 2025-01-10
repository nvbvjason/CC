#pragma once

#ifndef CC_TACKY_ABSTRACT_TREE_HPP
#define CC_TACKY_ABSTRACT_TREE_HPP

#include <memory>

#include "../AbbreviationsOfTypes.hpp"

#include <string>
#include <variant>
#include <vector>

/*
    program = Program(function_definition)
    function_definition = Function(identifier, instruction* body)
    instruction = Return(val) | Unary(unary_operator, val src, val dst)
    val = Constant(int) | Var(identifier)
    unary_operator = Complement | Negate
*/

namespace Tacky {
struct ProgramNode;
struct FunctionNode;
struct InstructionNode;
struct ValueNode;
struct UnaryNode;

struct ProgramNode {
    std::unique_ptr<FunctionNode> function = nullptr;
};

struct FunctionNode {
    std::string identifier;
    std::vector<ValueNode> instructions;
};

enum class InstructionType {
    Return, Unary,
    Invalid
};

struct InstructionNode {
    InstructionType type = InstructionType::Invalid;
    std::variant<std::unique_ptr<ValueNode>, std::unique_ptr<UnaryNode>> value;
};

enum class UnaryOperationType {
    Complement, Negate,
    Invalid
};

struct UnaryNode {
    UnaryOperationType type = UnaryOperationType::Invalid;
    std::unique_ptr<ValueNode> source = nullptr;
    std::unique_ptr<ValueNode> destination = nullptr;
};

struct ValueNode {
    std::variant<i32, std::string> value;
    explicit ValueNode(i32 value)
        : value(value) {}
    explicit ValueNode(std::string value)
        : value(std::move(value)) {}
};
} // Tacky

#endif // CC_TACKY_ABSTRACT_TREE_HPP
