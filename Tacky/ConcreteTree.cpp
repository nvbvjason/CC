#include "ConcreteTree.hpp"

namespace Tacky {
static std::string makeTemporaryName();
static UnaryOperationType convertUnaryOperation(Parsing::UnaryOperator unaryOperation);

void programTacky(const Parsing::Program *parsingProgram, Program& tackyProgram)
{
    tackyProgram.function = static_cast<std::unique_ptr<Function>>(functionTacky(parsingProgram->function.get()));
}

Function* functionTacky(const Parsing::Function* parsingFunction)
{
    const auto functionTacky = new Function();
    functionTacky->identifier = parsingFunction->name;
    const Parsing::Expression* parsingExpressionNode = parsingFunction->body->expression.get();
    instructionTacky(parsingExpressionNode, functionTacky->instructions);
    return functionTacky;
}

Value* instructionTacky(const Parsing::Expression *parsingExpression,
                            std::vector<Instruction> &instructions)
{
    switch (parsingExpression->type) {
        case Parsing::ExpressionType::Constant: {
            const auto returnValue = new Value(std::get<i32>(parsingExpression->value));
            return returnValue;
        }
        case Parsing::ExpressionType::Unary: {
            const auto unaryParsing = std::get<std::unique_ptr<Parsing::Unary>>(parsingExpression->value).get();
            const Parsing::Expression *inner = unaryParsing->expression.get();
            Value* source = instructionTacky(inner, instructions);
            const auto tackyUnary = new Unary();
            tackyUnary->operation = convertUnaryOperation(unaryParsing->unaryOperator);
            tackyUnary->source = static_cast<std::unique_ptr<Value>>(source);
            tackyUnary->destination = std::make_unique<Value>(makeTemporaryName());
            instructions.emplace_back(InstructionType::Unary, tackyUnary);
            return tackyUnary->destination.get();
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