#include "AstVisualizer.hpp"

namespace Parsing {

std::string Visualizer::visualize(const Program& programNode)
{
    result.clear();
    result << "Program(\n";
    function(*programNode.function, 1);
    result << ")\n";
    return result.str();
}

void Visualizer::function(const Function& function, const i32 level)
{
    const std::string outerLevelIdent = makeIndent(level);
    const std::string innerLevelIdent = makeIndent(level + 1);
    result << outerLevelIdent << "Function(\n"
           << innerLevelIdent << "name=\"" << function.name << "\",\n"
           << innerLevelIdent << "body=Return(\n";
    expression(*function.body->expression, level + 1);
    result << innerLevelIdent << ")\n"
           << outerLevelIdent << ")\n";
}

void Visualizer::expression(const Expr& expr, const i32 level)
{
    const std::string outerLevelIdent = makeIndent(level);
    const std::string innerLevelIdent = makeIndent(level);
    if (auto* constant = dynamic_cast<const ConstantExpr*>(&expr)) {
        result << innerLevelIdent << "Constant(" << std::to_string(constant->value) << ")\n";
        return;
    }
    if (auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        unaryNode(*unary, level + 1);
        return;
    }
    result << innerLevelIdent << "Unknow Expression(\n";
}

void Visualizer::unaryNode(const UnaryExpr& unary, const i32 level)
{
    const std::string outerLevelIdent(level * indentLength, ident);
    result << outerLevelIdent << unaryOperatorVisualizer(unary.op) << " (" << "\n";
    expression(*unary.operand, level + 1);
    result << outerLevelIdent << ")\n";
}

std::string unaryOperatorVisualizer(const UnaryExpr::Operator& unaryOperator)
{
    switch (unaryOperator) {
        case UnaryExpr::Operator::Complement:
            return "Complement()";
        case UnaryExpr::Operator::Negate:
            return "Negate()";
        default:
            return "Error";
    }
}

}
