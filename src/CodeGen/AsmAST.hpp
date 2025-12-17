#pragma once

#include "ShortTypes.hpp"
#include "Types/Type.hpp"

#include <memory>
#include <utility>
#include <vector>

/*

program = Program(function_definition)
assembly_type = Byte | Word | Longword | Quadword | Double | ByteArray(int size, int alignment)
top_level = Function(identifier name, bool global, instruction* instructions)
          | StaticVariable(identifier name, bool global, int alignment, int init)
          | StaticConstant(identifier name, int alignment, static init)
instruction = Mov(assembly_type, operand src, operand dst)
            | MovSX(operand src, operand dst)
            | MoveZeroExtend(operand src, operand dst)
            | Lea(operand src, operand dst)
            | Cvttsd2si(assembly_type, dst_type, operand src, operand dst)
            | Cvtsi2sd(assembly_type, src_type, operand src, operand dst)
            | Unary(unary_operator, assembly_type, operand)
            | Binary(binary_operator, assembly_type, operand, operand)
            | Cmp(operand, operand)
            | Idiv(assembly_type, operand)
            | Div(assembly_type, operand)
            | Cdq(assembly_type)
            | Jmp(identifier)
            | JmpCC(cond_code, identifier)
            | SetCC(cond_code, operand)
            | Label(identifier)
            | PseudoPush(Identifier, size, alignment)
            | Push(operand)
            | Call(identifier)
            | Ret
unary_operator = Neg | Not | Shr
binary_operator = Add | Sub | Mult
                | BitwiseOr | BitwiseAnd | BitwiseXor
                | LeftShiftSigned | RightShiftSigned | LeftShiftUnsigned | RightShiftUnsigned
                | DivDouble
operand = Imm(int)
        | Reg(reg)
        | Pseudo(identifier)
        | Memory(reg, int)
        | Data(identifier)
        | PseudoMem(Identifier, int)
        | Indexed(reg base, reg index, int scale)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | BP
    | XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15

*/

namespace CodeGen {

struct AsmType final {
    enum class Kind : u8 {
        Byte, Word, LongWord, QuadWord, Double, ByteArray
    };
    const Kind kind;
    const i64 size;

    constexpr explicit AsmType(const Kind kind, const i64 size)
        : kind(kind), size(size) {}

    AsmType() = delete;
};

inline bool operator==(const AsmType& lhs, const AsmType& rhs)
{
    return lhs.size == rhs.size && lhs.kind == rhs.kind;
}

struct Identifier {
    std::string value;
    explicit Identifier(std::string value)
        : value(std::move(value)) {}
};

struct Operand {
    enum class Kind : u8 {
        Imm, Register, Pseudo, Memory, Data, PseudoMem, Indexed
    };
    enum class RegKind : u8 {
        AX, CX, DX, DI, SI, R8, R9, R10, R11, SP, BP,
        XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM14, XMM15
    };
    const Kind kind;
    const AsmType type;
    const bool isSigned = true;

    virtual ~Operand() = default;

    Operand() = delete;
protected:
    explicit Operand(const Kind k, const AsmType asmType)
      : kind(k), type(asmType) {}

    Operand(const Kind k, const AsmType asmType, const bool isSigned)
        : kind(k), type(asmType), isSigned(isSigned) {}
};

struct ImmOperand final : Operand {
    const u64 value;

    explicit ImmOperand(const u64 value, const AsmType asmType)
        : Operand(Kind::Imm, asmType, false), value(value) {}

    static bool classOf(const Operand* operand) { return operand->kind == Kind::Imm; }

    ImmOperand() = delete;
};

struct RegisterOperand final : Operand {
    const RegKind regKind;

    explicit RegisterOperand(const RegKind rK, const AsmType asmType)
        : Operand(Kind::Register, asmType), regKind(rK) {}

    static bool classOf(const Operand* operand) { return operand->kind == Kind::Register; }

    RegisterOperand() = delete;
};

struct PseudoOperand final : Operand {
    Identifier identifier;
    const ReferringTo referringTo = ReferringTo::Local;
    const bool local;

    PseudoOperand(Identifier identifier, const ReferringTo referringTo, const AsmType asmType, const bool local)
        : Operand(Kind::Pseudo, asmType), identifier(std::move(identifier)),
          referringTo(referringTo), local(local) {}

    static bool classOf(const Operand* operand) { return operand->kind == Kind::Pseudo; }

