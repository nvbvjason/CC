#pragma once

#include "ShortTypes.hpp"
#include "Types/Type.hpp"
#include "IrType.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

/*

program = Program(top_level*)
top_level = Function(identifier, bool global, identifier params, instruction* body)
          | StaticVariable(identifier, bool global, type t, init)
          | StaticConstant(identifier, type t, static_init)
instruction = Return(val?)
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
            | CopyFromOffset(identifier src, int offset, val dst)
            | Jump(identifier target)
            | JumpIfZero(val condition, identifier target)
            | JumpIfNotZero(val condition, identifier target)
            | Label(identifier)
            | FunCall(identifier fun_name, val* args, val dst?)
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
    enum class Kind : u8 {
        Variable, Constant
    };
    const IrType type;
    const Kind kind;
    Value() = delete;
    virtual ~Value() = default;
protected:
    explicit Value(const IrType type, const Kind k)
        : type(type), kind(k) {}
};

struct ValueVar final : Value {
    ReferingTo referingTo = ReferingTo::Local;
    Identifier value;
    i64 size = 0;

    ValueVar(Identifier v, const IrType t)
        : Value(t, Kind::Variable), value(std::move(v)) {}

    ValueVar(Identifier v, const IrType t, const ReferingTo referingTo)
    : Value(t, Kind::Variable), value(std::move(v)), referingTo(referingTo) {}

    ValueVar(Identifier v, const IrType t, const i64 size)
        : Value(t, Kind::Variable), value(std::move(v)), size(size) {}

    static bool classOf(const Value* value) { return value->kind == Kind::Variable; }

    ValueVar() = delete;
};

struct ValueConst final : Value {
    std::variant<char, i8, u8, i32, i64, u32, u64, double> value;
    explicit ValueConst(const u8 v)
        : Value(u8Type, Kind::Constant), value(v) {}
    explicit ValueConst(const i8 v)
        : Value(i8Type, Kind::Constant), value(v) {}
    explicit ValueConst(const char ch)
        : Value(charType, Kind::Constant), value(ch) {}
    explicit ValueConst(const i32 v)
        : Value(i32Type,Kind::Constant), value(v) {}
    explicit ValueConst(const u32 v)
        : Value(u32Type ,Kind::Constant), value(v) {}
    explicit ValueConst(const i64 v)
        : Value(i64Type,Kind::Constant), value(v) {}
    explicit ValueConst(const u64 v)
        : Value(u64Type, Kind::Constant), value(v) {}
    explicit ValueConst(const double v)
        : Value(doubleType, Kind::Constant), value(v) {}

    static bool classOf(const Value* value) { return value->kind == Kind::Constant; }

    ValueConst() = delete;
};

struct Initializer {
    enum class Kind : u8 {
        Value, Zero
    };
    const Kind kind;

    Initializer() = delete;
protected:

    explicit Initializer(const Kind kind)
        : kind(kind) {}
};

struct ValueInitializer final : Initializer {
    const Value* value;

    explicit ValueInitializer(const Value* value)
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
    enum class Kind : u8 {
        Return,
        SignExtend, Truncate, ZeroExtend,
        DoubleToInt, DoubleToUInt, IntToDouble, UIntToDouble,
        Unary, Binary, Copy, GetAddress, Load, Store,
        AddPtr, CopyToOffset, CopyFromOffset,
        Jump, JumpIfZero, JumpIfNotZero, Label,
        FunCall, Allocate
    };
    const Kind kind;
    const IrType type;

    Instruction() = delete;

    virtual ~Instruction() = default;
protected:
    explicit Instruction(const Kind k, const IrType t)
        : kind(k), type(t) {}
};

struct ReturnInst final : Instruction {
    const Value* returnValue = nullptr;
    const i64 structReturnType = 0;

    explicit ReturnInst(const IrType t)
        : Instruction(Kind::Return, t) {}
    explicit ReturnInst(const Value* v, const IrType t)
        : Instruction(Kind::Return, t), returnValue(std::move(v)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Return; }

    ReturnInst() = delete;
};

struct SignExtendInst final : Instruction {
    const Value* src;
    const Value* dst;
    SignExtendInst(const Value* src,
                   const Value* dst,
                   const IrType t)
        : Instruction(Kind::SignExtend, t), src(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::SignExtend; }

    SignExtendInst() = delete;
};

struct TruncateInst final : Instruction {
    const Value* src;
    const Value* dst;
    TruncateInst(const Value* src,
                 const Value* dst,
                 const IrType t)
        : Instruction(Kind::Truncate, t), src(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Truncate; }

    TruncateInst() = delete;
};

struct ZeroExtendInst final : Instruction {
    const Value* src;
    const Value* dst;
    ZeroExtendInst(const Value* src,
                   const Value* dst,
                   const IrType t)
        : Instruction(Kind::ZeroExtend, t), src(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::ZeroExtend; }

    ZeroExtendInst() = delete;
};

struct DoubleToIntInst final : Instruction {
    const Value* src;
    const Value* dst;
    DoubleToIntInst(const Value* src,
                    const Value* dst,
                    const IrType t)
        : Instruction(Kind::DoubleToInt, t), src(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::DoubleToInt; }

    DoubleToIntInst() = delete;
};

struct DoubleToUIntInst final : Instruction {
    const Value* src;
    const Value* dst;
    DoubleToUIntInst(const Value* src,
                     const Value* dst,
                     const IrType t)
        : Instruction(Kind::DoubleToUInt, t), src(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::DoubleToUInt; }

    DoubleToUIntInst() = delete;
};

struct IntToDoubleInst final : Instruction {
    const Value* src;
    const Value* dst;
    IntToDoubleInst(const Value* src,
                    const Value* dst,
                    const IrType t)
        : Instruction(Kind::IntToDouble, t), src(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::IntToDouble; }

    IntToDoubleInst() = delete;
};

struct UIntToDoubleInst final : Instruction {
    const Value* src;
    const Value* dst;
    UIntToDoubleInst(const Value* src,
                     const Value* dst,
                     const IrType t)
        : Instruction(Kind::UIntToDouble, t), src(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::UIntToDouble; }

    UIntToDoubleInst() = delete;
};

struct UnaryInst final : Instruction {
    enum class Operation {
        Complement, Negate, Not
    };
    Operation operation;
    const Value* src;
    const Value* dst;
    UnaryInst(const Operation op,
              const Value* src,
              const Value* dst,
              const IrType t)
        : Instruction(Kind::Unary, t), operation(op), src(src), dst(dst) {}

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
    const Value* lhs;
    const Value* rhs;
    const Value* dst;
    BinaryInst(const Operation op,
               const Value* src1,
               const Value* src2,
               const Value* dst,
               const IrType t)
        : Instruction(Kind::Binary, t), operation(op), lhs(src1), rhs(src2), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Binary; }

    BinaryInst() = delete;
};

struct CopyInst final : Instruction {
    const Value* src;
    const Value* dst;
    CopyInst(const Value* src, const Value* dst, const IrType t)
        : Instruction(Kind::Copy, t), src(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Copy; }

    CopyInst() = delete;
};

struct GetAddressInst final : Instruction {
    const Value* src;
    const Value* dst;
    GetAddressInst(const Value* src,
                   const Value* dst,
                   const IrType t)
        : Instruction(Kind::GetAddress, t), src(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::GetAddress; }

    GetAddressInst() = delete;
};

struct LoadInst final : Instruction {
    const Value* ptr;
    const Value* dst;
    LoadInst(const Value* src,
             const Value* dst,
             const IrType t)
        : Instruction(Kind::Load, t), ptr(src), dst(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Load; }

    LoadInst() = delete;
};

struct StoreInst final : Instruction {
    const Value* src;
    const Value* ptr;
    StoreInst(const Value* src,
              const Value* dst,
              const IrType t)
        : Instruction(Kind::Store, t), src(src), ptr(dst) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Store; }

    StoreInst() = delete;
};

struct AddPtrInst final : Instruction {
    const Value* ptr;
    const Value* index;
    const Value* dst;
    i64 scale;

    AddPtrInst(const Value* src,
               const Value* index,
               const Value* dst,
               const i64 scale)
        : Instruction(Kind::AddPtr, IrType(IrType::Kind::Pointer, 8)),
          ptr(src),
          index(index),
          dst(dst),
          scale(scale) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::AddPtr; }

    AddPtrInst() = delete;
};

struct CopyToOffsetInst final : Instruction {
    const Value* src;
    const Identifier iden;
    const ReferingTo referingTo;
    const i64 offset;
    const i64 size;
    const i64 alignment;

    CopyToOffsetInst(const Value* src,
                     Identifier iden,
                     const ReferingTo referingTo,
                     const i64 offset,
                     const i64 size,
                     const i64 alignment,
                     const IrType t)
        : Instruction(Kind::CopyToOffset, t), src(src),
          iden(std::move(iden)), referingTo(referingTo), offset(offset), size(size), alignment(alignment) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::CopyToOffset; }

    CopyToOffsetInst() = delete;
};

struct CopyFromOffsetInst final : Instruction {
    const Identifier src;
    const ReferingTo referingTo;
    const Value* dst;
    const i64 offset;

    CopyFromOffsetInst(Identifier iden,
                       const ReferingTo referingTo,
                       const Value* dst,
                       const i64 offset,
                       const IrType t)
    : Instruction(Kind::CopyFromOffset, t), src(std::move(iden)), referingTo(referingTo),
            dst(dst), offset(offset) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::CopyFromOffset; }

    CopyFromOffsetInst() = delete;
};

struct JumpInst final : Instruction {
    Identifier target;
    explicit JumpInst(Identifier target)
        : Instruction(Kind::Jump, IrType(IrType::Kind::I32, 4)), target(std::move(target)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Jump; }

    JumpInst() = delete;
};

struct JumpIfZeroInst final : Instruction {
    const Value* condition;
    Identifier target;
    JumpIfZeroInst(const Value* condition, Identifier target)
        : Instruction(Kind::JumpIfZero, condition->type),
            condition(condition),
            target(std::move(target)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::JumpIfZero; }

    JumpIfZeroInst() = delete;
};

struct JumpIfNotZeroInst final : Instruction {
    const Value* condition;
    Identifier target;
    JumpIfNotZeroInst(const Value* condition, Identifier target)
        : Instruction(Kind::JumpIfNotZero, condition->type),
            condition(condition),
            target(std::move(target)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::JumpIfNotZero; }

    JumpIfNotZeroInst() = delete;
};

struct LabelInst final : Instruction {
    Identifier target;
    explicit LabelInst(Identifier target)
        : Instruction(Kind::Label, IrType(IrType::Kind::I32, 4)), target(std::move(target)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Label; }

    LabelInst() = delete;
};

struct FunCallInst final : Instruction {
    Identifier funName;
    std::vector<const Value*> args;
    const Value* destination = nullptr;

    FunCallInst(Identifier funName,
                std::vector<const Value*>&& args,
                const Value* dst,
                const IrType t)
        : Instruction(Kind::FunCall, t),
            funName(std::move(funName)),
            args(std::move(args)),
            destination(dst) {}

    FunCallInst(Identifier funName,
                std::vector<const Value*>&& args,
                const IrType t)
    : Instruction(Kind::FunCall, t), funName(std::move(funName)), args(std::move(args)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::FunCall; }

    FunCallInst() = delete;
};

struct AllocateInst final : Instruction {
    const i64 size;
    const Identifier iden;

    AllocateInst(const i64 size, Identifier iden)
        : Instruction(Kind::Allocate, u8Type), size(size), iden(std::move(iden)) {}

    static bool classOf(const Instruction* inst) { return inst->kind == Kind::Allocate; }

    AllocateInst() = delete;
};

struct TopLevel {
    enum class Kind : u8 {
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
    std::vector<IrType> argTypes;
    std::vector<std::unique_ptr<Instruction>> insts;
    const std::vector<i64> functions;
    const i64 returnType = 0;

    const bool isGlobal;
    Function(std::string identifier, const bool isGlobal)
        : TopLevel(Kind::Function), name(std::move(identifier)), isGlobal(isGlobal) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::Function; }

    Function() = delete;
};

struct StaticVariable final : TopLevel {
    const std::string name;
    const Value* value;
    const IrType type;
    const bool global;
    StaticVariable(std::string identifier,
                   const Value* value,
                   const IrType ty,
                   const bool isGlobal)
        : TopLevel(Kind::StaticVariable), name(std::move(identifier)),
                value(value), type(ty), global(isGlobal) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::StaticVariable; }

    StaticVariable() = delete;
};

struct StaticArray final : TopLevel {
    const std::string name;
    const std::vector<std::unique_ptr<Initializer>> initializers;
    const bool global;
    StaticArray(std::string identifier,
                std::vector<std::unique_ptr<Initializer>>&& initializers,
                const bool isGlobal)
        : TopLevel(Kind::StaticArray),
          name(std::move(identifier)),
          initializers(std::move(initializers)),
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

struct IrStruct final {
    const std::vector<IrType> types;
    const std::vector<i64> offsets;

    IrStruct(std::vector<IrType>&& types, std::vector<i64>&& offsets)
        : types(std::move(types)), offsets(std::move(offsets)) {}
};

struct Program {
    std::vector<std::unique_ptr<TopLevel>> topLevels;
    std::vector<std::unique_ptr<Value>> values;
    std::unordered_map<std::string, IrStruct> structs;

    Program() = default;

    Program(Program&&) = default;
    Program& operator=(Program&&) = default;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
};

} // IR