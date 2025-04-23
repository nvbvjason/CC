#pragma once

#ifndef CC_PARSING_ABSTRACT_TREE_HPP
#define CC_PARSING_ABSTRACT_TREE_HPP

#include <memory>
#include <string>

#include "ShortTypes.hpp"

/*
    program = Program(function_definition)
    function_definition = Function(identifier names, statement body)
    statement = Return(exp)
    exp = Constant(int) | Unary(unary_operator, exp)
    unary_operator = Complement | Negate
*/

namespace Parsing {

struct Program;
struct Function;
struct Statement;
struct Expr;

struct Program {
    std::unique_ptr<Function> function = nullptr;
};

struct Function {
    std::string name;
    std::unique_ptr<Statement> body = nullptr;
};

struct Statement {
    std::unique_ptr<Expr> expression = nullptr;
};

struct Expr {
    enum class Kind {
        Constant, Unary,
        Invalid
    };
    Kind kind = Kind::Invalid;

    Expr() = default;
    virtual ~Expr() = default;
    template<typename T>
    [[nodiscard]] bool is() const { return typeid(*this) == typeid(T); }

    template<typename T>
    T* as() { return is<T>() ? static_cast<T*>(this) : nullptr; }
};

struct ConstantExpr final : Expr {
    i32 value;
    explicit ConstantExpr(const i32 value) noexcept
        : value(value)
    {
        kind = Kind::Constant;
    }
};

struct UnaryExpr final : Expr {
    enum class Operator {
        Complement, Negate,
        Invalid
    };
    Operator op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(const Operator op, std::unique_ptr<Expr> expr)
        : op(op), operand(std::move(expr))
    {
        kind = Kind::Unary;
    }
};
} // Parsing

#endif // CC_PARSING_ABSTRACT_TREE_HPP