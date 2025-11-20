#pragma once

#include "ASTVisitor.hpp"
#include "ASTBase.hpp"
#include "ShortTypes.hpp"
#include "ASTTypes.hpp"

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Parsing {

struct ConstExpr final : Expr {
    std::variant<char, i8, u8, i32, i64, u32, u64, double> value;

    template<typename T>
    ConstExpr(T&& value, std::unique_ptr<TypeBase> varType) noexcept
        : Expr(Kind::Constant, std::move(varType)), value(std::forward<T>(value)) {}

    template<typename T>
    ConstExpr(const i64 location, T&& value, std::unique_ptr<TypeBase> varType) noexcept
        : Expr(location, Kind::Constant, std::move(varType)), value(std::forward<T>(value)) {}

    ConstExpr(ConstExpr&& constExpr) noexcept
        : Expr(constExpr.location, Kind::Constant, std::move(constExpr.type)),
          value(constExpr.value) {}

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

    StringExpr(StringExpr&& stringExpr) noexcept
        : Expr(stringExpr.location, Kind::String),
          value(stringExpr.value)
    {
        if (stringExpr.type)
            type = std::move(stringExpr.type);
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::String; }

    StringExpr() = delete;
};

struct VarExpr final : Expr {
    std::string name;
    ReferingTo referingTo = ReferingTo::Local;

    VarExpr(const i64 loc, std::string name) noexcept
        : Expr(loc, Kind::Var), name(std::move(name)) {}

    VarExpr(VarExpr&& varExpr) noexcept
        : Expr(varExpr.location, Kind::Var),
          name(std::move(varExpr.name)),
          referingTo(varExpr.referingTo)
    {
        if (varExpr.type)
            type = std::move(varExpr.type);
    }

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

    CastExpr(CastExpr&& castExpr) noexcept
        : Expr(castExpr.location, Kind::Cast, std::move(castExpr.type)),
         innerExpr(std::move(castExpr.innerExpr)) {}

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

    UnaryExpr(const i64 loc, const Operator op, std::unique_ptr<Expr> expr)
        : Expr(loc, Kind::Unary), op(op), innerExpr(std::move(expr)) {}

    UnaryExpr(UnaryExpr&& unaryExpr) noexcept
        : Expr(unaryExpr.location, Kind::Unary),
          op(unaryExpr.op),
          innerExpr(std::move(unaryExpr.innerExpr))
    {
        if (unaryExpr.type)
            type = std::move(unaryExpr.type);
    }

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

    BinaryExpr(const i64 loc, const Operator op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(loc, Kind::Binary), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    BinaryExpr(BinaryExpr&& binaryExpr) noexcept
        : Expr(binaryExpr.location, Kind::Binary),
          op(binaryExpr.op),
          lhs(std::move(binaryExpr.lhs)),
          rhs(std::move(binaryExpr.rhs))
    {
        if (binaryExpr.type)
            type = std::move(binaryExpr.type);
    }

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

    AssignmentExpr(const i64 loc, const Operator op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(loc, Kind::Assignment), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    AssignmentExpr(AssignmentExpr&& assignmentExpr) noexcept
        : Expr(assignmentExpr.location, Kind::Assignment),
          op(assignmentExpr.op),
          lhs(std::move(assignmentExpr.lhs)),
          rhs(std::move(assignmentExpr.rhs))
    {
        if (assignmentExpr.type)
            type = std::move(assignmentExpr.type);
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Assignment; }

    AssignmentExpr() = delete;
};

struct TernaryExpr final : Expr {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> trueExpr;
    std::unique_ptr<Expr> falseExpr;

    TernaryExpr(const i64 location,
                std::unique_ptr<Expr> condition,
                std::unique_ptr<Expr> trueExpr,
                std::unique_ptr<Expr> falseExpr)
    : Expr(location, Kind::Ternary), condition(std::move(condition)),
      trueExpr(std::move(trueExpr)), falseExpr(std::move(falseExpr)) {}

    TernaryExpr(TernaryExpr&& ternaryExpr) noexcept
        : Expr(ternaryExpr.location, Kind::Ternary),
          condition(std::move(ternaryExpr.condition)),
          trueExpr(std::move(ternaryExpr.trueExpr)),
          falseExpr(std::move(ternaryExpr.falseExpr))
    {
        if (ternaryExpr.type)
            type = std::move(ternaryExpr.type);
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Ternary; }

    TernaryExpr() = delete;
};

struct FuncCallExpr final : Expr {
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;

    FuncCallExpr(const i64 loc, std::string identifier, std::vector<std::unique_ptr<Expr>> args)
        : Expr(loc, Kind::FunctionCall), name(std::move(identifier)), args(std::move(args)) {}

    FuncCallExpr(FuncCallExpr&& funcCallExpr) noexcept
        : Expr(funcCallExpr.location, Kind::FunctionCall),
          name(std::move(funcCallExpr.name)),
          args(std::move(funcCallExpr.args))
    {
        if (funcCallExpr.type)
            type = std::move(funcCallExpr.type);
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::FunctionCall; }

    FuncCallExpr() = delete;
};

struct DereferenceExpr final : Expr {
    std::unique_ptr<Expr> reference;

    DereferenceExpr(const i64 loc, std::unique_ptr<Expr>&& reference)
        : Expr(loc, Kind::Dereference), reference(std::move(reference)) {}

    DereferenceExpr(DereferenceExpr&& dereferenceExpr) noexcept
        : Expr(dereferenceExpr.location, Kind::Dereference),
          reference(std::move(dereferenceExpr.reference))
    {
        if (dereferenceExpr.type)
            type = std::move(dereferenceExpr.type);
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Dereference; }

    DereferenceExpr() = delete;
};

struct AddrOffExpr final : Expr {
    std::unique_ptr<Expr> reference;

    AddrOffExpr(const i64 loc, std::unique_ptr<Expr>&& reference)
        : Expr(loc, Kind::AddrOf), reference(std::move(reference)) {}

    AddrOffExpr(AddrOffExpr&& addrOffExpr) noexcept
        : Expr(addrOffExpr.location, Kind::AddrOf),
          reference(std::move(addrOffExpr.reference))
    {
        if (addrOffExpr.type)
            type = std::move(addrOffExpr.type);
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::AddrOf; }

    AddrOffExpr() = delete;
};

struct SubscriptExpr final : Expr {
    std::unique_ptr<Expr> referencing;
    std::unique_ptr<Expr> index;

    SubscriptExpr(const i64 loc, std::unique_ptr<Expr> re, std::unique_ptr<Expr> index)
        : Expr(loc, Kind::Subscript), referencing(std::move(re)), index(std::move(index)) {}

    SubscriptExpr(SubscriptExpr&& subscriptExpr) noexcept
        : Expr(subscriptExpr.location, Kind::Subscript),
          referencing(std::move(subscriptExpr.referencing)),
          index(std::move(subscriptExpr.index))
    {
        if (subscriptExpr.type)
            type = std::move(subscriptExpr.type);
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Subscript; }

    SubscriptExpr() = delete;
};

struct SizeOfExprExpr final : Expr {
    std::unique_ptr<Expr> innerExpr;

    explicit SizeOfExprExpr(const i64 loc, std::unique_ptr<Expr>&& innerExpr)
        : Expr(loc, Kind::SizeOfExpr, std::make_unique<VarType>(Type::U64)),
          innerExpr(std::move(innerExpr)) {}

    SizeOfExprExpr(SizeOfExprExpr&& sizeOfExprExpr) noexcept
        : Expr(sizeOfExprExpr.location, Kind::SizeOfExpr, std::make_unique<VarType>(Type::U64)),
          innerExpr(std::move(sizeOfExprExpr.innerExpr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::SizeOfExpr; }

    SizeOfExprExpr() = delete;
};

struct SizeOfTypeExpr final : Expr {
    std::unique_ptr<TypeBase> sizeType;

    explicit SizeOfTypeExpr(const i64 loc, std::unique_ptr<TypeBase>&& sizeType)
        : Expr(loc, Kind::SizeOfType, std::make_unique<VarType>(Type::U64)),
          sizeType(std::move(sizeType)) {}

    SizeOfTypeExpr(SizeOfTypeExpr&& sizeOfTypeExpr) noexcept
        : Expr(sizeOfTypeExpr.location, Kind::SizeOfType, std::make_unique<VarType>(Type::U64)),
          sizeType(std::move(sizeOfTypeExpr.sizeType)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::SizeOfType; }

    SizeOfTypeExpr() = delete;
};

struct DotExpr final : Expr {
    std::unique_ptr<Expr> structuredExpr;
    const std::string identifier;

    explicit DotExpr(const i64 loc, std::unique_ptr<Expr>&& structuredExpr, std::string identifier)
        : Expr(loc, Kind::Dot), structuredExpr(std::move(structuredExpr)), identifier(std::move(identifier)) {}

    DotExpr(DotExpr&& dotExpr) noexcept
        : Expr(dotExpr.location, Kind::Arrow),
          structuredExpr(std::move(dotExpr.structuredExpr)),
          identifier(dotExpr.identifier)
    {
        if (dotExpr.type)
            type = std::move(dotExpr.type);
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Dot; }

    DotExpr() = delete;
};

struct ArrowExpr final : Expr {
    std::unique_ptr<Expr> pointerExpr;
    const std::string identifier;

    explicit ArrowExpr(const i64 loc, std::unique_ptr<Expr>&& pointerExpr, std::string identifier)
        : Expr(loc, Kind::Arrow), pointerExpr(std::move(pointerExpr)), identifier(std::move(identifier)) {}

    ArrowExpr(ArrowExpr&& arrowExpr) noexcept
        : Expr(arrowExpr.location, Kind::Arrow),
          pointerExpr(std::move(arrowExpr.pointerExpr)),
          identifier(arrowExpr.identifier)
    {
        if (arrowExpr.type)
            type = std::move(arrowExpr.type);
    }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Expr* expr) { return expr->kind == Kind::Arrow; }

    ArrowExpr() = delete;
};
} // Parsing