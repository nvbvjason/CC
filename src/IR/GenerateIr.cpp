#include "GenerateIr.hpp"
#include "Parsing/ParserAST.hpp"

#include <stdexcept>

namespace Ir {

static Identifier makeTemporaryName();
static UnaryInst::Operation convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);
static BinaryInst::Operation convertBinaryOperation(Parsing::BinaryExpr::Operator binaryOperation);

void program(const Parsing::Program* parsingProgram, Program& tackyProgram)
{
    tackyProgram.function = function(*parsingProgram->function);
}

std::unique_ptr<Function> function(const Parsing::Function& parsingFunction)
{
    auto functionTacky = std::make_unique<Function>(parsingFunction.name);
    for (const std::unique_ptr<Parsing::BlockItem>& item : parsingFunction.body)
        blockItem(*item, functionTacky->insts);
    // grrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
    auto zeroVal = std::make_shared<ValueConst>(0);
    functionTacky->insts.push_back(std::make_unique<ReturnInst>(zeroVal));
    return std::move(functionTacky);
}

std::shared_ptr<Value> blockItem(const Parsing::BlockItem& blockItem,
                                 std::vector<std::unique_ptr<Instruction>>& instructions)
{
    using Kind = Parsing::BlockItem::Kind;
    switch (blockItem.kind) {
        case Kind::Declaration: {
            const auto decl = dynamic_cast<const Parsing::DeclarationBlockItem*>(&blockItem);
            return declaration(*decl->decl, instructions);
        }
        case Kind::Statement: {
            const auto stmt = dynamic_cast<const Parsing::StmtBlockItem*>(&blockItem);
            return statement(*stmt->stmt, instructions);
        }
    }
}

std::shared_ptr<Value> declaration(const Parsing::Declaration& decl,
                                   std::vector<std::unique_ptr<Instruction>>& instructions)
{
    if (decl.init == nullptr)
        return nullptr;
    return inst(*decl.init, instructions);
}

std::shared_ptr<Value> statement(const Parsing::Stmt& stmt,
                                 std::vector<std::unique_ptr<Instruction>>& instructions)
{
    using Kind = Parsing::Stmt::Kind;
    switch (stmt.kind) {
        case Kind::Null:
            return nullptr;
        case Kind::Expression: {
            const auto stmtExpr = dynamic_cast<const Parsing::ExprStmt*>(&stmt);
            return inst(*stmtExpr->expression, instructions);
        }
        case Kind::Return: {
            const auto returnExpr = dynamic_cast<const Parsing::ReturnStmt*>(&stmt);
            return inst(*returnExpr->expression, instructions);;
        }
        default:
            throw std::invalid_argument("Unexpected statement type Ir generate");
    }
}

std::shared_ptr<Value> inst(const Parsing::Expr& parsingExpr,
                            std::vector<std::unique_ptr<Instruction>>& instructions)
{
    switch (parsingExpr.kind) {
        case Parsing::Expr::Kind::Constant:
            return returnInst(parsingExpr);
        case Parsing::Expr::Kind::Unary:
            return unaryInst(parsingExpr, instructions);
        case Parsing::Expr::Kind::Binary:
            return binaryInst(parsingExpr, instructions);
        case Parsing::Expr::Kind::Assignment:
            return assignInst(parsingExpr, instructions);
        default:
            throw std::invalid_argument("Unexpected expression type");
    }
}

std::shared_ptr<ValueVar> unaryInst(const Parsing::Expr& parsingExpr,
                                    std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto unaryParsingPtr = static_cast<const Parsing::UnaryExpr*>(&parsingExpr);
    UnaryInst::Operation operation = convertUnaryOperation(unaryParsingPtr->op);
    const Parsing::Expr inner = *unaryParsingPtr->operand;
    auto source = inst(inner, instructions);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    instructions.push_back(std::make_unique<UnaryInst>(operation, source, destination));
    return destination;
}

