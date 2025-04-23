#include "ConcreteTree.hpp"

#include <stdexcept>

namespace IR {

static std::string makeTemporaryName();
static UnaryOperationType convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);

void programTacky(const Parsing::Program *parsingProgram, Program& tackyProgram)
{
    tackyProgram.function = functionTacky(parsingProgram->function.get());
}

std::unique_ptr<Function> functionTacky(const Parsing::Function* parsingFunction)
{
    auto functionTacky = std::make_unique<Function>();
    functionTacky->identifier = parsingFunction->name;
    const Parsing::Expr* parsingExpressionNode = parsingFunction->body->expression.get();
    functionTacky->instructions.emplace_back(instructionTacky(parsingExpressionNode, functionTacky->instructions));
    return functionTacky;
}

Value instructionTacky(const Parsing::Expr *parsingExpr,
                       std::vector<Instruction> &instructions)
{
    if (const auto constant = dynamic_cast<const Parsing::ConstantExpr*>(parsingExpr)) {
        Value returnValue(constant->value);
        returnValue.type = ValueType::Constant;
        return returnValue;
    }
    if (auto* unaryParsing = dynamic_cast<const Parsing::UnaryExpr*>(parsingExpr)) {
        const Parsing::Expr *inner = unaryParsing->operand.get();
        Value source = instructionTacky(inner, instructions);
        UnaryOperationType operation = convertUnaryOperation(unaryParsing->op);
        Value destination(makeTemporaryName());
        destination.type = ValueType::Variable;
        instructions.emplace_back(operation, source, destination);
        return destination;
    }
    throw std::invalid_argument("Unexpected expression type");
}

std::string makeTemporaryName()
{
    static i32 id = 0;
    std::string result = "tmp.";
    result += std::to_string(id++);
    return result;
}

UnaryOperationType convertUnaryOperation(const Parsing::UnaryExpr::Operator unaryOperation)
{
    switch (unaryOperation) {
        case Parsing::UnaryExpr::Operator::Complement:
            return UnaryOperationType::Complement;
        case Parsing::UnaryExpr::Operator::Negate:
            return UnaryOperationType::Negate;
        default:
            throw std::invalid_argument("Invalid unary operation");
    }
}

} // IR