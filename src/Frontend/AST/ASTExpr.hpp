#pragma once

#include "ASTVisitor.hpp"
#include "ASTBase.hpp"
#include "ShortTypes.hpp"

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Parsing {

struct ConstExpr final : Expr {
    std::variant<i32, i64, u32, u64, double> value;

    template<typename T>
    ConstExpr(T&& value, std::unique_ptr<TypeBase> varType) noexcept
        : Expr(Kind::Constant, std::move(varType)), value(std::forward<T>(value)) {}

    template<typename T>
    ConstExpr(const i64 location, T&& value, std::unique_ptr<TypeBase> varType) noexcept
        : Expr(location, Kind::Constant, std::move(varType)), value(std::forward<T>(value)) {}

    template<typename TargetType>
    TargetType getValue() const
    {
        switch (type->type) {
            case Type::I32:
                return std::get<i32>(value);
            case Type::I64:
                return std::get<i64>(value);
            case Type::U32:
                return std::get<u32>(value);
            case Type::U64:
                return std::get<u64>(value);
            case Type::Double:
                return std::get<double>(value);
            default:
                std::abort();
        }
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Constant; }

    ConstExpr() = delete;
};

struct VarExpr final : Expr {
    std::string name;
    ReferingTo referingTo = ReferingTo::Local;

    explicit VarExpr(std::string name) noexcept
        : Expr(Kind::Var), name(std::move(name)) {}

    VarExpr(const i64 loc, std::string name) noexcept
        : Expr(loc, Kind::Var), name(std::move(name)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Var; }

    VarExpr() = delete;
};

struct CastExpr final : Expr {
    std::unique_ptr<Expr> expr;

    CastExpr(std::unique_ptr<TypeBase>&& type, std::unique_ptr<Expr>&& expr) noexcept
        : Expr(Kind::Cast, std::move(type)), expr(std::move(expr)) {}

    CastExpr(const i64 loc, std::unique_ptr<TypeBase>&& type, std::unique_ptr<Expr>&& expr) noexcept
        : Expr(loc, Kind::Cast, std::move(type)), expr(std::move(expr)) {}

    explicit CastExpr(std::unique_ptr<Expr>&& expr) noexcept
        : Expr(Kind::Cast), expr(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Cast; }

    CastExpr() = delete;
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

    UnaryExpr(const i64 loc, const Operator op, std::unique_ptr<Expr> expr)
        : Expr(loc, Kind::Unary), op(op), operand(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Unary; }

    UnaryExpr() = delete;
};

struct BinaryExpr final : Expr {
    enum class Operator {
        Add, Subtract, Multiply, Divide, Modulo,
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

    BinaryExpr(const i64 loc, const Operator op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(loc, Kind::Binary), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Binary; }

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

    AssignmentExpr(const i64 loc, const Operator op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(loc, Kind::Assignment), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Assignment; }

    AssignmentExpr() = delete;
};

struct TernaryExpr final : Expr {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> trueExpr;
    std::unique_ptr<Expr> falseExpr;

    TernaryExpr(std::unique_ptr<Expr> condition,
                std::unique_ptr<Expr> first,
                std::unique_ptr<Expr> second)
        : Expr(Kind::Ternary), condition(std::move(condition)),
          trueExpr(std::move(first)), falseExpr(std::move(second)) {}

    TernaryExpr(const i64 location,
                std::unique_ptr<Expr> condition,
                std::unique_ptr<Expr> first,
                std::unique_ptr<Expr> second)
    : Expr(Kind::Ternary), condition(std::move(condition)),
      trueExpr(std::move(first)), falseExpr(std::move(second)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Ternary; }

    TernaryExpr() = delete;
};

struct FuncCallExpr final : Expr {
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;

    FuncCallExpr(std::string identifier, std::vector<std::unique_ptr<Expr>> args)
        : Expr(Kind::FunctionCall), name(std::move(identifier)), args(std::move(args)) {}

    FuncCallExpr(const i64 loc, std::string identifier, std::vector<std::unique_ptr<Expr>> args)
        : Expr(loc, Kind::FunctionCall), name(std::move(identifier)), args(std::move(args)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::FunctionCall; }

    FuncCallExpr() = delete;
};

struct DereferenceExpr final : Expr {
    std::unique_ptr<Expr> reference;

    explicit DereferenceExpr(std::unique_ptr<Expr>&& reference)
        : Expr(Kind::Dereference), reference(std::move(reference)) {}

    DereferenceExpr(const i64 loc, std::unique_ptr<Expr>&& reference)
        : Expr(loc, Kind::Dereference), reference(std::move(reference)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Dereference; }

    DereferenceExpr() = delete;
};

struct AddrOffExpr final : Expr {
    std::unique_ptr<Expr> reference;

    explicit AddrOffExpr(std::unique_ptr<Expr>&& reference)
        : Expr(Kind::AddrOf), reference(std::move(reference)) {}

    AddrOffExpr(const i64 loc, std::unique_ptr<Expr>&& reference)
        : Expr(loc, Kind::AddrOf), reference(std::move(reference)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::AddrOf; }

    AddrOffExpr() = delete;
};
} // Parsing