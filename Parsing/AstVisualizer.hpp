#pragma once

#ifndef CC_PARSING_AST_VISUALIZER_HPP
#define CC_PARSING_AST_VISUALIZER_HPP

#include "AbstractTree.hpp"

namespace Parsing {
    std::string astVisualizer(const ProgramNode& programNode);
    void functionVisualizer(i32 level, std::string& result, const FunctionNode& function);
    void expressionVisualizer(i32 level, std::string& result, const ExpressionNode& expression);
    void unaryNOdeVisualizer(i32 level, std::string& result, const UnaryNode& unary);

    std::string unaryOperatorVisualizer(const UnaryOperator& unaryOperator);
}

#endif // CC_PARSING_AST_VISUALIZER_HPP
