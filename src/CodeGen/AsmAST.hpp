#pragma once

#ifndef CC_CODEGEN_ABSTRACTTREE_HPP
#define CC_CODEGEN_ABSTRACTTREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <utility>
#include <vector>

/*

program = Program(function_definition)
top_level = Function(identifier name, bool global, instruction* instructions)
          | StaticVariable(identifier name, bool global, int init)
instruction = Mov(operand src, operand dst)
            | Unary(unary_operator, operand)
            | Binary(binary_operator, operand, operand)
            | Cmp(operand, operand)
            | Idiv(operand)
            | Cdq
            | Jmp(identifier)
            | JmpCC(cond_code, identifier)
            | SetCC(cond_code, operand)
            | Label(identifier)
            | Allocate_stack(int)
            | DeallocateStack(int)
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
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11

*/

namespace CodeGen {

struct InstVisitor;

struct Identifier {
    std::string value;
    explicit Identifier(std::string value)
        : value(std::move(value)) {}
};

struct Operand {
    enum class Kind {
        Imm, Register, Pseudo, Stack, Data
    };
    Kind kind;

    virtual ~Operand() = default;

    Operand() = delete;
protected:
    explicit Operand(const Kind k)
        : kind(k) {}
};

struct ImmOperand final : Operand {
    i32 value;
    explicit ImmOperand(const i32 value)
        : Operand(Kind::Imm), value(value) {}

    ImmOperand() = delete;
};

struct RegisterOperand final : Operand {
    enum class Type : u8 {
        AX, CX, DX, DI, SI, R8, R9, R10, R11
    };
    Type type;
    u8 size;
    explicit RegisterOperand(const Type k, const u8 size)
        : Operand(Operand::Kind::Register), type(k), size(size) {}

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

struct DataOperand final : Operand {
    std::string identifier;
    explicit DataOperand(std::string iden)
        : Operand(Kind::Data), identifier(std::move(iden)) {}

    DataOperand() = delete;
};

struct Inst {
    enum class Kind {
        Move, Unary, Binary, Cmp, Idiv, Cdq, Jmp, JmpCC, SetCC, Label,
        AllocateStack, DeallocateStack, Push, Call, Ret
    };
    enum class CondCode {
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

struct MoveInst final : Inst {
    std::shared_ptr<Operand> src;
    std::shared_ptr<Operand> dst;

    MoveInst(const std::shared_ptr<Operand>& src, const std::shared_ptr<Operand>& dst)
        : Inst(Kind::Move), src(src), dst(dst) {}

    void accept(InstVisitor& visitor) override;

    MoveInst() = delete;
};

struct UnaryInst final : Inst {
    enum class Operator {
        Neg, Not
    };
    Operator oper;
    std::shared_ptr<Operand> destination;

    UnaryInst(const Operator op, const std::shared_ptr<Operand>& dst)
        : Inst(Kind::Unary), oper(op), destination(dst) {}

    void accept(InstVisitor& visitor) override;

    UnaryInst() = delete;
};

struct BinaryInst final : Inst {
    enum class Operator {
        Add, Sub, Mul,
        BitwiseAnd, BitwiseOr, BitwiseXor,
        LeftShift, RightShift,
    };
    Operator oper;
    std::shared_ptr<Operand> lhs;
    std::shared_ptr<Operand> rhs;
    BinaryInst(const Operator op, const std::shared_ptr<Operand>& lhs, const std::shared_ptr<Operand>& rhs)
        :Inst(Kind::Binary), oper(op), lhs(lhs), rhs(rhs) {}

    void accept(InstVisitor& visitor) override;

    BinaryInst() = delete;
};

struct CmpInst final : Inst {
    std::shared_ptr<Operand> lhs;
    std::shared_ptr<Operand> rhs;
    CmpInst(const std::shared_ptr<Operand>& lhs, const std::shared_ptr<Operand>& rhs)
        : Inst(Kind::Cmp), lhs(lhs), rhs(rhs) {}

    void accept(InstVisitor& visitor) override;

    CmpInst() = delete;
};

struct IdivInst final : Inst {
    std::shared_ptr<Operand> operand;
    explicit IdivInst(const std::shared_ptr<Operand>& operand)
        : Inst(Kind::Idiv), operand(operand) {}

    void accept(InstVisitor& visitor) override;

    IdivInst() = delete;
};

struct CdqInst final : Inst {
    explicit CdqInst()
        : Inst(Kind::Cdq) {}

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
    explicit SetCCInst(const CondCode condition, const std::shared_ptr<Operand>& operand)
        : Inst(Kind::SetCC), condition(condition), operand(operand) {}

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

struct AllocStackInst final : Inst {
    i32 alloc;
    explicit AllocStackInst(i32 alloc)
        : Inst(Kind::AllocateStack), alloc(alloc) {}

    void accept(InstVisitor& visitor) override;

    AllocStackInst() = delete;
};

struct DeallocStackInst final : Inst {
    i32 dealloc;
    explicit DeallocStackInst(i32 dealloc)
        : Inst(Kind::DeallocateStack), dealloc(dealloc) {}

    void accept(InstVisitor& visitor) override;

    DeallocStackInst() = delete;
};

struct PushInst final : Inst {
    std::shared_ptr<Operand> operand;
    explicit PushInst(const std::shared_ptr<Operand>& operand)
        : Inst(Kind::Push), operand(operand) {}

    void accept(InstVisitor& visitor) override;

    PushInst() = delete;
};

struct CallInst final : Inst {
    Identifier funName;
    explicit CallInst(Identifier  iden)
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
    Function(std::string name, bool isGlobal)
        : TopLevel(Type::Function), name(std::move(name)), isGlobal(isGlobal) {}

    Function() = delete;
};

struct StaticVariable : TopLevel {
    std::string name;
    i32 init;
    const bool global;
    StaticVariable(std::string name, bool isGlobal, i32 init)
        : TopLevel(Type::StaticVariable), name(std::move(name)), init(init), global(isGlobal) {}

    StaticVariable() = delete;
};

struct Program {
    std::vector<std::unique_ptr<TopLevel>> topLevels;
};

struct InstVisitor {
    virtual ~InstVisitor() = default;

    virtual void visit(MoveInst&) = 0;
    virtual void visit(UnaryInst&) = 0;
    virtual void visit(BinaryInst&) = 0;
    virtual void visit(CmpInst&) = 0;
    virtual void visit(IdivInst&) = 0;
    virtual void visit(CdqInst&) = 0;
    virtual void visit(JmpInst&) = 0;
    virtual void visit(JmpCCInst&) = 0;
    virtual void visit(SetCCInst&) = 0;
    virtual void visit(LabelInst&) = 0;
    virtual void visit(AllocStackInst&) = 0;
    virtual void visit(DeallocStackInst&) = 0;
    virtual void visit(PushInst&) = 0;
    virtual void visit(CallInst&) = 0;
    virtual void visit(ReturnInst&) = 0;
};

inline void MoveInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void UnaryInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void BinaryInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void CmpInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void IdivInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void CdqInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void JmpInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void JmpCCInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void SetCCInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void LabelInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void AllocStackInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void DeallocStackInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void PushInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void CallInst::accept(InstVisitor& visitor) { visitor.visit(*this); }
inline void ReturnInst::accept(InstVisitor& visitor) { visitor.visit(*this); }

}

#endif // CC_CODEGEN_ABSTRACTTREE_HPP
