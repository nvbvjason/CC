#pragma once

#ifndef CC_IR_ABSTRACT_TREE_HPP
#define CC_IR_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"
#include "AbstractTree.hpp"

#include <memory>
#include <mutex>
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

namespace Ir {

struct Program;
struct Function;
struct Instruction;
struct Value;

struct Program {
    std::unique_ptr<Function> function = nullptr;
};

struct Function {
    std::string identifier;
    std::vector<std::unique_ptr<Instruction>> instructions;
};

struct Instruction {
    enum class Type {
        Return, Unary,
        Invalid
    };
    Type type = Type::Invalid;

    Instruction() = delete;

    virtual ~Instruction() = default;
protected:
    explicit Instruction(const Type t)
        : type(t) {}
};

struct ReturnInst final : Instruction {
    std::unique_ptr<Value> returnValue;
    explicit ReturnInst(std::unique_ptr<Value> v)
        : Instruction(Type::Return), returnValue(std::move(v)) {}
};

struct UnaryInst final : Instruction {
    enum class Operation {
        Complement, Negate,
        Invalid
    };
    Operation operation = Operation::Invalid;
    std::unique_ptr<Value> source;
    std::unique_ptr<Value> destination;
    UnaryInst(const Operation op, std::unique_ptr<Value> src, std::unique_ptr<Value> dst)
        : Instruction(Type::Unary), operation(op), source(std::move(src)), destination(std::move(dst)) {}
};

struct Value {
    enum class Type {
        Variable, Constant,
        Invalid
    };
    Type type = Type::Invalid;
    Value() = delete;
    virtual ~Value() = default;
protected:
    explicit Value(const Type t)
        : type(t) {}
};

struct ValueVar final : Value {
    std::string value;
    explicit ValueVar(std::string  v)
        : Value(Type::Variable), value(std::move(v)) {}
};

struct ValueConst final : Value {
    i32 value;
    explicit ValueConst(const i32 v)
        : Value(Type::Constant), value(v) {}
};

} // IR

#endif // CC_IR_ABSTRACT_TREE_HPP
