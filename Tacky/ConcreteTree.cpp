#include "ConcreteTree.hpp"

namespace Tacky {
static std::string makeTemporaryName();
static UnaryOperationType convertUnaryOperation(Parsing::UnaryOperator unaryOperation);

void programTacky(const Parsing::ProgramNode *parsingProgram, ProgramNode& tackyProgram)
{
    tackyProgram.function = static_cast<std::unique_ptr<FunctionNode>>(functionTacky(parsingProgram->function.get()));
}

FunctionNode* functionTacky(const Parsing::FunctionNode* parsingFunction)
{
    const auto functionTacky = new FunctionNode();
    functionTacky->identifier = parsingFunction->name;
    const Parsing::ExpressionNode* parsingExpressionNode = parsingFunction->body->expression.get();
    instructionTacky(parsingExpressionNode, functionTacky->instructions);
    return functionTacky;
}

ValueNode* instructionTacky(const Parsing::ExpressionNode *parsingExpression,
                           std::vector<InstructionNode> &instructions)
{
    switch (parsingExpression->type) {
        case Parsing::ExpressionNodeType::Constant: {
            InstructionNode constantInstruction;
            constantInstruction.type = InstructionType::Return;
            const auto returnValue = new ValueNode(std::get<i32>(parsingExpression->value));
            constantInstruction.value = static_cast<std::unique_ptr<ValueNode>>(returnValue);
            return returnValue;
        }
        case Parsing::ExpressionNodeType::Unary: {
            const auto unaryParsing = std::get<std::unique_ptr<Parsing::UnaryNode>>(parsingExpression->value).get();
            const Parsing::ExpressionNode *inner = unaryParsing->expression.get();
            const auto tackyUnary = new UnaryNode();
            tackyUnary->type = convertUnaryOperation(unaryParsing->unaryOperator);
            tackyUnary->source = std::make_unique<ValueNode>(makeTemporaryName());
            tackyUnary->destination = std::make_unique<ValueNode>(makeTemporaryName());
            InstructionNode instruction;
            instruction.type = InstructionType::Unary;
            instruction.value = static_cast<std::unique_ptr<UnaryNode>>(tackyUnary);
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