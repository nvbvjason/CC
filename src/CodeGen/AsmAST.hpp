#pragma once

#ifndef CC_CODEGEN_ABSTRACTTREE_HPP
#define CC_CODEGEN_ABSTRACTTREE_HPP

#include "ShortTypes.hpp"
#include "Types/Type.hpp"

#include <memory>
#include <utility>
#include <variant>
#include <vector>

/*

program = Program(function_definition)
assembly_type = Byte | Word | Longword | Quadword | Double
top_level = Function(identifier name, bool global, instruction* instructions)
          | StaticVariable(identifier name, bool global, int alignment, int init)
          | StaticConstant(identifier name, int alignment, static init)
instruction = Mov(assembly_type, operand src, operand dst)
            | MovSX(operand src, operand dst)
            | MoveZeroExtend(operand src, operand dst)
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
            | Push(operand)
            | Call(identifier)
            | Ret
unary_operator = Neg | Not | Shr
binary_operator = Add | Sub | Mult |
                  BitwiseOr | BitwiseAnd | BitwiseXor |
                  LeftShiftSigned | RightShiftSigned | LeftShiftUnsigned | RightShiftUnsigned |
                  DivDouble
operand = Imm(int)
        | Reg(reg)
        | Pseudo(identifier)
        | Stack(int)
        | Data(identifier)
cond_code = E | NE | G | GE | L | LE | A | AE | B | BE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP
      XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15

*/

namespace CodeGen {

struct InstVisitor;

enum class AsmType : u8 {
    Byte, Word, LongWord, QuadWord, Double
};

struct Identifier {
    std::string value;
    explicit Identifier(std::string value)
        : value(std::move(value)) {}
};

struct Operand {
    enum class Kind : u8 {
        Imm, Register, Pseudo, Stack, Data
    };
    Kind kind;
    AsmType type;
    const bool isSigned = true;

    virtual ~Operand() = default;

    Operand() = delete;
protected:
    Operand(const Kind k, const AsmType t)
      : kind(k), type(t) {}

    Operand(const Kind k, const AsmType t, const bool isSigned)
        : kind(k), type(t), isSigned(isSigned) {}
};

struct ImmOperand final : Operand {
    std::variant<i32, i64, u32, u64> value;

    explicit ImmOperand(const i64 value)
        : Operand(Kind::Imm, AsmType::QuadWord, true), value(value) {}
    explicit ImmOperand(const i32 value)
        : Operand(Kind::Imm, AsmType::LongWord, true), value(value) {}
    explicit ImmOperand(const u64 value)
        : Operand(Kind::Imm, AsmType::QuadWord, false), value(value) {}
    explicit ImmOperand(const u32 value)
        : Operand(Kind::Imm, AsmType::LongWord, false), value(value) {}

    ImmOperand() = delete;
};

struct RegisterOperand final : Operand {
    enum class Kind : u8 {
        AX, CX, DX, DI, SI, R8, R9, R10, R11, SP,
        XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM14, XMM15
    };
    Kind kind;

    explicit RegisterOperand(const Kind k, const AsmType t)
        : Operand(Operand::Kind::Register, t), kind(k) {}

    RegisterOperand() = delete;
};

struct PseudoOperand final : Operand {
    Identifier identifier;
    ReferingTo referingTo = ReferingTo::Local;
    PseudoOperand(Identifier identifier, const ReferingTo referingTo, const AsmType t)
        : Operand(Kind::Pseudo, t), identifier(std::move(identifier)), referingTo(referingTo) {}

    PseudoOperand() = delete;
};

struct StackOperand final : Operand {
    i32 value;
    StackOperand(const i32 value, const AsmType type)
        : Operand(Kind::Stack, type), value(value) {}

    StackOperand() = delete;
};

struct DataOperand final : Operand {
    Identifier identifier;
    DataOperand(Identifier iden, const AsmType t)
        : Operand(Kind::Data, t), identifier(std::move(iden)) {}

    DataOperand() = delete;
};

struct Inst {
    enum class Kind : u8 {
        Move, MoveSX, MovZeroExtend,
        Cvttsd2si, Cvtsi2sd,
        Unary, Binary, Cmp, Idiv, Div, Cdq, Jmp, JmpCC, SetCC, Label,
        Push, Call, Ret
    };
    enum class CondCode : u8 {
        E, NE, G, GE, L, LE, A, AE, B, BE
    };
    Kind kind;

    virtual ~Inst() = default;

    virtual void accept(InstVisitor& visitor) = 0;

    Inst() = delete;
protected:
    explicit Inst(const Kind k)
        : kind(k) {}
};

struct MoveInst final : Inst {
    std::shared_ptr<Operand> src;
    std::shared_ptr<Operand> dst;
    AsmType type;

