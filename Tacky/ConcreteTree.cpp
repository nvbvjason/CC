#include "ConcreteTree.hpp"

namespace Tacky {
static std::string makeTemporaryName()
{
    static i32 id = 0;
    std::string result = "tmp.";
    result += std::to_string(id++);
    return result;
}

static UnaryOperationType convertUnaryOperation(const Parsing::UnaryOperator unaryOperation)
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

ProgramNode programTacky(const Parsing::ProgramNode *programNode)
{
    ProgramNode programTacky;
    programTacky.function = static_cast<std::unique_ptr<FunctionNode>>(functionTacky(programNode->function.get()));
    return programTacky;
}

FunctionNode* functionTacky(const Parsing::FunctionNode* functionNode)
{
    const auto functionTacky = new FunctionNode();
    functionTacky->identifier = functionNode->name;

    return functionTacky;
}

InstructionNode instructionTacky(const Parsing::ExpressionNode *expressionNode, std::vector<InstructionNode> &instructions)
{
    switch (expressionNode->type) {
        case Parsing::ExpressionNodeType::Constant: {
            InstructionNode constantInstruction;
            constantInstruction.type = InstructionType::Return;
            const auto returnValue = new ValueNode(std::get<i32>(expressionNode->value));
            constantInstruction.value = static_cast<std::unique_ptr<ValueNode>>(returnValue);
            return constantInstruction;
        }
        case Parsing::ExpressionNodeType::Unary: {
            const Parsing::UnaryNode* unaryParsingNode = std::get<std::unique_ptr<Parsing::UnaryNode>>(expressionNode->value).get();
            Parsing::ExpressionNode *inner = unaryParsingNode->expression.get();
            instructions.push_back(instructionTacky(inner, instructions));
            auto unaryNode = new UnaryNode();
            unaryNode->type = convertUnaryOperation(unaryParsingNode->unaryOperator);
            unaryNode->source = std::make_unique<ValueNode>(makeTemporaryName());
            unaryNode->destination = std::make_unique<ValueNode>(makeTemporaryName());
            InstructionNode instructionNode;
            instructionNode.type = InstructionType::Unary;
            instructionNode.value = static_cast<std::unique_ptr<UnaryNode>>(unaryNode);
            //instructions.emplace_back(instructionNode);
            return instructionNode;
        }
        default:
            throw std::invalid_argument("Unexpected expression type");
    }
}

} // Tacky