std::shared_ptr<Value> binaryInst(const Parsing::Expr& parsingExpr,
                                  std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto binaryParsing = dynamic_cast<const Parsing::BinaryExpr*>(&parsingExpr);
    BinaryInst::Operation operation = convertBinaryOperation(binaryParsing->op);
    if (operation == BinaryInst::Operation::And)
        return binaryAndInst(*binaryParsing, instructions);
    if (operation == BinaryInst::Operation::Or)
        return binaryOrInst(*binaryParsing, instructions);
    const Parsing::Expr lhs = *binaryParsing->lhs;
    const Parsing::Expr rhs = *binaryParsing->rhs;
    auto source1 = inst(lhs, instructions);
    auto source2 = inst(rhs, instructions);

    auto destination = std::make_shared<ValueVar>(makeTemporaryName());

    instructions.push_back(std::make_unique<BinaryInst>(operation, source1, source2, destination));
    return destination;
}

std::shared_ptr<Value> assignInst(const Parsing::Expr& binaryExpr,
                                        std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto assignExpr = dynamic_cast<const Parsing::AssignmentExpr*>(&binaryExpr);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    const Parsing::Expr rhs = *assignExpr->rhs;
    auto result = inst(rhs, instructions);
    instructions.push_back(std::make_unique<CopyInst>(result, destination));
    return destination;
}

std::shared_ptr<Value> binaryAndInst(const Parsing::BinaryExpr& parsingExpr,
                                     std::vector<std::unique_ptr<Instruction>>& instructions)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName());
    auto lhs = inst(*parsingExpr.lhs, instructions);
    Identifier falseLabelIden = makeTemporaryName();
    instructions.push_back(std::make_unique<JumpIfZeroInst>(lhs, falseLabelIden));
    auto rhs = inst(*parsingExpr.rhs, instructions);
    instructions.push_back(std::make_unique<JumpIfZeroInst>(rhs, falseLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    instructions.push_back(std::make_unique<CopyInst>(oneVal, result));
    Identifier endLabelIden = makeTemporaryName();
    instructions.push_back(std::make_unique<JumpInst>(endLabelIden));
    instructions.push_back(std::make_unique<LabelInst>(falseLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    instructions.push_back(std::make_unique<CopyInst>(zeroVal, result));
    instructions.push_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<Value> binaryOrInst(const Parsing::BinaryExpr& binaryExpr,
                                    std::vector<std::unique_ptr<Instruction>>& instructions)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName());
    auto lhs = inst(*binaryExpr.lhs, instructions);
    Identifier trueLabelIden = makeTemporaryName();
    instructions.push_back(std::make_unique<JumpIfNotZeroInst>(lhs, trueLabelIden));
    auto rhs = inst(*binaryExpr.rhs, instructions);
    instructions.push_back(std::make_unique<JumpIfNotZeroInst>(rhs, trueLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    instructions.push_back(std::make_unique<CopyInst>(zeroVal, result));
    Identifier endLabelIden = makeTemporaryName();
    instructions.push_back(std::make_unique<JumpInst>(endLabelIden));
    instructions.push_back(std::make_unique<LabelInst>(trueLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    instructions.push_back(std::make_unique<CopyInst>(oneVal, result));
    instructions.push_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<ValueConst> returnInst(const Parsing::Expr& parsingExpr)
{
    const auto constant = dynamic_cast<const Parsing::ConstExpr*>(&parsingExpr);
    auto result = std::make_unique<ValueConst>(constant->value);
    return result;
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
        case Parse::GreaterThan:    return Ir::GreaterThan;
        case Parse::GreaterOrEqual: return Ir::GreaterOrEqual;
        case Parse::LessThan:       return Ir::LessThan;
        case Parse::LessOrEqual:    return Ir::LessOrEqual;

        default:
            throw std::invalid_argument("Invalid binary operation");
    }
}

} // IR