#include "GenerateIr.hpp"

#include <stdexcept>

namespace Ir {

static Identifier makeTemporaryName();
static bool isPostfixOp(Parsing::UnaryExpr::Operator oper);
static bool isPrefixOp(Parsing::UnaryExpr::Operator oper);
static BinaryInst::Operation getPostPrefixOperation(Parsing::UnaryExpr::Operator oper);
static UnaryInst::Operation convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);
static BinaryInst::Operation convertBinaryOperation(Parsing::BinaryExpr::Operator binaryOperation);
static BinaryInst::Operation convertAssiOperation(Parsing::AssignmentExpr::Operator binaryOperation);

void program(const Parsing::Program* parsingProgram, Program& tackyProgram)
{
    tackyProgram.function = function(*parsingProgram->function);
}

std::unique_ptr<Function> function(const Parsing::Function& parsingFunction)
{
    auto functionTacky = std::make_unique<Function>(parsingFunction.name);
    for (const std::unique_ptr<Parsing::BlockItem>& item : parsingFunction.body->body)
        blockItem(*item, functionTacky->insts);
    return functionTacky;
}

void blockItem(const Parsing::BlockItem& blockItem,
               std::vector<std::unique_ptr<Instruction>>& instructions)
{
    using Kind = Parsing::BlockItem::Kind;
    switch (blockItem.kind) {
        case Kind::Declaration: {
            const auto decl = dynamic_cast<const Parsing::DeclBlockItem*>(&blockItem);
            declaration(*decl->decl, instructions);
            break;
        }
        case Kind::Statement: {
            const auto stmtBlockItem = dynamic_cast<const Parsing::StmtBlockItem*>(&blockItem);
            statement(*stmtBlockItem->stmt, instructions);
            break;
        }
        default:
            throw std::invalid_argument("Unexpected block item type ir generateBlockItem");
    }
}

void declaration(const Parsing::Declaration& decl,
                 std::vector<std::unique_ptr<Instruction>>& insts)
{
    if (decl.init == nullptr)
        return;
    auto value = inst(*decl.init, insts);
    auto temporary = std::make_shared<ValueVar>(makeTemporaryName());
    insts.push_back(std::make_unique<CopyInst>(value, temporary));
    const Identifier iden(decl.name);
    auto var = std::make_shared<ValueVar>(iden);
    insts.push_back(std::make_unique<CopyInst>(temporary, var));
}

void statement(const Parsing::Stmt& stmt,
               std::vector<std::unique_ptr<Instruction>>& insts)
{
    using Kind = Parsing::Stmt::Kind;
    switch (stmt.kind) {
        case Kind::Null:
            break;
        case Kind::Expression: {
            const auto stmtExpr = dynamic_cast<const Parsing::ExprStmt*>(&stmt);
            inst(*stmtExpr->expr, insts);
            break;
        }
        case Kind::If: {
            const auto parseIffStmt = dynamic_cast<const Parsing::IfStmt*>(&stmt);
            if (parseIffStmt->elseStmt == nullptr)
                ifStmt(*parseIffStmt, insts);
            else
                ifElseStmt(*parseIffStmt, insts);
            break;
        }
        case Kind::Return: {
            const auto returnStmt = dynamic_cast<const Parsing::ReturnStmt*>(&stmt);
            auto value = inst(*returnStmt->expr, insts);
            if (value == nullptr)
                break;
            insts.push_back(std::make_unique<ReturnInst>(value));
            break;
        }
        case Kind::Compound: {
            const auto compoundStmt = dynamic_cast<const Parsing::CompoundStmt*>(&stmt);
            for (const std::unique_ptr<Parsing::BlockItem>& item : compoundStmt->block->body)
                blockItem(*item, insts);
            break;
        }
        default:
            throw std::invalid_argument("Unexpected statement type Ir generate");
    }
}

void ifStmt(const Parsing::IfStmt& parseIfStmt,
            std::vector<std::unique_ptr<Instruction>>& insts)
{
    auto condition = inst(*parseIfStmt.condition, insts);
    Identifier endLabelIden = makeTemporaryName();
    insts.push_back(std::make_unique<JumpIfZeroInst>(condition, endLabelIden));
    statement(*parseIfStmt.thenStmt, insts);
    insts.push_back(std::make_unique<LabelInst>(endLabelIden));
}

