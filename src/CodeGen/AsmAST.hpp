#pragma once

#ifndef CC_CODEGEN_ABSTRACTTREE_HPP
#define CC_CODEGEN_ABSTRACTTREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "Types/Type.hpp"

/*

program = Program(function_definition)
assembly_type = Longword | Quadword
top_level = Function(identifier name, bool global, instruction* instructions)
          | StaticVariable(identifier name, bool global, int alignment, int init)
instruction = Mov(assembly_type, operand src, operand dst)
            | Movsx(operand src, operand dst)
            | Unary(unary_operator, assembly_type, operand)
            | Binary(binary_operator, assembly_type, operand, operand)
            | Cmp(operand, operand)
            | Idiv(assembly_type, operand)
            | Cdq(assembly_type)
            | Jmp(identifier)
            | JmpCC(cond_code, identifier)
            | SetCC(cond_code, operand)
            | Label(identifier)
            | Push(operand)
            | Call(identifier)
            | Ret
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult |
                  BitwiseOr | BitwiseAnd | BitwiseXor |
                  LeftShift | RightShift
operand = Imm(int)
        | Reg(reg)
        | Pseudo(identifier)
        | Stack(int)
        | Data(identifier)
cond_code = E | NE | G | GE | L | LE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP

*/

namespace CodeGen {

struct InstVisitor;

enum class AssemblyType : u8 {
    Byte, Word, LongWord, QuadWord
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
    AssemblyType type;

    virtual ~Operand() = default;

    Operand() = delete;
protected:
    explicit Operand(const Kind k)
        : kind(k) {}
    Operand(const Kind k, const AssemblyType t)
    : kind(k), type(t) {}
};

struct ImmOperand final : Operand {
    std::variant<i32, i64> value;

    ImmOperand(const AssemblyType t, const i64 v)
    : Operand(Kind::Imm, t), value(v) {}
    explicit ImmOperand(const i64 value)
        : Operand(Kind::Imm, AssemblyType::QuadWord), value(value) {}

    ImmOperand(const AssemblyType t, const i32 v)
        : Operand(Kind::Imm, t), value(v) {}
    explicit ImmOperand(const i32 value)
        : Operand(Kind::Imm, AssemblyType::LongWord), value(value) {}

    ImmOperand() = delete;
};

struct RegisterOperand final : Operand {
    enum class Kind : u8 {
        AX, CX, DX, DI, SI, R8, R9, R10, R11, SP
    };
    Kind kind;

    explicit RegisterOperand(const Kind k, const AssemblyType t)
        : Operand(Operand::Kind::Register, t), kind(k) {}

    RegisterOperand() = delete;
};

struct PseudoOperand final : Operand {
    std::string identifier;
    ReferingTo referingTo = ReferingTo::Local;
    PseudoOperand(std::string identifier, ReferingTo referingTo)
        : Operand(Kind::Pseudo), identifier(std::move(identifier)), referingTo(referingTo) {}

    PseudoOperand() = delete;
};

struct StackOperand final : Operand {
    i32 value;
    explicit StackOperand(const i32 value)
        : Operand(Kind::Stack), value(value) {}

    StackOperand() = delete;
};

struct DataOperand final : Operand {
    std::string identifier;
    explicit DataOperand(std::string iden)
        : Operand(Kind::Data), identifier(std::move(iden)) {}

    DataOperand() = delete;
};

struct Inst {
    enum class Kind : u8 {
        Move, MoveSX, Unary, Binary, Cmp, Idiv, Cdq, Jmp, JmpCC, SetCC, Label,
        Push, Call, Ret
    };
    enum class CondCode : u8 {
        E, NE, G, GE, L, LE
    };
    Kind kind;

    virtual ~Inst() = default;

    virtual void accept(InstVisitor& visitor) = 0;

    Inst() = delete;
protected:
    explicit Inst(const Kind k)
        : kind(k) {}
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

struct MoveInst final : Inst {
    std::shared_ptr<Operand> src;
    std::shared_ptr<Operand> dst;
    AssemblyType type;

    MoveInst(
        std::shared_ptr<Operand> src,
        std::shared_ptr<Operand> dst,
        const AssemblyType t)
        : Inst(Kind::Move), src(std::move(src)), dst(std::move(dst)), type(t) {}

    void accept(InstVisitor& visitor) override;

    MoveInst() = delete;
};

struct UnaryInst final : Inst {
    enum class Operator : u8 {
        Neg, Not
    };
    std::shared_ptr<Operand> destination;
    Operator oper;
    AssemblyType type;

    UnaryInst(std::shared_ptr<Operand> dst, const Operator op, const AssemblyType type)
        : Inst(Kind::Unary), destination(std::move(dst)), oper(op), type(type) {}

    void accept(InstVisitor& visitor) override;

    UnaryInst() = delete;
};

struct BinaryInst final : Inst {
    enum class Operator : u8 {
        Add, Sub, Mul,
        BitwiseAnd, BitwiseOr, BitwiseXor,
        LeftShift, RightShift,
    };
    std::shared_ptr<Operand> lhs;
    std::shared_ptr<Operand> rhs;
    Operator oper;
    AssemblyType type;
    BinaryInst(std::shared_ptr<Operand> lhs,
               std::shared_ptr<Operand> rhs,
               const Operator op,
               const AssemblyType ty)
        :Inst(Kind::Binary), lhs(std::move(lhs)), rhs(std::move(rhs)), oper(op), type(ty) {}

    void accept(InstVisitor& visitor) override;

    BinaryInst() = delete;
};

struct CmpInst final : Inst {
    std::shared_ptr<Operand> lhs;
    std::shared_ptr<Operand> rhs;
    AssemblyType type;
    CmpInst(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs, const AssemblyType ty)
        : Inst(Kind::Cmp), lhs(std::move(lhs)), rhs(std::move(rhs)), type(ty) {}

    void accept(InstVisitor& visitor) override;

    CmpInst() = delete;
};

struct IdivInst final : Inst {
    std::shared_ptr<Operand> operand;
    AssemblyType type;

    IdivInst(std::shared_ptr<Operand> operand, const AssemblyType ty)
        : Inst(Kind::Idiv), operand(std::move(operand)), type(ty) {}

    void accept(InstVisitor& visitor) override;

    IdivInst() = delete;
};

struct CdqInst final : Inst {
    AssemblyType type;

    explicit CdqInst(const AssemblyType ty)
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

struct Function : TopLevel {
    std::string name;
    std::vector<std::unique_ptr<Inst>> instructions;
    i64 stackAlloc = 0;
    const bool isGlobal;
    Function(std::string name, const bool isGlobal)
        : TopLevel(Type::Function), name(std::move(name)), isGlobal(isGlobal) {}

    Function() = delete;
};

struct StaticVariable : TopLevel {
    std::string name;
    i64 init;
    AssemblyType type;
    const bool global;
    StaticVariable(std::string name, const i64 init, const AssemblyType type, const bool isGlobal)
        : TopLevel(Type::StaticVariable), name(std::move(name)), init(init), type(type), global(isGlobal) {}

    StaticVariable() = delete;
};

struct Program {
    std::vector<std::unique_ptr<TopLevel>> topLevels;
};

struct InstVisitor {
    virtual ~InstVisitor() = default;

    virtual void visit(MoveInst&) = 0;
    virtual void visit(MoveSXInst&) = 0;
    virtual void visit(UnaryInst&) = 0;
    virtual void visit(BinaryInst&) = 0;
    virtual void visit(CmpInst&) = 0;
    virtual void visit(IdivInst&) = 0;
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
inline void UnaryInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void BinaryInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void CmpInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void IdivInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
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