    PseudoOperand() = delete;
};

struct MemoryOperand final : Operand {
    const RegKind regKind;
    const i64 value;
    const i64 offset = 0;

    MemoryOperand(const RegKind rK, const i64 value, const AsmType type)
        : Operand(Kind::Memory, type), regKind(rK), value(value) {}

    MemoryOperand(const RegKind rK, const i64 value, const AsmType type, const i64 offset)
        : Operand(Kind::Memory, type), regKind(rK), value(value), offset(offset) {}

    static bool classOf(const Operand* operand) { return operand->kind == Kind::Memory; }

    MemoryOperand() = delete;
};

struct DataOperand final : Operand {
    const Identifier identifier;
    const i64 offset;
    const bool local;
    const bool isRoData = false;

    DataOperand(const AsmType asmType, const i64 offset, Identifier iden, const bool local)
        : Operand(Kind::Data, asmType), identifier(std::move(iden)), offset(offset), local(local) {}

    DataOperand(
        const AsmType asmType,
        const i64 offset,
        Identifier iden,
        const bool local,
        const bool isRoData)
    : Operand(Kind::Data, asmType),
        identifier(std::move(iden)),
        offset(offset),
        local(local),
        isRoData(isRoData) {}

    static bool classOf(const Operand* operand) { return operand->kind == Kind::Data; }

    DataOperand() = delete;
};

struct PseudoMemOperand final : Operand {
    const Identifier identifier;
    const i64 offset;
    const i64 size = 0;
    const i64 alignment;
    const ReferringTo referringTo = ReferringTo::Local;
    const bool local;

    PseudoMemOperand(Identifier identifier,
                     const i64 offset,
                     const i64 size,
                     const i64 alignment,
                     const bool local,
                     const AsmType type)
        : Operand(Kind::PseudoMem, type),
            identifier(std::move(identifier)),
            offset(offset),
            size(size),
            alignment(alignment),
            local(local) {}

    PseudoMemOperand(Identifier identifier,
                     const i64 offset,
                     const i64 size,
                     const i64 alignment,
                     const bool local,
                     const AsmType type,
                     const ReferringTo referringTo)
        : Operand(Kind::PseudoMem, type),
            identifier(std::move(identifier)),
            offset(offset),
            size(size),
            alignment(alignment),
            referringTo(referringTo),
            local(local) {}

    static bool classOf(const Operand* operand) { return operand->kind == Kind::PseudoMem; }

    PseudoMemOperand() = delete;
};

struct IndexedOperand final : Operand {
    const RegKind regKind;
    const RegKind indexRegKind;
    const i64 scale;

    IndexedOperand(const RegKind rK, const RegKind indexRegKind, const i64 scale, const AsmType asmType)
        : Operand(Kind::Indexed, asmType), regKind(rK), indexRegKind(indexRegKind), scale(scale) {}

    static bool classOf(const Operand* operand) { return operand->kind == Kind::Indexed; }

    IndexedOperand() = delete;
};

struct Initializer {
    enum class Kind : u8 {
        Zero, Value
    };
    const Kind kind;

    Initializer() = delete;

    virtual ~Initializer() = default;
protected:
    explicit Initializer(const Kind kind)
        : kind(kind) {}
};

struct ZeroInitializer final : Initializer {
    const i64 size;

    explicit ZeroInitializer(const i64 size)
        : Initializer(Kind::Zero), size(size) {}

    static bool classOf(const Initializer* initializer) { return initializer->kind == Kind::Zero; }

    ZeroInitializer() = delete;
};

struct ValueInitializer final : Initializer {
    const Operand* init;

    explicit ValueInitializer(const Operand* init)
        : Initializer(Kind::Value), init(init) {}

    static bool classOf(const Initializer* initializer) { return initializer->kind == Kind::Value; }

    ValueInitializer() = delete;
};

struct Inst {
    enum class Kind : u8 {
        Move, MoveSX, MoveZeroExtend, Lea,
        Cvttsd2si, Cvtsi2sd,
        Unary, Binary, Cmp, Idiv, Div, Cdq, Jmp, JmpCC, SetCC, Label,
        PushPseudo, Push, Call, Ret
    };
    enum class CondCode : u8 {
        E, NE, G, GE, L, LE, A, AE, B, BE, PF
    };
    const Kind kind;

    virtual ~Inst() = default;