    MoveInst(
        std::shared_ptr<Operand> src,
        std::shared_ptr<Operand> dst,
        const AsmType t)
        : Inst(Kind::Move), src(std::move(src)), dst(std::move(dst)), type(t) {}

    void accept(InstVisitor& visitor) override;

    MoveInst() = delete;
};

struct MoveSXInst final : Inst {
    std::shared_ptr<Operand> src;
    std::shared_ptr<Operand> dst;

    MoveSXInst(
        std::shared_ptr<Operand> src,
        std::shared_ptr<Operand> dst)
        : Inst(Kind::MoveSX), src(std::move(src)), dst(std::move(dst)) {}

    void accept(InstVisitor& visitor) override;

    MoveSXInst() = delete;
};

struct MoveZeroExtendInst final : Inst {
    std::shared_ptr<Operand> src;
    std::shared_ptr<Operand> dst;
    AsmType type;

    MoveZeroExtendInst(
        std::shared_ptr<Operand> src,
        std::shared_ptr<Operand> dst,
        const AsmType t)
        : Inst(Kind::MovZeroExtend), src(std::move(src)), dst(std::move(dst)), type(t) {}

    void accept(InstVisitor& visitor) override;

    MoveZeroExtendInst() = delete;
};

struct Cvttsd2siInst final : Inst {
    std::shared_ptr<Operand> src;
    std::shared_ptr<Operand> dst;
    AsmType dstType;

    Cvttsd2siInst(
        std::shared_ptr<Operand> src,
        std::shared_ptr<Operand> dst,
        AsmType dstType)
        : Inst(Kind::Cvttsd2si), src(std::move(src)), dst(std::move(dst)), dstType(dstType) {}

    void accept(InstVisitor& visitor) override;

    Cvttsd2siInst() = delete;
};

struct Cvtsi2sdInst final : Inst {
    std::shared_ptr<Operand> src;
    std::shared_ptr<Operand> dst;
    AsmType srcType;

    Cvtsi2sdInst(
        std::shared_ptr<Operand> src,
        std::shared_ptr<Operand> dst,
        const AsmType srcType)
        : Inst(Kind::Cvttsd2si), src(std::move(src)), dst(std::move(dst)), srcType(srcType) {}

    void accept(InstVisitor& visitor) override;

    Cvtsi2sdInst() = delete;
};

struct UnaryInst final : Inst {
    enum class Operator : u8 {
        Neg, Not, Shr
    };
    std::shared_ptr<Operand> destination;
    Operator oper;
    AsmType type;

    UnaryInst(std::shared_ptr<Operand> dst, const Operator op, const AsmType type)
        : Inst(Kind::Unary), destination(std::move(dst)), oper(op), type(type) {}

    void accept(InstVisitor& visitor) override;

    UnaryInst() = delete;
};

struct BinaryInst final : Inst {
    enum class Operator : u8 {
        Add, Sub, Mul,
        AndBitwise, OrBitwise, BitwiseXor,
        LeftShiftSigned, RightShiftSigned,
        LeftShiftUnsigned, RightShiftUnsigned,
        DivDouble,
    };
    std::shared_ptr<Operand> lhs;
    std::shared_ptr<Operand> rhs;
    Operator oper;
    AsmType type;
    BinaryInst(std::shared_ptr<Operand> lhs,
               std::shared_ptr<Operand> rhs,
               const Operator op,
               const AsmType ty)
        :Inst(Kind::Binary), lhs(std::move(lhs)), rhs(std::move(rhs)), oper(op), type(ty) {}

    void accept(InstVisitor& visitor) override;

    BinaryInst() = delete;
};

struct CmpInst final : Inst {
    std::shared_ptr<Operand> lhs;
    std::shared_ptr<Operand> rhs;
    AsmType type;
    CmpInst(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs, const AsmType ty)
        : Inst(Kind::Cmp), lhs(std::move(lhs)), rhs(std::move(rhs)), type(ty) {}

    void accept(InstVisitor& visitor) override;

    CmpInst() = delete;
};

struct IdivInst final : Inst {
    std::shared_ptr<Operand> operand;
    AsmType type;

    IdivInst(std::shared_ptr<Operand> operand, const AsmType ty)
        : Inst(Kind::Idiv), operand(std::move(operand)), type(ty) {}

    void accept(InstVisitor& visitor) override;

    IdivInst() = delete;
};

struct DivInst final : Inst {
    std::shared_ptr<Operand> operand;
    AsmType type;

    DivInst(std::shared_ptr<Operand> operand, const AsmType ty)
        : Inst(Kind::Div), operand(std::move(operand)), type(ty) {}

    void accept(InstVisitor& visitor) override;

    DivInst() = delete;
};

struct CdqInst final : Inst {
    AsmType type;

