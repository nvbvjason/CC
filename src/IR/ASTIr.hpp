#pragma once

#ifndef CC_IR_ABSTRACT_TREE_HPP
#define CC_IR_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"
#include "Types/Type.hpp"

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

/*

program = Program(top_level*)
top_level = Function(identifier, bool global, identifier params, instruction* body)
          | StaticVariable(identifier, bool global, type t, init)
instruction = Return(val)
            | SignExtend(val src, val dst)
            | Truncate(val src, val dst)
            | ZeroExtern(val src, val dst)
            | DoubleToInt(val src, val dst)
            | DoubleToUInt(val src, val dst)
            | IntToDouble(val src, val dst)
            | UIntToDouble(val src, val dst)
            | Unary(unary_operator, val src, val dst)
            | Binary(binary_operator, val src1, val src2, val dst)
            | Copy(val src, val dst)
            | Jump(identifier target)
            | JumpIfZero(val condition, identifier target)
            | JumpIfNotZero(val condition, identifier target)
            | Label(identifier)
            | FunCall(identifier fun_name, val* args, val dst)
val = Constant(init, type) | Var(identifier, type)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder |
                  BitwiseOr | BitwiseAnd | BitwiseXor |
                  Leftshift | Rightshift |
                  And | Or | Equal | NotEqual |
                  LessThan | LessOrEqual | GreaterThan | GreaterOrEqual
*/

namespace Ir {

struct Identifier {
    std::string value;
};

struct Value {
    enum class Kind {
        Variable, Constant
    };
    Type type;
    Kind kind;
    Value() = delete;
    virtual ~Value() = default;
protected:
    explicit Value(const Type t, const Kind k)
        : type(t), kind(k) {}
};

struct ValueVar final : Value {
    ReferingTo referingTo = ReferingTo::Local;
    Identifier value;
    explicit ValueVar(Identifier v, const Type t)
        : Value(t, Kind::Variable), value(std::move(v)) {}

    ~ValueVar() override;

    ValueVar() = delete;
};

struct ValueConst final : Value {
    std::variant<i32, i64, u32, u64, double> value;
    explicit ValueConst(const i32 v)
        : Value(Type::I32 ,Kind::Constant), value(v) {}
    explicit ValueConst(const i64 v)
        : Value(Type::I64 ,Kind::Constant), value(v) {}
    explicit ValueConst(const u32 v)
        : Value(Type::U32 ,Kind::Constant), value(v) {}
    explicit ValueConst(const u64 v)
        : Value(Type::U64 ,Kind::Constant), value(v) {}
    explicit ValueConst(const double v)
        : Value(Type::Double ,Kind::Constant), value(v) {}

    ~ValueConst() override;

    ValueConst() = delete;
};

struct Instruction {
    enum class Kind {
        Return,
        SignExtend, Truncate, ZeroExtend,
        DoubleToInt, DoubleToUInt, IntToDouble, UIntToDouble,
        Unary, Binary, Copy,
        Jump, JumpIfZero, JumpIfNotZero, Label,
        FunCall
    };
    Kind kind;
    Type type;

    Instruction() = delete;

    virtual ~Instruction() = default;
protected:
    explicit Instruction(const Kind k, const Type t)
        : kind(k), type(t) {}
};

struct ReturnInst final : Instruction {
    std::shared_ptr<Value> returnValue;
    explicit ReturnInst(std::shared_ptr<Value> v, const Type t)
        : Instruction(Kind::Return, t), returnValue(std::move(v)) {}

    ~ReturnInst() override;
};

struct SignExtendInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    SignExtendInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::SignExtend, t), src(std::move(src)), dst(std::move(dst)) {}
};

struct TruncateInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    TruncateInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::Truncate, t), src(std::move(src)), dst(std::move(dst)) {}

    TruncateInst() = delete;
};

struct ZeroExtendInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    ZeroExtendInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::ZeroExtend, t), src(std::move(src)), dst(std::move(dst)) {}

    ZeroExtendInst() = delete;
};

struct DoubleToIntInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    DoubleToIntInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::DoubleToInt, t), src(std::move(src)), dst(std::move(dst)) {}

    DoubleToIntInst() = delete;
};

struct DoubleToUIntInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    DoubleToUIntInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::DoubleToUInt, t), src(std::move(src)), dst(std::move(dst)) {}

    DoubleToUIntInst() = delete;
};

struct IntToDoubleInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    IntToDoubleInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::IntToDouble, t), src(std::move(src)), dst(std::move(dst)) {}

    IntToDoubleInst() = delete;
};

