#pragma once

#ifndef AST_VISUALIZER_HPP
#define AST_VISUALIZER_HPP

#include "AbstractTree.hpp"

namespace Parsing {
    std::string astVisualizer(const Parsing::ProgramNode& programNode);
    void functionVisualizer(i32 level, std::string& result, const FunctionNode& function);
    void expressionVisualizer(i32 level, std::string& result, const ExpressionNode& expression);
    void unaryNOdeVisualizer(i32 level, std::string& result, const UnaryNode& unary);

    std::string unaryOperatorVisualizer(const UnaryOperator& unaryOperator);
}

#endif // AST_VISUALIZER_HPP