    Inst() = delete;
protected:
    explicit Inst(const Kind k)
        : kind(k) {}
};

struct MoveInst final : Inst {
    const Operand* src;
    const Operand* dst;
    const AsmType type;

    MoveInst(
        const Operand* src,
        const Operand* dst,
        const AsmType t)
        : Inst(Kind::Move), src(src), dst(dst), type(t) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Move; }

    MoveInst() = delete;
};

struct MoveSXInst final : Inst {
    const Operand* src;
    const Operand* dst;
    const AsmType srcType;
    const AsmType dstType;

    MoveSXInst(
        const Operand* src,
        const Operand* dst,
        const AsmType srcType,
        const AsmType dstType)
        : Inst(Kind::MoveSX), src(src), dst(dst), srcType(srcType), dstType(dstType) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::MoveSX; }

    MoveSXInst() = delete;
};

struct MoveZeroExtendInst final : Inst {
    const Operand* src;
    const Operand* dst;
    const AsmType srcType;
    const AsmType dstType;

    MoveZeroExtendInst(
        const Operand* src,
        const Operand* dst,
        const AsmType srcType,
        const AsmType dstType)
        : Inst(Kind::MoveZeroExtend), src(src),
                                        dst(dst),
                                        srcType(srcType),
                                        dstType(dstType) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::MoveZeroExtend; }

    MoveZeroExtendInst() = delete;
};

struct LeaInst final : Inst {
    const Operand* src;
    const Operand* dst;
    const AsmType type;

    LeaInst(const Operand* src, const Operand* dst, const AsmType t)
        : Inst(Kind::Lea), src(src), dst(dst), type(t) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Lea; }

    LeaInst() = delete;
};

struct Cvttsd2siInst final : Inst {
    const Operand* src;
    const Operand* dst;
    const AsmType dstType;

    Cvttsd2siInst(
        const Operand* src,
        const Operand* dst,
        const AsmType dstType)
        : Inst(Kind::Cvttsd2si), src(src), dst(dst), dstType(dstType) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Cvttsd2si; }

    Cvttsd2siInst() = delete;
};

struct Cvtsi2sdInst final : Inst {
    const Operand* src;
    const Operand* dst;
    const AsmType srcType;

    Cvtsi2sdInst(const Operand* src, const Operand* dst, const AsmType srcType)
        : Inst(Kind::Cvtsi2sd), src(src), dst(dst), srcType(srcType) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Cvtsi2sd; }

    Cvtsi2sdInst() = delete;
};

struct UnaryInst final : Inst {
    enum class Operator : u8 {
        Neg, Not, Shr
    };
    const Operand* dst;
    const Operator oper;
    const AsmType type;

    UnaryInst(const Operand* dst, const Operator op, const AsmType type)
        : Inst(Kind::Unary), dst(dst), oper(op), type(type) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Unary; }

    UnaryInst() = delete;
};

struct BinaryInst final : Inst {
    enum class Operator : u8 {
        Add, Sub, Mul,
        BitwiseAnd, BitwiseOr, BitwiseXor,
        LeftShiftSigned, RightShiftSigned,
        LeftShiftUnsigned, RightShiftUnsigned,
        DivDouble,
    };
    const Operand* lhs;
    const Operand* rhs;
    const Operator oper;
    const AsmType type;
    BinaryInst(const Operand* lhs,
               const Operand* rhs,
               const Operator op,
               const AsmType ty)
        : Inst(Kind::Binary), lhs(lhs), rhs(rhs), oper(op), type(ty) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Binary; }

    BinaryInst() = delete;
};

struct CmpInst final : Inst {
    const Operand* lhs;
    const Operand* rhs;
    const AsmType type;
    CmpInst(const Operand* lhs, const Operand* rhs, const AsmType ty)
        : Inst(Kind::Cmp), lhs(lhs), rhs(rhs), type(ty) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Cmp; }

    CmpInst() = delete;
};

struct IdivInst final : Inst {
    const Operand* operand;
    const AsmType type;

    IdivInst(const Operand* operand, const AsmType ty)
        : Inst(Kind::Idiv), operand(operand), type(ty) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Idiv; }

    IdivInst() = delete;
};

struct DivInst final : Inst {
    const Operand* operand;
    const AsmType type;

    DivInst(const Operand* operand, const AsmType ty)
        : Inst(Kind::Div), operand(operand), type(ty) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Div; }

    DivInst() = delete;
};

struct CdqInst final : Inst {
    const AsmType type;

