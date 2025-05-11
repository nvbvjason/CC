#pragma once

#ifndef CC_IR_ABSTRACT_TREE_HPP
#define CC_IR_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

/*

program = Program(top_level*)
top_level = Function(identifier, bool global, identifier params, instruction* body)
          | StaticVariable(identifier, bool global, int init)
instruction = Return(val)
            | Unary(unary_operator, val src, val dst)
            | Binary(binary_operator, val src1, val src2, val dst)
            | Copy(val src, val dst)
            | Jump(identifier target)
            | JumpIfZero(val condition, identifier target)
            | JumpIfNotZero(val condition, identifier target)
            | Label(identifier)
            | FunCall(identifier fun_name, val* args, val dst)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiplay | Divide | Remainder |
                  BitwiseOr | BitwiseAnd | BitwiseXor |
                  Leftshift | Rightshift |
                  And | Or | Equal | NotEqual |
                  LessThan | LessOrEqual | GreaterThan | GreaterOrEqual
*/

namespace Ir {

struct Program;
struct TopLevel;
struct Function;
struct Instruction;
struct Value;
struct Identifier;

struct Identifier {
    std::string value;
};

struct Program {
    std::vector<std::unique_ptr<TopLevel>> topLevels;
};

struct TopLevel {
    enum class Type {
        Function, StaticVariable
    };
    Type type;

    TopLevel() = delete;

    virtual ~TopLevel() = default;
protected:
    explicit TopLevel(const Type t)
        : type(t) {}
};

struct Function : public TopLevel {
    std::string name;
    std::vector<Identifier> args;
    std::vector<std::unique_ptr<Instruction>> insts;
    bool isGlobal;
    explicit Function(std::string identifier, const bool isGlobal)
        : TopLevel(Type::Function), name(std::move(identifier)), isGlobal(isGlobal) {}

    Function() = delete;
};

struct StaticVariable : public TopLevel {
    std::string name;
    std::shared_ptr<Value> value;
    bool isGlobal;
    explicit StaticVariable(std::string identifier,
                            const std::shared_ptr<Value>& value,
                            const bool isGlobal)
        : TopLevel(Type::StaticVariable), name
                (std::move(identifier)), value(value), isGlobal(isGlobal) {}

    StaticVariable() = delete;
};

struct Instruction {
    enum class Type {
        Return, Unary, Binary, Copy, Jump, JumpIfZero, JumpIfNotZero, Label,
        FunCall
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
        Complement, Negate, Not
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
        Add, Subtract, Multiply, Divide, Remainder,
        BitwiseAnd, BitwiseOr, BitwiseXor,
        LeftShift, RightShift,
        And, Or, Equal, NotEqual,
        LessThan, LessOrEqual, GreaterThan, GreaterOrEqual,
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

struct CopyInst final : Instruction {
    std::shared_ptr<Value> source;
    std::shared_ptr<Value> destination;
    CopyInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst)
        : Instruction(Type::Copy), source(std::move(src)), destination(std::move(dst)) {}

    CopyInst() = delete;
};

struct JumpInst final : Instruction {
    Identifier target;
    explicit JumpInst(Identifier target)
        : Instruction(Type::Jump), target(std::move(target)) {}

    JumpInst() = delete;
};

struct JumpIfZeroInst final : Instruction {
    std::shared_ptr<Value> condition;
    Identifier target;
    JumpIfZeroInst(std::shared_ptr<Value> condition, Identifier target)
        : Instruction(Type::JumpIfZero), condition(std::move(condition)), target(std::move(target)) {}

    JumpIfZeroInst() = delete;
};

struct JumpIfNotZeroInst final : Instruction {
    std::shared_ptr<Value> condition;
    Identifier target;
    JumpIfNotZeroInst(std::shared_ptr<Value> condition, Identifier target)
        : Instruction(Type::JumpIfNotZero), condition(std::move(condition)), target(std::move(target)) {}

    JumpIfNotZeroInst() = delete;
};

struct LabelInst final : Instruction {
    Identifier target;
    explicit LabelInst(Identifier target)
        : Instruction(Type::Label), target(std::move(target)) {}

    LabelInst() = delete;
};

struct FunCallInst final : Instruction {
    Identifier funName;
    std::vector<std::shared_ptr<Value>> args;
    std::shared_ptr<Value> destination;
    FunCallInst(Identifier funName, std::vector<std::shared_ptr<Value>> args, std::shared_ptr<Value> dst)
        : Instruction(Type::FunCall), funName(std::move(funName)), args(std::move(args)), destination(std::move(dst)) {}

    FunCallInst() = delete;
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
    Identifier value;
    explicit ValueVar(Identifier v)
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
