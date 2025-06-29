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
            | GetAddress(val src, val dst)
            | Load(val src, val dst)
            | Store(val src, val dst)
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

    static bool classOf(const Value* value) { return value->kind == Kind::Variable; }

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

    static bool classOf(const Value* value) { return value->kind == Kind::Constant; }

    ValueConst() = delete;
};

struct Inst {
    enum class Kind {
        Return,
        SignExtend, Truncate, ZeroExtend,
        DoubleToInt, DoubleToUInt, IntToDouble, UIntToDouble,
        Unary, Binary, Copy, GetAddress, Load, Store,
        Jump, JumpIfZero, JumpIfNotZero, Label,
        FunCall
    };
    Kind kind;
    Type type;

    Inst() = delete;

    virtual ~Inst() = default;
protected:
    explicit Inst(const Kind k, const Type t)
        : kind(k), type(t) {}
};

struct ReturnInst final : Inst {
    std::shared_ptr<Value> returnValue;
    explicit ReturnInst(std::shared_ptr<Value> v, const Type t)
        : Inst(Kind::Return, t), returnValue(std::move(v)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Return; }

    ReturnInst() = delete;
};

struct SignExtendInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    SignExtendInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::SignExtend, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::SignExtend; }

    SignExtendInst() = delete;
};

struct TruncateInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    TruncateInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::Truncate, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Truncate; }

    TruncateInst() = delete;
};

struct ZeroExtendInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    ZeroExtendInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::ZeroExtend, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::ZeroExtend; }

    ZeroExtendInst() = delete;
};

struct DoubleToIntInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    DoubleToIntInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::DoubleToInt, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::DoubleToInt; }

    DoubleToIntInst() = delete;
};

struct DoubleToUIntInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    DoubleToUIntInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::DoubleToUInt, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::DoubleToUInt; }

    DoubleToUIntInst() = delete;
};

struct IntToDoubleInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    IntToDoubleInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::IntToDouble, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::IntToDouble; }

    IntToDoubleInst() = delete;
};

struct UIntToDoubleInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    UIntToDoubleInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::UIntToDouble, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::UIntToDouble; }

    UIntToDoubleInst() = delete;
};

struct UnaryInst final : Inst {
    enum class Operation {
        Complement, Negate, Not
    };
    Operation operation;
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    UnaryInst(const Operation op, std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::Unary, t), operation(op), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Unary; }

    UnaryInst() = delete;
};

struct BinaryInst final : Inst {
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
    std::shared_ptr<Value> dst;
    BinaryInst(const Operation op,
               const std::shared_ptr<Value>& src1,
               const std::shared_ptr<Value>& src2,
               const std::shared_ptr<Value>& dst,
               const Type t)
        : Inst(Kind::Binary, t), operation(op), lhs(src1), rhs(src2), dst(dst) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Binary; }

    BinaryInst() = delete;
};

struct CopyInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    CopyInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::Copy, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Copy; }

    CopyInst() = delete;
};

struct GetAddressInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    GetAddressInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::GetAddress, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::GetAddress; }

    GetAddressInst() = delete;
};

struct LoadInst final : Inst {
    std::shared_ptr<Value> ptr;
    std::shared_ptr<Value> dst;
    LoadInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::Load, t), ptr(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Load; }

    LoadInst() = delete;
};

struct StoreInst final : Inst {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> ptr;
    StoreInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Inst(Kind::Store, t), src(std::move(src)), ptr(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Store; }

    StoreInst() = delete;
};

struct JumpInst final : Inst {
    Identifier target;
    explicit JumpInst(Identifier target)
        : Inst(Kind::Jump, Type::I32), target(std::move(target)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Jump; }

    JumpInst() = delete;
};

struct JumpIfZeroInst final : Inst {
    std::shared_ptr<Value> condition;
    Identifier target;
    JumpIfZeroInst(std::shared_ptr<Value> condition, Identifier target)
        : Inst(Kind::JumpIfZero, condition->type), condition(std::move(condition)), target(std::move(target)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::JumpIfZero; }

    JumpIfZeroInst() = delete;
};

struct JumpIfNotZeroInst final : Inst {
    std::shared_ptr<Value> condition;
    Identifier target;
    JumpIfNotZeroInst(std::shared_ptr<Value> condition, Identifier target)
        : Inst(Kind::JumpIfNotZero, condition->type), condition(std::move(condition)), target(std::move(target)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::JumpIfNotZero; }

    JumpIfNotZeroInst() = delete;
};

struct LabelInst final : Inst {
    Identifier target;
    explicit LabelInst(Identifier target)
        : Inst(Kind::Label, Type::I32), target(std::move(target)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Label; }

    LabelInst() = delete;
};

struct FunCallInst final : Inst {
    Identifier funName;
    std::vector<std::shared_ptr<Value>> args;
    std::shared_ptr<Value> destination;
    FunCallInst(Identifier funName,
                std::vector<std::shared_ptr<Value>> args,
                std::shared_ptr<Value> dst,
                const Type t)
        : Inst(Kind::FunCall, t), funName(std::move(funName)), args(std::move(args)), destination(std::move(dst)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::FunCall; }

    FunCallInst() = delete;
};

struct TopLevel {
    enum class Kind {
        Function, StaticVariable
    };
    Kind kind;

    TopLevel() = delete;

    virtual ~TopLevel() = default;
protected:
    explicit TopLevel(const Kind t)
        : kind(t) {}
};

struct Function : TopLevel {
    std::string name;
    std::vector<Identifier> args;
    std::vector<Type> argTypes;
    std::vector<std::unique_ptr<Inst>> insts;
    const bool isGlobal;
    Function(std::string identifier, const bool isGlobal)
        : TopLevel(Kind::Function), name(std::move(identifier)), isGlobal(isGlobal) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::Function; }

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

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::StaticVariable; }

    StaticVariable() = delete;
};

struct Program {
    std::vector<std::unique_ptr<TopLevel>> topLevels;
    Program() = default;

    Program(Program&&) = default;
    Program& operator=(Program&&) = default;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
};

} // IR

#endif // CC_IR_ABSTRACT_TREE_HPP