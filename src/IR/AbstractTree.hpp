#pragma once

#ifndef CC_IR_ABSTRACT_TREE_HPP
#define CC_IR_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <string>
#include <vector>

/*

program = Program(function_definition)
function_definition = Function(identifier, instruction* body)
instruction = Return(val) | Unary(unary_operator, val src, val dst)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate

*/

namespace IR {

struct Program;
struct Function;
struct Instruction;
struct Value;
struct Unary;

struct Program {
    std::unique_ptr<Function> function = nullptr;
};

struct Function {
    std::string identifier;
    std::vector<Instruction> instructions;
};

enum class InstructionType {
    Return, Unary,
    Invalid
};

struct Instruction {
    InstructionType type = InstructionType::Invalid;
    std::variant<std::unique_ptr<Value>, std::unique_ptr<Unary>> value;
    Instruction(const InstructionType type, Unary* value)
        : type(type), value(static_cast<std::unique_ptr<Unary>>(value)) {}
};

enum class UnaryOperationType {
    Complement, Negate,
    Invalid
};

struct Unary {
    UnaryOperationType operation = UnaryOperationType::Invalid;
    std::unique_ptr<Value> source = nullptr;
    std::unique_ptr<Value> destination = nullptr;
};

enum class ValueType {
    Variable, Constant,

    Invalid
};

struct Value {
    ValueType type = ValueType::Invalid;
    std::variant<i32, std::string> value;
    explicit Value(i32 value)
        : value(value) {}
    explicit Value(std::string value)
        : value(std::move(value)) {}
};
} // IR

#endif // CC_IR_ABSTRACT_TREE_HPP
