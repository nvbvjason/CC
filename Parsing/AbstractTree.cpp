#include "AbstractTree.hpp"

namespace Parsing {

// ProgramNode programNode(FunctionNode* function)
// {
//     ProgramNode program;
//     program.function = static_cast<std::unique_ptr>(function);
//     return program;
// }
//
// FunctionNode functionNode(const std::string& name, StatementNode* body)
// {
//     FunctionNode function;
//     function.name = name;
//     function.body = static_cast<std::unique_ptr>(body);
//     return function;
// }
//
// StatementNode statementNode(ExpressionNode* expression)
// {
//     StatementNode statement;
//     statement.expression = static_cast<std::unique_ptr>(expression);
//     return statement;
// }
//
// ExpressionNode constantNode(const i32 value)
// {
//     ExpressionNode constant;
//     constant.type = ExpressionNodeType::Constant;
//     constant.data.constant = value;
//     return constant;
// }
//
// ExpressionNode unaryNode(const UnaryOperator unaryOperator, ExpressionNode* expression)
// {
//     ExpressionNode unary;
//     unary.type = ExpressionNodeType::Unary;
//     unary.data.unary.unaryOperator = unaryOperator;
//     unary.data.unary.expression = static_cast<std::unique_ptr>(expression);
//     return unary;
// }

} // Parsing