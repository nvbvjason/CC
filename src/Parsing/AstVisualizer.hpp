#pragma once

#ifndef CC_PARSING_AST_VISUALIZER_HPP
#define CC_PARSING_AST_VISUALIZER_HPP

#include "AbstractTree.hpp"

#include <sstream>

namespace Parsing {
class Visualizer {
    static constexpr char ident = ' ';
    static constexpr i32 indentLength = 4;
    static std::string makeIndent(const i32 level) { return std::string(level * indentLength, ident); }
    std::ostringstream result;
public:
    std::string visualize(const Program& programNode);
    void function(const Function& function, i32 level);
    void expression(const Expr& expression, i32 level);
    void unaryNode(const UnaryExpr& unary, i32 level);
};

std::string unaryOperatorVisualizer(const UnaryExpr::Operator& unaryOperator);
}

#endif // CC_PARSING_AST_VISUALIZER_HPP
