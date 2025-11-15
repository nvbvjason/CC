#pragma once

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
          | StaticConstant(identifier, type t, static_init)
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
            | AddPtr(val ptr, val index, int scale, val dst)
            | CopyToOffset(val src, identifier dst, int offset)
            | Jump(identifier target)
            | JumpIfZero(val condition, identifier target)
            | JumpIfNotZero(val condition, identifier target)
            | Label(identifier)
            | FunCall(identifier fun_name, val* args, val dst)
            | PushStackSlot(identifier name, int size)
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
    const Kind kind;
    Value() = delete;
    virtual ~Value() = default;
protected:
    explicit Value(const Type t, const Kind k)
        : type(t), kind(k) {}
};

struct ValueVar final : Value {
    ReferingTo referingTo = ReferingTo::Local;
    Identifier value;
    i64 size = 0;

    ValueVar(Identifier v, const Type t)
        : Value(t, Kind::Variable), value(std::move(v)) {}

    ValueVar(Identifier v, const Type t, const i64 size)
        : Value(t, Kind::Variable), value(std::move(v)), size(size) {}

    static bool classOf(const Value* value) { return value->kind == Kind::Variable; }

    ValueVar() = delete;
};

struct ValueConst final : Value {
    std::variant<i8, u8, i32, i64, u32, u64, double> value;
    explicit ValueConst(const u8 v)
        : Value(Type::U8, Kind::Constant), value(v) {}
    explicit ValueConst(const i8 v)
        : Value(Type::I8, Kind::Constant), value(v) {}
    explicit ValueConst(const char ch)
        : Value(Type::I8, Kind::Constant), value(ch) {}
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

struct Initializer {
    enum class Kind {
        Value, Zero
    };
    const Kind kind;

    Initializer() = delete;
protected:

    explicit Initializer(const Kind kind)
        : kind(kind) {}
};

struct ValueInitializer final : Initializer {
    std::shared_ptr<Value> value;

    explicit ValueInitializer(std::shared_ptr<Value> value)
        : Initializer(Kind::Value), value(std::move(value)) {}

    static bool classOf(const Initializer* initializer) { return initializer->kind == Kind::Value; }
};

struct ZeroInitializer final : Initializer {
    i64 size;

    explicit ZeroInitializer(const i64 size)
        : Initializer(Kind::Zero), size(size) {}

    static bool classOf(const Initializer* initializer) { return initializer->kind == Kind::Zero; }
};

struct Instruction {
    enum class Kind {
        Return,
        SignExtend, Truncate, ZeroExtend,
        DoubleToInt, DoubleToUInt, IntToDouble, UIntToDouble,
        Unary, Binary, Copy, GetAddress, Load, Store,
        AddPtr, CopyToOffset,
        Jump, JumpIfZero, JumpIfNotZero, Label,
        FunCall, Allocate
    };
    const Kind kind;
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

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Return; }

    ReturnInst() = delete;
};

struct SignExtendInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    SignExtendInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::SignExtend, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::SignExtend; }

    SignExtendInst() = delete;
};

struct TruncateInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    TruncateInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::Truncate, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Truncate; }

    TruncateInst() = delete;
};

struct ZeroExtendInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    ZeroExtendInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::ZeroExtend, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::ZeroExtend; }

    ZeroExtendInst() = delete;
};

struct DoubleToIntInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    DoubleToIntInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::DoubleToInt, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::DoubleToInt; }

    DoubleToIntInst() = delete;
};

struct DoubleToUIntInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    DoubleToUIntInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::DoubleToUInt, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::DoubleToUInt; }

    DoubleToUIntInst() = delete;
};

struct IntToDoubleInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    IntToDoubleInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::IntToDouble, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::IntToDouble; }

    IntToDoubleInst() = delete;
};

struct UIntToDoubleInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    UIntToDoubleInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::UIntToDouble, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::UIntToDouble; }

    UIntToDoubleInst() = delete;
};

struct UnaryInst final : Instruction {
    enum class Operation {
        Complement, Negate, Not
    };
    Operation operation;
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    UnaryInst(const Operation op, std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::Unary, t), operation(op), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Unary; }

    UnaryInst() = delete;
};

struct BinaryInst final : Instruction {
    enum class Operation {
        Add, Subtract, Multiply, Divide, Remainder,
        BitwiseAnd, BitwiseOr, BitwiseXor,
        LeftShift, RightShift,
        And, Or, Equal, NotEqual,
        LessThan, LessOrEqual, GreaterThan, GreaterOrEqual
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
        : Instruction(Kind::Binary, t), operation(op), lhs(src1), rhs(src2), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Binary; }

    BinaryInst() = delete;
};

struct CopyInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    CopyInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::Copy, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Copy; }

    CopyInst() = delete;
};

struct GetAddressInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> dst;
    GetAddressInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::GetAddress, t), src(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::GetAddress; }

    GetAddressInst() = delete;
};

struct LoadInst final : Instruction {
    std::shared_ptr<Value> ptr;
    std::shared_ptr<Value> dst;
    LoadInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::Load, t), ptr(std::move(src)), dst(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Load; }

    LoadInst() = delete;
};

struct StoreInst final : Instruction {
    std::shared_ptr<Value> src;
    std::shared_ptr<Value> ptr;
    StoreInst(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type t)
        : Instruction(Kind::Store, t), src(std::move(src)), ptr(std::move(dst)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Store; }

