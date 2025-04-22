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

void Visualizer::expression(const Expression& expression, const i32 level)
{
    const std::string outerLevelIdent = makeIndent(level);
    const std::string innerLevelIdent = makeIndent(level);
    switch (expression.type) {
        case ExpressionType::Constant:
            result << innerLevelIdent << "Constant(" << std::to_string(std::get<i32>(expression.value)) << ")\n";
            break;
        case ExpressionType::Unary:
            unaryNode(*std::get<std::unique_ptr<Unary>>(expression.value), level + 1);
            break;
        default:
            break;
    }
}

void Visualizer::unaryNode(const Unary& unary, const i32 level)
{
    const std::string outerLevelIdent(level * indentLength, ident);
    result << outerLevelIdent << unaryOperatorVisualizer(unary.unaryOperator) << " (" << "\n";
    expression(*unary.expression, level + 1);
    result << outerLevelIdent << ")\n";
}

std::string unaryOperatorVisualizer(const UnaryOperator& unaryOperator)
{
    switch (unaryOperator) {
        case UnaryOperator::Complement:
            return "Complement()";
        case UnaryOperator::Negate:
            return "Negate()";
        default:
            return "Error";
    }
}

}
