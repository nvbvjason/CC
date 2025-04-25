#pragma once

#ifndef CC_CODEGEN_ABSTRACTTREE_HPP
#define CC_CODEGEN_ABSTRACTTREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <utility>
#include <vector>

/*

program = Program(function_definition)
function_definition = Function(identifier name, instruciton* instructions)
instruction = Mov(operand src, operand dst)
            | Unary(unary_operator, operand)
            | Allocate_stack(int)
            | Ret
unary_operator = Neg | Not
operand = Imm(int)
        | Reg(reg)
        | Pseudo(identifier)
        | Stack(int)
reg = AX | R10

*/

namespace CodeGen {

struct Program;
struct Function;
struct Inst;
struct Operand;

struct Program {
    std::shared_ptr<Function> function;
};

struct Function {
    std::string name;
    std::vector<std::shared_ptr<Inst>> instructions;
};

struct Inst {
    enum class Kind {
        Move, Unary, AllocateStack, Ret,

        Invalid
    };
    Kind kind = Kind::Invalid;

    virtual ~Inst() = default;

    explicit Inst(const Kind k)
        : kind(k) {}

    Inst() = delete;
};

struct MoveInst final : Inst {
    std::shared_ptr<Operand> source;
    std::shared_ptr<Operand> destination;

    MoveInst(std::shared_ptr<Operand>&& src, std::shared_ptr<Operand>&& dst)
        : Inst(Kind::Move), source(std::move(src)), destination(std::move(dst)) {}

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

    UnaryInst() = delete;
};

struct InstAllocStack final : Inst {
    i32 alloc;
    explicit InstAllocStack(i32 alloc)
        : Inst(Kind::AllocateStack), alloc(alloc) {}

    InstAllocStack() = delete;
};

struct InstRet final : Inst {
    InstRet()
        : Inst(Kind::Ret) {}
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

struct OperandImm final : Operand {
    i32 value;
    explicit OperandImm(const i32 value)
        : Operand(Kind::Imm), value(value) {}

    OperandImm() = delete;
};

struct OperandRegister final : Operand {
    enum class Kind {
        AX, R10,

        Invalid
    };
    Kind kind;
    explicit OperandRegister(const Kind k)
        : Operand(Operand::Kind::Register), kind(k) {}

    OperandRegister() = delete;
};

struct OperandPseudo final : Operand {
    std::string identifier;
    explicit OperandPseudo(std::string identifier)
        : Operand(Kind::Pseudo), identifier(std::move(identifier)) {}

    OperandPseudo() = delete;
};

struct OperandStack final : Operand {
    i32 value;
    explicit OperandStack(const i32 value)
        : Operand(Kind::Stack), value(value) {}

    OperandStack() = delete;
};

}

#endif // CC_CODEGEN_ABSTRACTTREE_HPP
