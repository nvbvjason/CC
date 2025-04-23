#pragma once

#ifndef CC_PARSING_ABSTRACT_TREE_HPP
#define CC_PARSING_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <string>

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
struct Expression;

struct Program {
    std::unique_ptr<Function> function = nullptr;
};

struct Function {
    std::string name;
    std::unique_ptr<Statement> body = nullptr;
};

struct Statement {
    std::unique_ptr<Expression> expression = nullptr;
};

enum class UnaryOperator {
    Complement, Negate,
    Invalid
};

struct Unary {
    UnaryOperator unaryOperator = UnaryOperator::Invalid;
    std::unique_ptr<Expression> expression = nullptr;
};

enum class ExpressionType {
    Constant, Unary,
    Invalid
};

struct Expression {
    ExpressionType type = ExpressionType::Invalid;
    std::variant<i32, std::unique_ptr<Expression>, std::unique_ptr<Unary>> value;
    Expression() = default;
    Expression(Expression&& other) noexcept
    : type(other.type), value(std::move(other.value)) {
        other.type = ExpressionType::Invalid;
    }
    Expression& operator=(Expression&& other) noexcept {
        if (this == &other)
            return *this;
        type = other.type;
        value = std::move(other.value);
        other.type = ExpressionType::Invalid;
        return *this;
    }
};
} // Parsing

#endif // CC_PARSING_ABSTRACT_TREE_HPP