    StoreInst() = delete;
};

struct AddPtrInst final : Instruction {
    std::shared_ptr<Value> ptr;
    std::shared_ptr<Value> index;
    std::shared_ptr<Value> dst;
    i64 scale;

    AddPtrInst(std::shared_ptr<Value> src, std::shared_ptr<Value> index,
               std::shared_ptr<Value> dst, const i64 scale)
        : Instruction(Kind::AddPtr, Type::Pointer),
          ptr(std::move(src)),
          index(std::move(index)),
          dst(std::move(dst)),
          scale(scale) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::AddPtr; }

    AddPtrInst() = delete;
};

struct CopyToOffsetInst final : Instruction {
    std::shared_ptr<Value> src;
    const Identifier iden;
    const i64 offset;
    const i64 size;
    const i64 alignment;

    CopyToOffsetInst(std::shared_ptr<Value> src,
                     Identifier iden,
                     const i64 offset,
                     const i64 sizeArray,
                     const i64 alignment,
                     const Type t)
        : Instruction(Kind::CopyToOffset, t), src(std::move(src)),
          iden(std::move(iden)), offset(offset), size(sizeArray), alignment(alignment) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::CopyToOffset; }

    CopyToOffsetInst() = delete;
};

struct JumpInst final : Instruction {
    Identifier target;
    explicit JumpInst(Identifier target)
        : Instruction(Kind::Jump, Type::I32), target(std::move(target)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Jump; }

    JumpInst() = delete;
};

struct JumpIfZeroInst final : Instruction {
    std::shared_ptr<Value> condition;
    Identifier target;
    JumpIfZeroInst(std::shared_ptr<Value> condition, Identifier target)
        : Instruction(Kind::JumpIfZero, condition->type), condition(std::move(condition)), target(std::move(target)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::JumpIfZero; }

    JumpIfZeroInst() = delete;
};

struct JumpIfNotZeroInst final : Instruction {
    std::shared_ptr<Value> condition;
    Identifier target;
    JumpIfNotZeroInst(std::shared_ptr<Value> condition, Identifier target)
        : Instruction(Kind::JumpIfNotZero, condition->type), condition(std::move(condition)), target(std::move(target)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::JumpIfNotZero; }

    JumpIfNotZeroInst() = delete;
};

struct LabelInst final : Instruction {
    Identifier target;
    explicit LabelInst(Identifier target)
        : Instruction(Kind::Label, Type::I32), target(std::move(target)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Label; }

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

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::FunCall; }

    FunCallInst() = delete;
};

struct AllocateInst final : Instruction {
    const i64 size;
    const Identifier iden;

    AllocateInst(const i64 size, Identifier iden, const Type type)
        : Instruction(Kind::Allocate, type), size(size), iden(std::move(iden)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Allocate; }

    AllocateInst() = delete;
};

struct TopLevel {
    enum class Kind {
        Function, StaticVariable, StaticArray, StaticConstant
    };
    const Kind kind;

    TopLevel() = delete;

    virtual ~TopLevel() = default;
protected:
    explicit TopLevel(const Kind t)
        : kind(t) {}
};

struct Function final : TopLevel {
    std::string name;
    std::vector<Identifier> args;
    std::vector<Type> argTypes;
    std::vector<std::unique_ptr<Instruction>> insts;
    const bool isGlobal;
    Function(std::string identifier, const bool isGlobal)
        : TopLevel(Kind::Function), name(std::move(identifier)), isGlobal(isGlobal) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::Function; }

    Function() = delete;
};

struct StaticVariable final : TopLevel {
    const std::string name;
    const std::shared_ptr<Value> value;
    const Type type;
    const bool global;
    StaticVariable(std::string identifier,
                   const std::shared_ptr<Value>& value,
                   const Type ty,
                   const bool isGlobal)
        : TopLevel(Kind::StaticVariable), name(std::move(identifier)),
                value(value), type(ty), global(isGlobal) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::StaticVariable; }

    StaticVariable() = delete;
};

struct StaticArray final : TopLevel {
    const std::string name;
    const std::vector<std::unique_ptr<Initializer>> initializers;
    const Type type;
    const bool global;
    StaticArray(std::string identifier,
                std::vector<std::unique_ptr<Initializer>>&& initializers,
                const Type ty,
                const bool isGlobal)
        : TopLevel(Kind::StaticArray),
          name(std::move(identifier)),
          initializers(std::move(initializers)),
          type(ty),
          global(isGlobal) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::StaticArray; }

    StaticArray() = delete;
};

struct StaticConstant final : TopLevel {
    const Identifier identifier;
    const std::string value;
    const bool global;
    const bool nullTerminated;

    StaticConstant(Identifier identifier, std::string  value, const bool global, const bool nullTerminated)
        : TopLevel(Kind::StaticConstant), identifier(std::move(identifier)),
                                            value(std::move(value)),
                                            global(global),
                                            nullTerminated(nullTerminated){}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::StaticConstant; }

    StaticConstant() = delete;
};

struct Program {
    std::vector<std::unique_ptr<TopLevel>> topLevels;
    std::vector<std::unique_ptr<Value>> values;
    Program() = default;

    Program(Program&&) = default;
    Program& operator=(Program&&) = default;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
};

} // IR