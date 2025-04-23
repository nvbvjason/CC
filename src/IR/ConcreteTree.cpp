#include "ConcreteTree.hpp"

#include <stdexcept>

namespace Ir {

static std::string makeTemporaryName();
static Unary::Operation convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);

void program(const Parsing::Program *parsingProgram, Program& tackyProgram)
{
    tackyProgram.function = function(parsingProgram->function.get());
}

std::unique_ptr<Function> function(const Parsing::Function* parsingFunction)
{
    auto functionTacky = std::make_unique<Function>();
    functionTacky->identifier = parsingFunction->name;
    const Parsing::Expr* parsingExpressionNode = parsingFunction->body->expression.get();
    functionTacky->instructions.emplace_back(instruction(parsingExpressionNode, functionTacky->instructions));
    return functionTacky;
}

Value unaryInstruction(const Parsing::Expr *parsingExpr, std::vector<Instruction> &instructions)
{
    const auto unaryParsing = dynamic_cast<const Parsing::UnaryExpr*>(parsingExpr);
    const Parsing::Expr *inner = unaryParsing->operand.get();
    Value source = instruction(inner, instructions);
    Unary::Operation operation = convertUnaryOperation(unaryParsing->op);
    Value destination(makeTemporaryName());
    destination.type = Value::Type::Variable;
    instructions.emplace_back(operation, source, destination);
    return destination;
}

Value returnInstruction(const Parsing::Expr *parsingExpr)
{
    const auto constant = dynamic_cast<const Parsing::ConstantExpr*>(parsingExpr);
    Value returnValue(constant->value);
    returnValue.type = Value::Type::Constant;
    return returnValue;
}

Value instruction(const Parsing::Expr *parsingExpr,
                  std::vector<Instruction> &instructions)
{
    switch (parsingExpr->kind) {
        case Parsing::Expr::Kind::Constant: {
            return returnInstruction(parsingExpr);
        }
        case Parsing::Expr::Kind::Unary: {
            return unaryInstruction(parsingExpr, instructions);
        }
        default:
            throw std::invalid_argument("Unexpected expression type");
    }
}

std::string makeTemporaryName()
{
    static i32 id = 0;
    std::string result = "tmp.";
    result += std::to_string(id++);
    return result;
}

Unary::Operation convertUnaryOperation(const Parsing::UnaryExpr::Operator unaryOperation)
{
    switch (unaryOperation) {
        case Parsing::UnaryExpr::Operator::Complement:
            return Unary::Operation::Complement;
        case Parsing::UnaryExpr::Operator::Negate:
            return Unary::Operation::Negate;
        default:
            throw std::invalid_argument("Invalid unary operation");
    }
}

} // IR