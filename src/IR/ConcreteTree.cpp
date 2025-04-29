#include "ConcreteTree.hpp"
#include "Parsing/AbstractTree.hpp"

#include <stdexcept>

namespace Ir {

static Identifier makeTemporaryName();
static UnaryInst::Operation convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);
static BinaryInst::Operation convertBinaryOperation(Parsing::BinaryExpr::Operator binaryOperation);

void program(const Parsing::Program *parsingProgram, Program& tackyProgram)
{
    tackyProgram.function = function(parsingProgram->function.get());
}

std::shared_ptr<Function> function(const Parsing::Function* parsingFunction)
{
    auto functionTacky = std::make_shared<Function>();
    functionTacky->identifier = parsingFunction->name;
    const Parsing::Expr* parsingExpressionNode = parsingFunction->body->expression.get();
    auto lastValue = inst(parsingExpressionNode, functionTacky->instructions);
    functionTacky->instructions.push_back(std::make_shared<ReturnInst>(lastValue));
    return functionTacky;
}

std::shared_ptr<ValueVar> unaryInst(const Parsing::Expr *parsingExpr,
                                           std::vector<std::shared_ptr<Instruction>>& instructions)
{
    const auto unaryParsing = dynamic_cast<const Parsing::UnaryExpr*>(parsingExpr);
    UnaryInst::Operation operation = convertUnaryOperation(unaryParsing->op);
    const Parsing::Expr *inner = unaryParsing->operand.get();
    auto source = inst(inner, instructions);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    instructions.push_back(std::make_shared<UnaryInst>(operation, source, destination));
    return destination;
}

std::shared_ptr<Value> binaryInst(const Parsing::Expr *parsingExpr,
                                         std::vector<std::shared_ptr<Instruction>>& instructions)
{
    const auto binaryParsing = dynamic_cast<const Parsing::BinaryExpr*>(parsingExpr);
    BinaryInst::Operation operation = convertBinaryOperation(binaryParsing->op);
    if (operation == BinaryInst::Operation::Add)
        return binaryAndInst(binaryParsing, instructions);
    if (operation == BinaryInst::Operation::Or)
        return binaryOrInst(binaryParsing, instructions);
    const Parsing::Expr *lhs = binaryParsing->lhs.get();
    auto source1 = inst(lhs, instructions);
    const Parsing::Expr *rhs = binaryParsing->rhs.get();
    auto source2 = inst(rhs, instructions);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    instructions.push_back(std::make_shared<BinaryInst>(operation, source1, source2, destination));
    return destination;
}

std::shared_ptr<Value> binaryAndInst(const Parsing::BinaryExpr* const binaryExpr,
                                     std::vector<std::shared_ptr<Instruction>>& instructions)
{
    auto result = std::make_shared<ValueConst>(-1);
    auto lhs = inst(binaryExpr->lhs.get(), instructions);
    Identifier falseLabelIden = makeTemporaryName();
    instructions.push_back(std::make_shared<JumpIfZeroInst>(lhs, falseLabelIden));
    auto rhs = inst(binaryExpr->rhs.get(), instructions);
    instructions.push_back(std::make_shared<JumpIfZeroInst>(rhs, falseLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    instructions.push_back(std::make_shared<CopyInst>(oneVal, result));
    Identifier endLabelIden = makeTemporaryName();
    instructions.push_back(std::make_shared<JumpInst>(endLabelIden));
    instructions.push_back(std::make_shared<LabelInst>(falseLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    instructions.push_back(std::make_shared<CopyInst>(zeroVal, result));
    instructions.push_back(std::make_shared<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<Value> binaryOrInst(const Parsing::BinaryExpr* binaryExpr,
                                    std::vector<std::shared_ptr<Instruction>>& instructions)
{
    auto result = std::make_shared<ValueConst>(-1);
    auto lhs = inst(binaryExpr->lhs.get(), instructions);
    Identifier trueLabelIden = makeTemporaryName();
    instructions.push_back(std::make_shared<JumpIfNotZeroInst>(lhs, trueLabelIden));
    auto rhs = inst(binaryExpr->rhs.get(), instructions);
    instructions.push_back(std::make_shared<JumpIfNotZeroInst>(rhs, trueLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    instructions.push_back(std::make_shared<CopyInst>(zeroVal, result));
    Identifier endLabelIden = makeTemporaryName();
    instructions.push_back(std::make_shared<JumpInst>(endLabelIden));
    instructions.push_back(std::make_shared<LabelInst>(trueLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    instructions.push_back(std::make_shared<CopyInst>(oneVal, result));
    instructions.push_back(std::make_shared<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<ValueConst> returnInst(const Parsing::Expr *parsingExpr)
{
    const auto constant = dynamic_cast<const Parsing::ConstantExpr*>(parsingExpr);
    auto result = std::make_shared<ValueConst>(constant->value);
    return result;
}

std::shared_ptr<Value> inst(const Parsing::Expr *parsingExpr,
                                   std::vector<std::shared_ptr<Instruction>> &instructions)
{
    switch (parsingExpr->kind) {
        case Parsing::Expr::Kind::Constant:
            return returnInst(parsingExpr);
        case Parsing::Expr::Kind::Unary:
            return unaryInst(parsingExpr, instructions);
        case Parsing::Expr::Kind::Binary:
            return binaryInst(parsingExpr, instructions);
        default:
            throw std::invalid_argument("Unexpected expression type");
    }
}

Identifier makeTemporaryName()
{
    static i32 id = 0;
    std::string result = "tmp.";
    result += std::to_string(id++);
    return {result};
}

UnaryInst::Operation convertUnaryOperation(const Parsing::UnaryExpr::Operator unaryOperation)
{
    switch (unaryOperation) {
        case Parsing::UnaryExpr::Operator::Complement:
            return UnaryInst::Operation::Complement;
        case Parsing::UnaryExpr::Operator::Negate:
            return UnaryInst::Operation::Negate;
        case Parsing::UnaryExpr::Operator::Not:
            return UnaryInst::Operation::Not;
        default:
            throw std::invalid_argument("Invalid unary operation");
    }
}

BinaryInst::Operation convertBinaryOperation(const Parsing::BinaryExpr::Operator binaryOperation)
{
    using Parse = Parsing::BinaryExpr::Operator;
    using Ir = BinaryInst::Operation;

    switch (binaryOperation) {
        case Parse::Add:            return Ir::Add;
        case Parse::Subtract:       return Ir::Subtract;
        case Parse::Multiply:       return Ir::Multiply;
        case Parse::Divide:         return Ir::Divide;
        case Parse::Remainder:      return Ir::Remainder;

        case Parse::BitwiseAnd:     return Ir::BitwiseAnd;
        case Parse::BitwiseOr:      return Ir::BitwiseOr;
        case Parse::BitwiseXor:     return Ir::BitwiseXor;

        case Parse::LeftShift:      return Ir::LeftShift;
        case Parse::RightShift:     return Ir::RightShift;

        case Parse::And:            return Ir::And;
        case Parse::Or:             return Ir::Or;
        case Parse::Equal:          return Ir::Equal;
        case Parse::NotEqual:       return Ir::NotEqual;
        case Parse::Greater:        return Ir::GreaterThan;
        case Parse::GreaterOrEqual: return Ir::GreaterOrEqual;
        case Parse::LessThan:       return Ir::LessThan;
        case Parse::LessOrEqual:    return Ir::LessOrEqual;

        default:
            throw std::invalid_argument("Invalid binary operation");
    }
}

} // IR