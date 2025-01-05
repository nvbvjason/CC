#pragma once

#ifndef ABSTRACTTREE_HPP
#define ABSTRACTTREE_HPP

#include "../ShortTypes.hpp"

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

struct ProgramNode;
struct FunctionNode;
struct StatementNode;
struct ExpressionNode;

struct ProgramNode {
    std::unique_ptr<FunctionNode> function = nullptr;
};

struct FunctionNode {
    std::string name;
    std::unique_ptr<StatementNode> body = nullptr;
};

struct StatementNode {
    std::unique_ptr<ExpressionNode> expression = nullptr;
};

enum class ExpressionNodeType {
    Constant, Unary
};

enum class UnaryOperator {
    Complement, Negate
};

struct UnaryNode {
    UnaryOperator unaryOperator;
    std::unique_ptr<ExpressionNode> expression = nullptr;
};

struct ExpressionNode {
    ExpressionNodeType type;
    std::variant<i32, std::unique_ptr<UnaryNode>> value;
};
} // Parsing

#endif //ABSTRACTTREE_HPP