#pragma once

#ifndef CC_CODEGEN_ABSTRACTTREE_HPP
#define CC_CODEGEN_ABSTRACTTREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <utility>
#include <list>

/*

program = Program(function_definition)
function_definition = Function(identifier name, instruciton* instructions)
instruction = Mov(operand src, operand dst)
            | Unary(unary_operator, operand)
            | Binary(binary_operator, operand, operand)
            | Idiv(operand)
            | Cdq
            | Allocate_stack(int)
            | Ret
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult
operand = Imm(int)
        | Reg(reg)
        | Pseudo(identifier)
        | Stack(int)
reg = AX | DX | R10 | R11

*/

namespace CodeGen {

struct Program;
struct Function;
struct Inst;
struct Operand;

struct InstVisitor;

struct Program {
    std::shared_ptr<Function> function;
};

struct Function {
    std::string name;
    std::list<std::shared_ptr<Inst>> instructions;
};

struct Inst {
    enum class Kind {
        Move, Unary, Binary, Idiv, Cdq, AllocateStack, Ret,
        Invalid
    };
    Kind kind = Kind::Invalid;

    virtual ~Inst() = default;

    virtual void accept(InstVisitor& visitor) = 0;

    Inst() = delete;
protected:
    explicit Inst(const Kind k)
        : kind(k) {}
};

struct MoveInst final : Inst {
    std::shared_ptr<Operand> source;
    std::shared_ptr<Operand> destination;

    MoveInst(std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
        : Inst(Kind::Move), source(std::move(src)), destination(std::move(dst)) {}

    void accept(InstVisitor& visitor) override;

    MoveInst() = delete;
};

struct UnaryInst final : Inst {
    enum class Operator {
        Neg, Not,

        Invalid
    };
    Operator oper;
    std::shared_ptr<Operand> destination = nullptr;

    UnaryInst(const Operator op, std::shared_ptr<Operand> dst)
        : Inst(Kind::Unary), oper(op), destination(std::move(dst)) {}

    void accept(InstVisitor& visitor) override;

    UnaryInst() = delete;
};

struct BinaryInst final : Inst {
    enum class Operator {
        Add, Sub, Mul, Div, Mod,
        Invalid
    };
    Operator oper;
    std::shared_ptr<Operand> lhs;
    std::shared_ptr<Operand> rhs;
    BinaryInst(const Operator op, std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs)
        :Inst(Kind::Binary), oper(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void accept(InstVisitor& visitor) override;

    BinaryInst() = delete;
};

struct IdivInst final : Inst {
    std::shared_ptr<Operand> operand;
    explicit IdivInst(std::shared_ptr<Operand> operand)
        : Inst(Kind::Idiv), operand(std::move(operand)) {}

    void accept(InstVisitor& visitor) override;

    IdivInst() = delete;
};

struct CdqInst final : Inst {
    explicit CdqInst()
        : Inst(Kind::Cdq) {}

    void accept(InstVisitor& visitor) override;
};

struct AllocStackInst final : Inst {
    i32 alloc;
    explicit AllocStackInst(i32 alloc)
        : Inst(Kind::AllocateStack), alloc(alloc) {}

    void accept(InstVisitor& visitor) override;

    AllocStackInst() = delete;
};

struct ReturnInst final : Inst {
    ReturnInst()
        : Inst(Kind::Ret) {}

    void accept(InstVisitor& visitor) override;
};

struct Operand {
    enum class Kind {
        Imm, Register, Pseudo, Stack,
        Invalid
    };
    Kind kind;

    virtual ~Operand() = default;

    explicit Operand(const Kind k)
        : kind(k) {}

    Operand() = delete;
};

struct ImmOperand final : Operand {
    i32 value;
    explicit ImmOperand(const i32 value)
        : Operand(Kind::Imm), value(value) {}

    ImmOperand() = delete;
};

struct RegisterOperand final : Operand {
    enum class Kind {
        AX, DX, R10, R11,
        Invalid
    };
    Kind kind;
    explicit RegisterOperand(const Kind k)
        : Operand(Operand::Kind::Register), kind(k) {}

    RegisterOperand() = delete;
};

struct PseudoOperand final : Operand {
    std::string identifier;
    explicit PseudoOperand(std::string identifier)
        : Operand(Kind::Pseudo), identifier(std::move(identifier)) {}

    PseudoOperand() = delete;
};

struct StackOperand final : Operand {
    i32 value;
    explicit StackOperand(const i32 value)
        : Operand(Kind::Stack), value(value) {}

    StackOperand() = delete;
};

struct InstVisitor {
    virtual ~InstVisitor() = default;

    virtual void visit(MoveInst&) = 0;
    virtual void visit(UnaryInst&) = 0;
    virtual void visit(BinaryInst&) = 0;
    virtual void visit(IdivInst&) = 0;
    virtual void visit(CdqInst&) = 0;
    virtual void visit(AllocStackInst&) = 0;
    virtual void visit(ReturnInst&) = 0;
};

inline void MoveInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void UnaryInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void BinaryInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void IdivInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void CdqInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void AllocStackInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void ReturnInst::accept(InstVisitor& visitor) { visitor.visit(*this); }

}

#endif // CC_CODEGEN_ABSTRACTTREE_HPP
