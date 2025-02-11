#include "AstVisualizer.hpp"

namespace Parsing {
std::string astVisualizer(const Program& programNode)
{
    std::string result = "Program(\n";
    functionVisualizer(1, result, *programNode.function);
    result += ")\n";
    return result;
}

void functionVisualizer(const i32 level, std::string& result, const Function& function)
{
    const std::string outerLevelIdent(level, '\t');
    result += outerLevelIdent + "Function(\n";
    const std::string innerLevelIdent(level + 1, '\t');
    result += innerLevelIdent + "name=\"" + function.name + "\",\n";
    result += innerLevelIdent + "body=Return(\n";
    expressionVisualizer(level + 1, result, *function.body->expression);
    result += innerLevelIdent + ")\n";
    result += outerLevelIdent + ")\n";
}

void expressionVisualizer(const i32 level, std::string& result, const Expression& expression)
{
    const std::string outerLevelIdent(level, '\t');
    const std::string innerLevelIdent(level + 1, '\t');
    switch (expression.type) {
        case ExpressionType::Constant:
            result += innerLevelIdent + "Constant(" + std::to_string(std::get<i32>(expression.value)) + ")\n";
            break;
        case ExpressionType::Unary:
            unaryNOdeVisualizer(level + 1, result, *std::get<std::unique_ptr<Unary>>(expression.value));
            break;
        default:
            break;
    }
}

void unaryNOdeVisualizer(const i32 level, std::string& result, const Unary& unary)
{
    const std::string outerLevelIdent(level, '\t');
    result += outerLevelIdent + unaryOperatorVisualizer(unary.unaryOperator) + " (" + "\n";
    expressionVisualizer(level, result, *unary.expression);
    result += outerLevelIdent + ")\n";
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
