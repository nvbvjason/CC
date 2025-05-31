#pragma once

#ifndef CC_PARSING_EXPR_TREE_HPP
#define CC_PARSING_EXPR_TREE_HPP

#include "ASTVisitor.hpp"
#include "ShortTypes.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Parsing {

struct Expr {
    enum class Kind {
        Constant, Var, Unary, Binary, Assignment, Conditional, FunctionCall,
    };
    Kind kind;

    virtual ~Expr() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    Expr() = delete;
protected:
    explicit Expr(const Kind kind)
        : kind(kind) {}
};

struct ConstExpr final : Expr {
    i32 value;

    explicit ConstExpr(const i32 value) noexcept
        : Expr(Kind::Constant), value(value) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    ConstExpr() = delete;
};

struct VarExpr final : Expr {
    std::string name;

    explicit VarExpr(std::string name) noexcept
        : Expr(Kind::Var), name(std::move(name)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    VarExpr() = delete;
};

struct UnaryExpr final : Expr {
    enum class Operator {
        Complement, Negate, Not, Plus,
        PrefixIncrement, PostFixIncrement,
        PrefixDecrement, PostFixDecrement
    };
    Operator op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(const Operator op, std::unique_ptr<Expr> expr)
        : Expr(Kind::Unary), op(op), operand(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    UnaryExpr() = delete;
};

struct BinaryExpr final : Expr {
    enum class Operator {
        Add, Subtract, Multiply, Divide, Remainder,
        BitwiseAnd, BitwiseOr, BitwiseXor,
        LeftShift, RightShift,
        And, Or,
        Equal, NotEqual,
        LessThan, LessOrEqual,
        GreaterThan, GreaterOrEqual
    };
    Operator op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    BinaryExpr(const Operator op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(Kind::Binary), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    BinaryExpr() = delete;
};

struct AssignmentExpr final : Expr {
    enum class Operator {
        Assign, PlusAssign, MinusAssign,
        MultiplyAssign, DivideAssign, ModuloAssign,
        BitwiseAndAssign, BitwiseOrAssign, BitwiseXorAssign,
        LeftShiftAssign, RightShiftAssign
    };
    Operator op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    AssignmentExpr(const Operator op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(Kind::Assignment), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    AssignmentExpr() = delete;
};

struct ConditionalExpr final : Expr {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> first;
    std::unique_ptr<Expr> second;
    ConditionalExpr(std::unique_ptr<Expr> condition,
                    std::unique_ptr<Expr> first,
                    std::unique_ptr<Expr> second)
        : Expr(Kind::Conditional), condition(std::move(condition)),
          first(std::move(first)), second(std::move(second)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    ConditionalExpr() = delete;
};

struct FunCallExpr final : Expr {
    std::string name;
    std::
    vector<std::unique_ptr<Expr>> args;

    FunCallExpr(std::string identifier, std::vector<std::unique_ptr<Expr>> args)
        : Expr(Kind::FunctionCall), name(std::move(identifier)), args(std::move(args)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    FunCallExpr() = delete;
};
} // Parsing

#endif // CC_PARSING_EXPR_TREE_HPP