void ifElseStmt(const Parsing::IfStmt& parseIfStmt,
                std::vector<std::unique_ptr<Instruction>>& insts)
{
    auto condition = inst(*parseIfStmt.condition, insts);
    Identifier elseStmtLabel = makeTemporaryName();
    insts.push_back(std::make_unique<JumpIfZeroInst>(condition, elseStmtLabel));
    statement(*parseIfStmt.thenStmt, insts);
    Identifier endLabelIden = makeTemporaryName();
    insts.push_back(std::make_unique<JumpInst>(endLabelIden));
    insts.push_back(std::make_unique<LabelInst>(elseStmtLabel));
    statement(*parseIfStmt.elseStmt, insts);
    insts.push_back(std::make_unique<LabelInst>(endLabelIden));
}

std::shared_ptr<Value> inst(const Parsing::Expr& parsingExpr,
                            std::vector<std::unique_ptr<Instruction>>& instructions)
{
    switch (parsingExpr.kind) {
        case Parsing::Expr::Kind::Var:
            return varInst(parsingExpr);
        case Parsing::Expr::Kind::Constant:
            return constInst(parsingExpr);
        case Parsing::Expr::Kind::Unary:
            return unaryInst(parsingExpr, instructions);
        case Parsing::Expr::Kind::Binary:
            return binaryInst(parsingExpr, instructions);
        case Parsing::Expr::Kind::Assignment:
            return assignInst(parsingExpr, instructions);
        case Parsing::Expr::Kind::Conditional:
            return conditionalExpr(parsingExpr, instructions);
        default:
            throw std::invalid_argument("Unexpected expression type ir generateInst");
    }
}

std::shared_ptr<Value> unaryPostfixInst(const Parsing::UnaryExpr& unaryExpr,
                                        std::vector<std::unique_ptr<Instruction>>& instructions)
{
    auto originalForReturn = std::make_shared<ValueVar>(makeTemporaryName());
    auto tempNew = std::make_shared<ValueVar>(makeTemporaryName());
    auto original = inst(*unaryExpr.operand, instructions);
    instructions.push_back(std::make_unique<CopyInst>(original, originalForReturn));
    auto operation = std::make_shared<ValueConst>(1);
    instructions.push_back(std::make_unique<BinaryInst>(
        getPostPrefixOperation(unaryExpr.op), originalForReturn, operation, tempNew
    ));
    instructions.push_back(std::make_unique<CopyInst>(tempNew, original));
    return originalForReturn;
}

std::shared_ptr<Value> unaryPrefixInst(const Parsing::UnaryExpr& unaryExpr,
                                        std::vector<std::unique_ptr<Instruction>>& instructions)
{
    auto original = inst(*unaryExpr.operand, instructions);
    auto temp = std::make_shared<ValueVar>(Identifier(makeTemporaryName()));
    instructions.push_back(std::make_unique<BinaryInst>(
        getPostPrefixOperation(unaryExpr.op), original, std::make_shared<ValueConst>(1), temp)
    );
    instructions.push_back(std::make_unique<CopyInst>(temp, original));
    return temp;
}

std::shared_ptr<Value> unaryInst(const Parsing::Expr& parsingExpr,
                                 std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto unaryParsingPtr = static_cast<const Parsing::UnaryExpr*>(&parsingExpr);
    if (isPostfixOp(unaryParsingPtr->op))
        return unaryPostfixInst(*unaryParsingPtr, instructions);
    if (isPrefixOp(unaryParsingPtr->op))
        return unaryPrefixInst(*unaryParsingPtr, instructions);
    UnaryInst::Operation operation = convertUnaryOperation(unaryParsingPtr->op);
    auto source = inst(*unaryParsingPtr->operand, instructions);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    instructions.push_back(std::make_unique<UnaryInst>(operation, source, destination));
    return destination;
}

std::shared_ptr<Value> varInst(const Parsing::Expr& parsingExpr)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(&parsingExpr);
    const Identifier iden(varExpr->name);
    auto var = std::make_shared<ValueVar>(iden);
    return var;
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
    auto source1 = inst(*binaryParsing->lhs, instructions);
    auto source2 = inst(*binaryParsing->rhs, instructions);

    auto destination = std::make_shared<ValueVar>(makeTemporaryName());

    instructions.push_back(std::make_unique<BinaryInst>(operation, source1, source2, destination));
    return destination;
}

std::shared_ptr<Value> assignInst(const Parsing::Expr& binaryExpr,
                                  std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto assignExpr = dynamic_cast<const Parsing::AssignmentExpr*>(&binaryExpr);
    if (assignExpr->op != Parsing::AssignmentExpr::Operator::Assign)
        return compoundAssignInst(*assignExpr, instructions);
    return simpleAssignInst(*assignExpr, instructions);
}

