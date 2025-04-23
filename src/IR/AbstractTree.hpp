#pragma once

#ifndef CC_IR_ABSTRACT_TREE_HPP
#define CC_IR_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"
#include "AbstractTree.hpp"

#include <memory>
#include <string>
#include <utility>
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

struct Value {
    enum class Type {
        Variable, Constant,
        Invalid
    };
    Type type = Type::Invalid;
    std::variant<i32, std::string> value;
    explicit Value(i32 value)
        : value(value) {}
    explicit Value(std::string value)
        : value(std::move(value)) {}
};

struct Unary {
    enum class OperationType {
        Complement, Negate,
        Invalid
    };
    OperationType operation = OperationType::Invalid;
    Value source;
    Value destination;
    Unary(const OperationType operation, Value source, Value destination)
        : operation(operation), source(std::move(source)), destination(std::move(destination)) {}
};

struct Program {
    std::unique_ptr<Function> function = nullptr;
};

struct Function {
    std::string identifier;
    std::vector<Instruction> instructions;
};

struct Instruction {
    enum class Type {
        Return, Unary,
        Invalid
    };
    Type type = Type::Invalid;
    std::variant<std::unique_ptr<Value>, std::unique_ptr<Unary>> value;
    explicit Instruction(const Value& v)
        : type(Type::Return), value(std::make_unique<Value>(v)) {}
    Instruction(const Unary::OperationType type, const Value& src, const Value& dst)
        : type(Type::Unary), value(std::make_unique<Unary>(type, src, dst)) {}
};

} // IR

#endif // CC_IR_ABSTRACT_TREE_HPP