    explicit CdqInst(const AsmType ty)
        : Inst(Kind::Cdq), type(ty) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Cdq; }
};

struct JmpInst final : Inst {
    const Identifier target;
    explicit JmpInst(Identifier target)
        : Inst(Kind::Jmp), target(std::move(target)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Jmp; }

    JmpInst() = delete;
};

struct JmpCCInst final : Inst {
    const CondCode condition;
    const Identifier target;
    explicit JmpCCInst(const CondCode condition, Identifier target)
        : Inst(Kind::JmpCC), condition(condition), target(std::move(target)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::JmpCC; }

    JmpCCInst() = delete;
};

struct SetCCInst final : Inst {
    const Operand* operand;
    const CondCode condition;
    explicit SetCCInst(const CondCode condition, const Operand* operand)
        : Inst(Kind::SetCC), operand(operand), condition(condition) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::SetCC; }

    SetCCInst() = delete;
};

struct LabelInst final : Inst {
    const Identifier target;
    explicit LabelInst(Identifier target)
        : Inst(Kind::Label), target(std::move(target)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Label; }

    LabelInst() = delete;
};

struct PushPseudoInst final : Inst {
    const i64 size;
    const i64 alignment;
    const AsmType type;
    const Identifier identifier;

    PushPseudoInst(const i64 size, const i64 alignment, const AsmType type, Identifier identifier)
        : Inst(Kind::PushPseudo), size(size), alignment(alignment),
                                    type(type), identifier(std::move(identifier)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::PushPseudo; }

    PushPseudoInst() = delete;
};

struct PushInst final : Inst {
    const Operand* operand;
    explicit PushInst(const Operand* operand)
        : Inst(Kind::Push), operand(operand) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Push; }

    PushInst() = delete;
};

struct CallInst final : Inst {
    const Identifier funName;
    explicit CallInst(Identifier iden)
        : Inst(Kind::Call), funName(std::move(iden)) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Call; }

    CallInst() = delete;
};

struct ReturnInst final : Inst {
    ReturnInst()
        : Inst(Kind::Ret) {}

    static bool classOf(const Inst* inst) { return inst->kind == Kind::Ret; }
};

struct TopLevel {
    enum class Kind : u8 {
        Function, StaticVariable, StaticConstant, StaticCompound, StaticString
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
    std::vector<std::unique_ptr<Inst>> instructions;
    i64 stackAlloc = 0;
    const bool isGlobal;
    Function(std::string name, const bool isGlobal)
        : TopLevel(Kind::Function), name(std::move(name)), isGlobal(isGlobal) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::Function; }

    Function() = delete;
};

struct StaticVariable final : TopLevel {
    std::string name;
    const Operand* init = nullptr;
    const AsmType type;
    const bool global;

    StaticVariable(std::string name, const AsmType type, const bool isGlobal)
        : TopLevel(Kind::StaticVariable), name(std::move(name)), type(type), global(isGlobal) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::StaticVariable; }

    StaticVariable() = delete;
};

struct ConstVariable final : TopLevel {
    Identifier name;
    const double staticInit;
    const i32 alignment;
    const bool local;

    ConstVariable(Identifier name, const double staticInit, const i32 alignment, const bool local)
        : TopLevel(Kind::StaticConstant), name(std::move(name)), staticInit(staticInit),
                                            alignment(alignment), local(local) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::StaticConstant; }

    ConstVariable() = delete;
};

struct CompoundVariable final : TopLevel {
    Identifier name;
    std::vector<std::unique_ptr<Initializer>> initializers;
    i32 alignment;
    const bool isGlobal;

    CompoundVariable(Identifier name,
                  std::vector<std::unique_ptr<Initializer>>&& initializers,
                  const i32 alignment,
                  const bool local)
        : TopLevel(Kind::StaticCompound), name(std::move(name)), initializers(std::move(initializers)),
                                         alignment(alignment),
                                         isGlobal(local) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::StaticCompound; }

    CompoundVariable() = delete;
};

struct StringVariable final : TopLevel {
    const std::string name;
    const std::string value;
    const bool global;
    const bool nullTerminated;

    StringVariable(std::string name, std::string value, const bool global, const bool nullTerminated)
        : TopLevel(Kind::StaticString), name(std::move(name)), value(std::move(value)),
                                         global(global), nullTerminated(nullTerminated) {}

    static bool classOf(const TopLevel* topLevel) { return topLevel->kind == Kind::StaticString; }

