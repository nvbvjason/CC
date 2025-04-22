#include "ConcreteTree.hpp"

namespace Tacky {
static std::string makeTemporaryName();
static UnaryOperationType convertUnaryOperation(Parsing::UnaryOperator unaryOperation);

void programTacky(const Parsing::Program *parsingProgram, Program& tackyProgram)
{
    tackyProgram.function = functionTacky(parsingProgram->function.get());
}

std::unique_ptr<Function> functionTacky(const Parsing::Function* parsingFunction)
{
    auto functionTacky = std::make_unique<Function>();
    functionTacky->identifier = parsingFunction->name;
    const Parsing::Expression* parsingExpressionNode = parsingFunction->body->expression.get();
    instructionTacky(parsingExpressionNode, functionTacky->instructions);
    return functionTacky;
}

std::unique_ptr<Value> instructionTacky(const Parsing::Expression *parsingExpression,
                                        std::vector<Instruction> &instructions)
{
    switch (parsingExpression->type) {
        case Parsing::ExpressionType::Constant: {
            auto returnValue = std::make_unique<Value>(std::get<i32>(parsingExpression->value));
            returnValue->type = ValueType::Constant;
            return returnValue;
        }
        case Parsing::ExpressionType::Unary: {
            const auto unaryParsing = std::get<std::unique_ptr<Parsing::Unary>>(parsingExpression->value).get();
            const Parsing::Expression *inner = unaryParsing->expression.get();
            const auto tackyUnary = new Unary();
            tackyUnary->source = instructionTacky(inner, instructions);
            tackyUnary->operation = convertUnaryOperation(unaryParsing->unaryOperator);
            tackyUnary->destination = std::make_unique<Value>(makeTemporaryName());
            tackyUnary->destination->type = ValueType::Variable;
            instructions.emplace_back(InstructionType::Unary, tackyUnary);
            return std::move(tackyUnary->destination);
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

UnaryOperationType convertUnaryOperation(const Parsing::UnaryOperator unaryOperation)
{
    switch (unaryOperation) {
        case Parsing::UnaryOperator::Complement:
            return UnaryOperationType::Complement;
        case Parsing::UnaryOperator::Negate:
            return UnaryOperationType::Negate;
        default:
            throw std::invalid_argument("Invalid unary operation");
    }
}

} // Tacky