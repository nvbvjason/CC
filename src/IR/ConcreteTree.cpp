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
    instructionTacky(parsingExpressionNode, functionTacky->instructions);
    return functionTacky;
}

std::unique_ptr<Value> instructionTacky(const Parsing::Expr *parsingExpr,
                                        std::vector<Instruction> &instructions)
{
    if (const auto constant = dynamic_cast<const Parsing::ConstantExpr*>(parsingExpr)) {
        auto returnValue = std::make_unique<Value>(constant->value);
        returnValue->type = ValueType::Constant;
        return returnValue;
    }
    if (auto* unaryParsing = dynamic_cast<const Parsing::UnaryExpr*>(parsingExpr)) {
        const Parsing::Expr *inner = unaryParsing->operand.get();
        const auto tackyUnary = new Unary();
        tackyUnary->source = instructionTacky(inner, instructions);
        tackyUnary->operation = convertUnaryOperation(unaryParsing->op);
        tackyUnary->destination = std::make_unique<Value>(makeTemporaryName());
        tackyUnary->destination->type = ValueType::Variable;
        instructions.emplace_back(InstructionType::Unary, tackyUnary);
        return std::move(tackyUnary->destination);
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