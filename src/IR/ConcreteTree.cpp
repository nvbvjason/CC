#include "ConcreteTree.hpp"

#include <stdexcept>

namespace Ir {

static std::string makeTemporaryName();
static UnaryInst::Operation convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);

void program(const Parsing::Program *parsingProgram, Program& tackyProgram)
{
    tackyProgram.function = function(parsingProgram->function.get());
}

std::shared_ptr<Function> function(const Parsing::Function* parsingFunction)
{
    auto functionTacky = std::make_shared<Function>();
    functionTacky->identifier = parsingFunction->name;
    const Parsing::Expr* parsingExpressionNode = parsingFunction->body->expression.get();
    auto lastValue = instruction(parsingExpressionNode, functionTacky->instructions);
    functionTacky->instructions.push_back(std::make_shared<ReturnInst>(lastValue));
    return functionTacky;
}

std::shared_ptr<ValueVar> unaryInstruction(const Parsing::Expr *parsingExpr,
                                           std::vector<std::shared_ptr<Instruction>>& instructions)
{
    const auto unaryParsing = dynamic_cast<const Parsing::UnaryExpr*>(parsingExpr);
    const Parsing::Expr *inner = unaryParsing->operand.get();
    auto source = instruction(inner, instructions);
    UnaryInst::Operation operation = convertUnaryOperation(unaryParsing->op);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    instructions.push_back(std::make_shared<UnaryInst>(operation, source, destination));
    return destination;
}

std::shared_ptr<ValueConst> returnInstruction(const Parsing::Expr *parsingExpr)
{
    const auto constant = dynamic_cast<const Parsing::ConstantExpr*>(parsingExpr);
    auto result = std::make_shared<ValueConst>(constant->value);
    return result;
}

std::shared_ptr<Value> instruction(const Parsing::Expr *parsingExpr,
                                   std::vector<std::shared_ptr<Instruction>> &instructions)
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

UnaryInst::Operation convertUnaryOperation(const Parsing::UnaryExpr::Operator unaryOperation)
{
    switch (unaryOperation) {
        case Parsing::UnaryExpr::Operator::Complement:
            return UnaryInst::Operation::Complement;
        case Parsing::UnaryExpr::Operator::Negate:
            return UnaryInst::Operation::Negate;
        default:
            throw std::invalid_argument("Invalid unary operation");
    }
}

} // IR