    explicit CdqInst(const AsmType ty)
        : Inst(Kind::Cdq), type(ty) {}

    void accept(InstVisitor& visitor) override;
};

struct JmpInst final : Inst {
    Identifier target;
    explicit JmpInst(Identifier target)
        : Inst(Kind::Jmp), target(std::move(target)) {}

    void accept(InstVisitor& visitor) override;

    JmpInst() = delete;
};

struct JmpCCInst final : Inst {
    CondCode condition;
    Identifier target;
    explicit JmpCCInst(const CondCode condition, Identifier target)
        : Inst(Kind::JmpCC), condition(condition), target(std::move(target)) {}

    void accept(InstVisitor& visitor) override;

    JmpCCInst() = delete;
};

struct SetCCInst final : Inst {
    CondCode condition;
    std::shared_ptr<Operand> operand;
    explicit SetCCInst(const CondCode condition, std::shared_ptr<Operand> operand)
        : Inst(Kind::SetCC), condition(condition), operand(std::move(operand)) {}

    void accept(InstVisitor& visitor) override;

    SetCCInst() = delete;
};

struct LabelInst final : Inst {
    Identifier target;
    explicit LabelInst(Identifier target)
        : Inst(Kind::Label), target(std::move(target)) {}

    void accept(InstVisitor& visitor) override;

    LabelInst() = delete;
};

struct PushInst final : Inst {
    std::shared_ptr<Operand> operand;
    explicit PushInst(std::shared_ptr<Operand> operand)
        : Inst(Kind::Push), operand(std::move(operand)) {}

    void accept(InstVisitor& visitor) override;

    PushInst() = delete;
};

struct CallInst final : Inst {
    Identifier funName;
    explicit CallInst(Identifier iden)
        : Inst(Kind::Call), funName(std::move(iden)) {}

    void accept(InstVisitor& visitor) override;

    CallInst() = delete;
};

struct ReturnInst final : Inst {
    ReturnInst()
        : Inst(Kind::Ret) {}

    void accept(InstVisitor& visitor) override;
};

struct TopLevel {
    enum class Kind {
        Function, StaticVariable, StaticConstant,
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
    std::vector<std::unique_ptr<Inst>> instructions;
    i64 stackAlloc = 0;
    const bool isGlobal;
    Function(std::string name, const bool isGlobal)
        : TopLevel(Kind::Function), name(std::move(name)), isGlobal(isGlobal) {}

    Function() = delete;
};

struct StaticVariable : TopLevel {
    std::string name;
    i64 init;
    AsmType type;
    const bool global;
    StaticVariable(std::string name, const AsmType type, const bool isGlobal)
        : TopLevel(Kind::StaticVariable), name(std::move(name)), type(type), global(isGlobal) {}

    StaticVariable() = delete;
};

struct ConstVariable : TopLevel {
    Identifier name;
    i32 alignment;
    double staticInit;

    ConstVariable(Identifier name, const i32 alignment, const double staticInit)
        : TopLevel(Kind::StaticConstant), name(std::move(name)), alignment(alignment), staticInit(staticInit) {}

    ConstVariable() = delete;
};

struct Program {
    std::vector<std::unique_ptr<TopLevel>> topLevels;
};

struct InstVisitor {
    virtual ~InstVisitor() = default;

    virtual void visit(MoveInst&) = 0;
    virtual void visit(MoveSXInst&) = 0;
    virtual void visit(MoveZeroExtendInst&) = 0;
    virtual void visit(Cvttsd2siInst&) = 0;
    virtual void visit(Cvtsi2sdInst&) = 0;
    virtual void visit(UnaryInst&) = 0;
    virtual void visit(BinaryInst&) = 0;
    virtual void visit(CmpInst&) = 0;
    virtual void visit(IdivInst&) = 0;
    virtual void visit(DivInst&) = 0;
    virtual void visit(CdqInst&) = 0;
    virtual void visit(JmpInst&) = 0;
    virtual void visit(JmpCCInst&) = 0;
    virtual void visit(SetCCInst&) = 0;
    virtual void visit(LabelInst&) = 0;
    virtual void visit(PushInst&) = 0;
    virtual void visit(CallInst&) = 0;
    virtual void visit(ReturnInst&) = 0;
};

inline void MoveInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void MoveSXInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void MoveZeroExtendInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void Cvttsd2siInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void Cvtsi2sdInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void UnaryInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void BinaryInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void CmpInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void IdivInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void DivInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void CdqInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void JmpInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void JmpCCInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void SetCCInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void LabelInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void PushInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void CallInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void ReturnInst::accept(InstVisitor& visitor) { visitor.visit(*this); }

}

#endif // CC_CODEGEN_ABSTRACTTREE_HPP