    StringVariable() = delete;
};

struct Program {
    std::vector<std::unique_ptr<TopLevel>> topLevels;
    std::vector<std::unique_ptr<Operand>> operands;

    Program() = default;
    Program(Program&& other) noexcept
        : topLevels(std::move(other.topLevels)), operands(std::move(other.operands)) {}

    const Operand* getImmOperand(u64 value, AsmType type);
    const Operand* getPseudoOperand(const Identifier& identifier, ReferringTo referringTo, AsmType asmType, bool local);
    const Operand* getPseudoMemOperand(const Identifier& identifier,
                                       i64 offset,
                                       i64 size,
                                       i64 alignment,
                                       bool local,
                                       AsmType type);
    const Operand* getPseudoMemOperand(Identifier identifier,
                                       i64 offset,
                                       i64 size,
                                       i64 alignment,
                                       bool local,
                                       AsmType type,
                                       ReferringTo referringTo);
    const Operand* getMemoryOperand(RegisterOperand::RegKind rK, i64 value, AsmType type);
    const Operand* getMemoryOperand(RegisterOperand::RegKind rK, i64 value, AsmType type, i64 offset);
    const Operand* getRegisterOperand(RegisterOperand::RegKind regType, const AsmType& type);
    const Operand* getIndexedOperand(
        RegisterOperand::RegKind rK, RegisterOperand::RegKind indexRegKind, i64 scale, AsmType asmType);
    const Operand* getDataOperand(AsmType asmType, i64 offset, const Identifier& iden, bool local);
    const Operand* getDataOperand(
        AsmType asmType,
        i64 offset,
        const Identifier& iden,
        bool local,
        bool isRoData);
};

inline const Operand* Program::getImmOperand(u64 value, AsmType type)
{
    operands.emplace_back(std::make_unique<ImmOperand>(value, type));
    return operands.back().get();
}

inline const Operand* Program::getPseudoOperand(const Identifier& identifier, ReferringTo referringTo, AsmType asmType,
    bool local)
{
    operands.emplace_back(std::make_unique<PseudoOperand>(identifier, referringTo, asmType, local));
    return operands.back().get();
}

inline const Operand* Program::getPseudoMemOperand(const Identifier& identifier, i64 offset, i64 size, i64 alignment,
    bool local, AsmType type)
{
    operands.emplace_back(std::make_unique<PseudoMemOperand>(
        identifier, offset, size, alignment, local, type));
    return operands.back().get();
}

inline const Operand* Program::getPseudoMemOperand(
    Identifier identifier, i64 offset, i64 size, i64 alignment,
    bool local, AsmType type, ReferringTo referringTo)
{
    operands.emplace_back(std::make_unique<PseudoMemOperand>(
        identifier, offset, size, alignment, local, type, referringTo));
    return operands.back().get();
}

inline const Operand* Program::getMemoryOperand(RegisterOperand::RegKind rK, i64 value, AsmType type)
{
    operands.emplace_back(std::make_unique<MemoryOperand>(rK, value, type));
    return operands.back().get();
}

inline const Operand* Program::getMemoryOperand(RegisterOperand::RegKind rK, i64 value, AsmType type, i64 offset)
{
    operands.emplace_back(std::make_unique<MemoryOperand>(rK, value, type, offset));
    return operands.back().get();
}

inline const Operand* Program::getRegisterOperand(const RegisterOperand::RegKind regType, const AsmType& type)
{
    operands.emplace_back(std::make_unique<RegisterOperand>(regType, type));
    return operands.back().get();
}

inline const Operand* Program::getIndexedOperand(
    RegisterOperand::RegKind rK, RegisterOperand::RegKind indexRegKind,
    i64 scale, AsmType asmType)
{
    operands.emplace_back(std::make_unique<IndexedOperand>(rK, indexRegKind, scale, asmType));
    return operands.back().get();
}

inline const Operand* Program::getDataOperand(AsmType asmType, i64 offset, const Identifier& iden, bool local)
{
    operands.emplace_back(std::make_unique<DataOperand>(asmType, offset, iden, local));
    return operands.back().get();
}

inline const Operand* Program::getDataOperand(
        AsmType asmType,
        i64 offset,
        const Identifier& iden,
        bool local,
        bool isRoData
)
{
    operands.emplace_back(std::make_unique<DataOperand>(asmType, offset, iden, local, isRoData));
    return operands.back().get();
}
} // CodeGen