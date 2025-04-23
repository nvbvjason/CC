#pragma once

#ifndef CC_IR_ABSTRACT_TREE_HPP
#define CC_IR_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AbstractTree.hpp"

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

enum class UnaryOperationType {
    Complement, Negate,
    Invalid
};

struct Unary {
    UnaryOperationType operation = UnaryOperationType::Invalid;
    Value source;
    Value destination;
    Unary(const UnaryOperationType operation, Value source, Value destination)
        : operation(operation), source(std::move(source)), destination(std::move(destination)) {}
};

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
    explicit Instruction(const Value& v)
        : type(InstructionType::Return), value(std::make_unique<Value>(v)) {}
    Instruction(const UnaryOperationType type, const Value& src, const Value& dst)
        : type(InstructionType::Unary), value(std::make_unique<Unary>(type, src, dst)) {}
};

} // IR

#endif // CC_IR_ABSTRACT_TREE_HPP
