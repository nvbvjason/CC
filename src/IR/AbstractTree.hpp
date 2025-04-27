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
instruction = Return(val)
            | Unary(unary_operator, val src, val dst)
            | Binary(binary_operator, val src1, val src2, val dst)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate
binary_operator = Add | Subtract | Multiplay | Divide | Remainder

*/

namespace Ir {

struct Program;
struct Function;
struct Instruction;
struct Value;

struct Program {
    std::shared_ptr<Function> function = nullptr;
};

struct Function {
    std::string identifier;
    std::vector<std::shared_ptr<Instruction>> instructions;
};

struct Instruction {
    enum class Type {
        Return, Unary, Binary
    };
    Type type;

    Instruction() = delete;

    virtual ~Instruction() = default;
protected:
    explicit Instruction(const Type t)
        : type(t) {}
};

struct ReturnInst final : Instruction {
    std::shared_ptr<Value> returnValue;
    explicit ReturnInst(std::shared_ptr<Value> v)
        : Instruction(Type::Return), returnValue(std::move(v)) {}
};

struct UnaryInst final : Instruction {
    enum class Operation {
        Complement, Negate
    };
    Operation operation;
    std::shared_ptr<Value> source;
    std::shared_ptr<Value> destination;
    UnaryInst(const Operation op, std::shared_ptr<Value> src, std::shared_ptr<Value> dst)
        : Instruction(Type::Unary), operation(op), source(std::move(src)), destination(std::move(dst)) {}

    UnaryInst() = delete;
};

struct BinaryInst final : Instruction {
    enum class Operation {
        Add, Subtract, Multiply, Divide, Remainder
    };
    Operation operation;
    std::shared_ptr<Value> source1;
    std::shared_ptr<Value> source2;
    std::shared_ptr<Value> destination;
    BinaryInst(const Operation op,
               const std::shared_ptr<Value>& src1,
               const std::shared_ptr<Value>& src2,
               const std::shared_ptr<Value>& dst)
        : Instruction(Type::Binary), operation(op), source1(src1), source2(src2), destination(dst) {}

    BinaryInst() = delete;
};

struct Value {
    enum class Type {
        Variable, Constant
    };
    Type type;
    Value() = delete;
    virtual ~Value() = default;
protected:
    explicit Value(const Type t)
        : type(t) {}
};

struct ValueVar final : Value {
    std::string value;
    explicit ValueVar(std::string v)
        : Value(Type::Variable), value(std::move(v)) {}

    ValueVar() = delete;
};

struct ValueConst final : Value {
    i32 value;
    explicit ValueConst(const i32 v)
        : Value(Type::Constant), value(v) {}

    ValueConst() = delete;
};

} // IR

#endif // CC_IR_ABSTRACT_TREE_HPP
