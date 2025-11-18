#pragma once

#include "ASTVisitor.hpp"
#include "ASTBase.hpp"
#include "ShortTypes.hpp"

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "ASTTypes.hpp"

namespace Parsing {

struct ConstExpr final : Expr {
    std::variant<char, i8, u8, i32, i64, u32, u64, double> value;

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
            case Type::I8:      return std::get<i8>(value);
            case Type::U8:      return std::get<u8>(value);
            case Type::I32:     return std::get<i32>(value);
            case Type::U32:     return std::get<u32>(value);
            case Type::I64:     return std::get<i64>(value);
            case Type::U64:     return std::get<u64>(value);
            case Type::Double:  return std::get<double>(value);
            case Type::Char:    return std::get<char>(value);
            default:
                std::abort();
        }
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Constant; }

    ConstExpr() = delete;
};

struct StringExpr final : Expr {
    const std::string value;

    StringExpr(const i64 location, std::string&& value) noexcept
        : Expr(location, Kind::String), value(std::move(value)) {}
    StringExpr(const i64 location, std::string&& value, std::unique_ptr<TypeBase> varType) noexcept
        : Expr(location, Kind::String, std::move(varType)), value(std::move(value)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::String; }

    StringExpr() = delete;
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
    std::unique_ptr<Expr> innerExpr;

    CastExpr(std::unique_ptr<TypeBase>&& type, std::unique_ptr<Expr>&& expr) noexcept
        : Expr(Kind::Cast, std::move(type)), innerExpr(std::move(expr)) {}

    CastExpr(const i64 loc, std::unique_ptr<TypeBase>&& type, std::unique_ptr<Expr>&& expr) noexcept
        : Expr(loc, Kind::Cast, std::move(type)), innerExpr(std::move(expr)) {}

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
    std::unique_ptr<Expr> innerExpr;

    UnaryExpr(const Operator op, std::unique_ptr<Expr> expr)
        : Expr(Kind::Unary), op(op), innerExpr(std::move(expr)) {}

    UnaryExpr(const i64 loc, const Operator op, std::unique_ptr<Expr> expr)
        : Expr(loc, Kind::Unary), op(op), innerExpr(std::move(expr)) {}

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
                std::unique_ptr<Expr> trueExpr,
                std::unique_ptr<Expr> falseExpr)
        : Expr(Kind::Ternary), condition(std::move(condition)),
          trueExpr(std::move(trueExpr)), falseExpr(std::move(falseExpr)) {}

    TernaryExpr(const i64 location,
                std::unique_ptr<Expr> condition,
                std::unique_ptr<Expr> trueExpr,
                std::unique_ptr<Expr> falseExpr)
    : Expr(Kind::Ternary), condition(std::move(condition)),
      trueExpr(std::move(trueExpr)), falseExpr(std::move(falseExpr)) {}

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

struct SubscriptExpr final : Expr {
    std::unique_ptr<Expr> referencing;
    std::unique_ptr<Expr> index;

    SubscriptExpr(std::unique_ptr<Expr> re, std::unique_ptr<Expr> index)
        : Expr(Kind::Subscript), referencing(std::move(re)), index(std::move(index)) {}

    SubscriptExpr(const i64 loc, std::unique_ptr<Expr> re, std::unique_ptr<Expr> index)
        : Expr(loc, Kind::Subscript), referencing(std::move(re)), index(std::move(index)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Subscript; }

    SubscriptExpr() = delete;
};

struct SizeOfExprExpr final : Expr {
    std::unique_ptr<Expr> innerExpr;

    explicit SizeOfExprExpr(const i64 loc, std::unique_ptr<Expr>&& innerExpr)
        : Expr(loc, Kind::SizeOfExpr, std::make_unique<VarType>(Type::U64)), innerExpr(std::move(innerExpr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::SizeOfExpr; }

    SizeOfExprExpr() = delete;
};

struct SizeOfTypeExpr final : Expr {
    std::unique_ptr<TypeBase> sizeType;

    explicit SizeOfTypeExpr(const i64 loc, std::unique_ptr<TypeBase>&& sizeType)
        : Expr(loc, Kind::SizeOfType, std::make_unique<VarType>(Type::U64)), sizeType(std::move(sizeType)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::SizeOfType; }

    SizeOfTypeExpr() = delete;
};

struct DotExpr final : Expr {
    std::unique_ptr<Expr> structExpr;
    const std::string identifier;

    explicit DotExpr(const i64 loc, std::unique_ptr<Expr>&& sizeType, std::string  identifier)
        : Expr(loc, Kind::Dot), structExpr(std::move(sizeType)), identifier(std::move(identifier)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Dot; }

    DotExpr() = delete;
};

struct ArrowExpr final : Expr {
    std::unique_ptr<Expr> pointerExpr;
    const std::string identifier;

    explicit ArrowExpr(const i64 loc, std::unique_ptr<Expr>&& sizeType, std::string  identifier)
        : Expr(loc, Kind::Arrow), pointerExpr(std::move(sizeType)), identifier(std::move(identifier)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Arrow; }

    ArrowExpr() = delete;
};
} // Parsing