std::shared_ptr<Value> simpleAssignInst(const Parsing::AssignmentExpr& assignExpr,
                                        std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(assignExpr.lhs.get());
    const Identifier iden(varExpr->name);
    auto destination = std::make_shared<ValueVar>(iden);
    auto result = inst(*assignExpr.rhs, instructions);
    instructions.push_back(std::make_unique<CopyInst>(result, destination));
    return destination;
}

std::shared_ptr<Value> compoundAssignInst(const Parsing::AssignmentExpr& assignExpr,
                                          std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(assignExpr.lhs.get());
    const Identifier iden(varExpr->name);
    auto lhs = std::make_shared<ValueVar>(iden);
    BinaryInst::Operation operation = convertAssiOperation(assignExpr.op);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    auto rhs = inst(*assignExpr.rhs, instructions);
    instructions.push_back(std::make_unique<BinaryInst>(operation, lhs, rhs, destination));
    instructions.push_back(std::make_unique<CopyInst>(destination, lhs));
    return lhs;
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

std::shared_ptr<Value> constInst(const Parsing::Expr& parsingExpr)
{
    const auto constant = dynamic_cast<const Parsing::ConstExpr*>(&parsingExpr);
    auto result = std::make_unique<ValueConst>(constant->value);
    return result;
}

std::shared_ptr<Value> conditionalExpr(const Parsing::Expr& stmt,
                                       std::vector<std::unique_ptr<Instruction>>& insts)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName());

    Identifier endLabelIden = makeTemporaryName();
    Identifier falseLabelName = makeTemporaryName();

    const auto conditionalExpr = dynamic_cast<const Parsing::ConditionalExpr*>(&stmt);
    auto condition = inst(*conditionalExpr->condition, insts);

    insts.push_back(std::make_unique<JumpIfZeroInst>(condition, falseLabelName));

    auto trueValue = inst(*conditionalExpr->first, insts);
    insts.push_back(std::make_unique<CopyInst>(trueValue, result));
    insts.push_back(std::make_unique<JumpInst>(endLabelIden));

    insts.push_back(std::make_unique<LabelInst>(falseLabelName));
    auto right = inst(*conditionalExpr->second, insts);
    insts.push_back(std::make_unique<CopyInst>(right, result));

    insts.push_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

Identifier makeTemporaryName()
{
    static i32 id = 0;
    std::string prefix = "tmp.";
    prefix += std::to_string(id++);
    return {prefix};
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
            throw std::invalid_argument("Invalid binary operation convertBinaryOperation generateIr");
    }
}

BinaryInst::Operation convertAssiOperation(const Parsing::AssignmentExpr::Operator binaryOperation)
{
    using AssignOper = Parsing::AssignmentExpr::Operator;
    using Ir = BinaryInst::Operation;
    switch (binaryOperation) {
        case AssignOper::BitwiseAndAssign: return Ir::BitwiseAnd;
        case AssignOper::BitwiseOrAssign:  return Ir::BitwiseOr;
        case AssignOper::BitwiseXorAssign: return Ir::BitwiseXor;
        case AssignOper::LeftShiftAssign:  return Ir::LeftShift;
        case AssignOper::RightShiftAssign: return Ir::RightShift;
        case AssignOper::MinusAssign:      return Ir::Subtract;
        case AssignOper::MultiplyAssign:   return Ir::Multiply;
        case AssignOper::DivideAssign:     return Ir::Divide;
        case AssignOper::ModuloAssign:     return Ir::Remainder;
        case AssignOper::PlusAssign:       return Ir::Add;

        default:
            throw std::invalid_argument("Invalid binary operation convertAssiOperation generateIr");
    }
}

bool isPostfixOp(const Parsing::UnaryExpr::Operator oper)
{
    return oper == Parsing::UnaryExpr::Operator::PostFixDecrement
        || oper == Parsing::UnaryExpr::Operator::PostFixIncrement;
}

bool isPrefixOp(const Parsing::UnaryExpr::Operator oper)
{
    return oper == Parsing::UnaryExpr::Operator::PrefixDecrement
        || oper == Parsing::UnaryExpr::Operator::PrefixIncrement;
}

BinaryInst::Operation getPostPrefixOperation(Parsing::UnaryExpr::Operator oper)
{
    using Operator = Parsing::UnaryExpr::Operator;
    switch (oper) {
        case Operator::PostFixDecrement:
        case Operator::PrefixDecrement:
            return BinaryInst::Operation::Subtract;
        case Operator::PostFixIncrement:
        case Operator::PrefixIncrement:
            return BinaryInst::Operation::Add;
        default:
            throw std::invalid_argument("Invalid postfix operation");
    }
}
} // IR