struct UIntToDoubleInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    UIntToDoubleInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::UIntToDouble, t), src(std::move(src)), dst(std::move(dst)) {}

    UIntToDoubleInst() = delete;
};

struct UnaryInst final : Instruction {
    enum class Operation {
        Complement, Negate, Not
    };
    Operation operation;
    std::shared_ptr<Value> source;
    std::shared_ptr<Value> destination;
    UnaryInst(const Operation op, std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::Unary, t), operation(op), source(std::move(src)), destination(std::move(dst)) {}

    ~UnaryInst() override;

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
    std::shared_ptr<Value> lhs;
    std::shared_ptr<Value> rhs;
    std::shared_ptr<Value> destination;
    BinaryInst(const Operation op,
               const std::shared_ptr<Value>& src1,
               const std::shared_ptr<Value>& src2,
               const std::shared_ptr<Value>& dst,
               const Type t)
        : Instruction(Kind::Binary, t), operation(op), lhs(src1), rhs(src2), destination(dst) {}

    ~BinaryInst() override;

    BinaryInst() = delete;
};

struct CopyInst final : Instruction {
    std::shared_ptr<Value> source;
    std::shared_ptr<Value> destination;
    CopyInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::Copy, t), source(std::move(src)), destination(std::move(dst)) {}

    ~CopyInst() override;

    CopyInst() = delete;
};

struct JumpInst final : Instruction {
    Identifier target;
    explicit JumpInst(Identifier target)
        : Instruction(Kind::Jump, Type::I32), target(std::move(target)) {}

    ~JumpInst() override;

    JumpInst() = delete;
};

struct JumpIfZeroInst final : Instruction {
    std::shared_ptr<Value> condition;
    Identifier target;
    JumpIfZeroInst(std::shared_ptr<Value> condition, Identifier target)
        : Instruction(Kind::JumpIfZero, Type::I32), condition(std::move(condition)), target(std::move(target)) {}

    ~JumpIfZeroInst() override;

    JumpIfZeroInst() = delete;
};

struct JumpIfNotZeroInst final : Instruction {
    std::shared_ptr<Value> condition;
    Identifier target;
    JumpIfNotZeroInst(std::shared_ptr<Value> condition, Identifier target)
        : Instruction(Kind::JumpIfNotZero, Type::I32), condition(std::move(condition)), target(std::move(target)) {}

    ~JumpIfNotZeroInst() override;

    JumpIfNotZeroInst() = delete;
};

struct LabelInst final : Instruction {
    Identifier target;
    explicit LabelInst(Identifier target)
        : Instruction(Kind::Label, Type::I32), target(std::move(target)) {}

    ~LabelInst() override;

    LabelInst() = delete;
};

struct FunCallInst final : Instruction {
    Identifier funName;
    std::vector<std::shared_ptr<Value>> args;
    std::shared_ptr<Value> destination;
    FunCallInst(Identifier funName,
                std::vector<std::shared_ptr<Value>> args,
                std::shared_ptr<Value> dst,
                const Type t)
        : Instruction(Kind::FunCall, t), funName(std::move(funName)), args(std::move(args)), destination(std::move(dst)) {}

    ~FunCallInst() override;

    FunCallInst() = delete;
};

struct TopLevel {
    enum class Kind {
        Function, StaticVariable
    };
    Kind type;

    TopLevel() = delete;

    virtual ~TopLevel() = default;
protected:
    explicit TopLevel(const Kind t)
        : type(t) {}
};

struct Function : TopLevel {
    std::string name;
    std::vector<Identifier> args;
    std::vector<Type> argTypes;
    std::vector<std::unique_ptr<Instruction>> insts;
    const bool isGlobal;
    Function(std::string identifier, const bool isGlobal)
        : TopLevel(Kind::Function), name(std::move(identifier)), isGlobal(isGlobal) {}

    ~Function() override;

    Function() = delete;
};

struct StaticVariable : TopLevel {
    std::string name;
    std::shared_ptr<Value> value;
    const Type type;
    const bool global;
    explicit StaticVariable(std::string identifier,
                            const std::shared_ptr<Value>& value,
                            const Type ty,
                            const bool isGlobal)
        : TopLevel(Kind::StaticVariable), name
                (std::move(identifier)), value(value), type(ty), global(isGlobal) {}

    ~StaticVariable() override;

    StaticVariable() = delete;
};

struct Program {
    std::vector<std::unique_ptr<TopLevel>> topLevels;
    Program() = default;

    Program(Program&&) = default;
    Program& operator=(Program&&) = default;
    ~Program();

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
};

} // IR

#endif // CC_IR_ABSTRACT_TREE_HPP