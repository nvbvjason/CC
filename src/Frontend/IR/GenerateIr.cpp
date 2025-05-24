#include "ASTParser.hpp"
#include "GenerateIr.hpp"
#include "CodeGen/AsmAST.hpp"

#include <cassert>
#include <stdexcept>


namespace Ir {
static Identifier makeTemporaryName();
static bool isPostfixOp(Parsing::UnaryExpr::Operator oper);
static bool isPrefixOp(Parsing::UnaryExpr::Operator oper);
static bool isGlobal(Parsing::Declaration::StorageClass storage);
static BinaryInst::Operation getPostPrefixOperation(Parsing::UnaryExpr::Operator oper);
static UnaryInst::Operation convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);
static BinaryInst::Operation convertBinaryOperation(Parsing::BinaryExpr::Operator binaryOperation);
static BinaryInst::Operation convertAssiOperation(Parsing::AssignmentExpr::Operator binaryOperation);

void program(const Parsing::Program& parsingProgram, Program& tackyProgram)
{
    for (const auto& decl : parsingProgram.declarations) {
        auto topLevel = topLevelIr(*decl);
        if (topLevel == nullptr)
            continue;
        tackyProgram.topLevels.push_back(std::move(topLevel));
    }
}

std::unique_ptr<TopLevel> topLevelIr(const Parsing::Declaration& decl)
{
    assert(&decl);
    using Kind = Parsing::Declaration::Kind;
    switch (decl.kind) {
        case Kind::FuncDecl: {
            const auto funcDecl = dynamic_cast<const Parsing::FunDecl*>(&decl);
            if (funcDecl->body == nullptr)
                return nullptr;
            return functionIr(*funcDecl);
        }
        case Kind::VarDecl: {
            const auto varDecl = dynamic_cast<const Parsing::VarDecl*>(&decl);
            if (varDecl->init == nullptr)
                return nullptr;
            if (varDecl->storage != Parsing::Declaration::StorageClass::StaticGlobalInitialized)
                return nullptr;;
            return staticVariableIr(*varDecl);
        }
        assert("topLevelIr");
    }
    std::unreachable();
}

std::unique_ptr<TopLevel> staticVariableIr(const Parsing::VarDecl& varDecl)
{
    std::vector<std::unique_ptr<Instruction>> temp;
    auto value = generateInst(*varDecl.init, temp);
    auto variable = std::make_unique<StaticVariable>(varDecl.name, value, isGlobal(varDecl.storage));
    return variable;
}

std::unique_ptr<TopLevel> functionIr(const Parsing::FunDecl& parsingFunction)
{
    auto functionTacky = std::make_unique<Function>(parsingFunction.name, isGlobal(parsingFunction.storage));
    for (const auto& param : parsingFunction.params)
        functionTacky->args.push_back(Identifier(param));
    generateBlock(*parsingFunction.body, functionTacky->insts);
    return functionTacky;
}

void generateBlock(const Parsing::Block& block, std::vector<std::unique_ptr<Instruction>>& instructions)
{
    for (const std::unique_ptr<Parsing::BlockItem>& item : block.body)
        generateBlockItem(*item, instructions);
}

void generateBlockItem(const Parsing::BlockItem& blockItem,
                       std::vector<std::unique_ptr<Instruction>>& instructions)
{
    using Kind = Parsing::BlockItem::Kind;
    switch (blockItem.kind) {
        case Kind::Declaration: {
            const auto decl = dynamic_cast<const Parsing::DeclBlockItem*>(&blockItem);
            generateDeclaration(*decl->decl, instructions);
            break;
        }
        case Kind::Statement: {
            const auto stmtBlockItem = dynamic_cast<const Parsing::StmtBlockItem*>(&blockItem);
            generateStmt(*stmtBlockItem->stmt, instructions);
            break;
        }
        default:
            throw std::invalid_argument("Unexpected block item type ir generateBlockItem");
    }
}

void generateForInit(const Parsing::ForInit& forInit,
                     std::vector<std::unique_ptr<Instruction>>& insts)
{
    if (forInit.kind == Parsing::ForInit::Kind::Declaration) {
        auto decl = dynamic_cast<const Parsing::DeclForInit*>(&forInit);
        generateDeclaration(*decl->decl, insts);
        return;
    }
    if (forInit.kind == Parsing::ForInit::Kind::Expression) {
        auto expr = dynamic_cast<const Parsing::ExprForInit*>(&forInit);
        if (expr->expression)
            generateInst(*expr->expression, insts);
        return;
    }
}

void generateDeclaration(const Parsing::Declaration& declaration,
                         std::vector<std::unique_ptr<Instruction>>& insts)
{
    if (declaration.kind == Parsing::Declaration::Kind::VarDecl) {
        const auto varDecl = dynamic_cast<const Parsing::VarDecl*>(&declaration);
        if (varDecl->init == nullptr)
            return;
        auto value = generateInst(*varDecl->init, insts);
        auto temporary = std::make_shared<ValueVar>(makeTemporaryName());
        insts.push_back(std::make_unique<CopyInst>(value, temporary));
        const Identifier iden(varDecl->name);
        auto var = std::make_shared<ValueVar>(iden);
        insts.push_back(std::make_unique<CopyInst>(temporary, var));
    }
}

void generateStmt(const Parsing::Stmt& stmt,
                  std::vector<std::unique_ptr<Instruction>>& insts)
{
    using Kind = Parsing::Stmt::Kind;
    switch (stmt.kind) {
        case Kind::If: {
            const auto parseIffStmt = dynamic_cast<const Parsing::IfStmt*>(&stmt);
            if (parseIffStmt->elseStmt == nullptr)
                generateIfStmt(*parseIffStmt, insts);
            else
                generateIfElseStmt(*parseIffStmt, insts);
            break;
        }
        case Kind::Return: {
            const auto returnStmt = dynamic_cast<const Parsing::ReturnStmt*>(&stmt);
            auto value = generateInst(*returnStmt->expr, insts);
            if (value == nullptr)
                break;
            insts.push_back(std::make_unique<ReturnInst>(value));
            break;
        }
        case Kind::Expression: {
            const auto stmtExpr = dynamic_cast<const Parsing::ExprStmt*>(&stmt);
            generateInst(*stmtExpr->expr, insts);
            break;
        }
        case Kind::Goto: {
            const auto gotoStmt = dynamic_cast<const Parsing::GotoStmt*>(&stmt);
            generateGotoStmt(*gotoStmt, insts);
            break;
        }
        case Kind::Compound: {
            const auto compoundStmtPtr = dynamic_cast<const Parsing::CompoundStmt*>(&stmt);
            generateCompoundStmt(*compoundStmtPtr, insts);
            break;
        }
        case Kind::Break: {
            const auto breakStmtPtr = dynamic_cast<const Parsing::BreakStmt*>(&stmt);
            generateBreakStmt(*breakStmtPtr, insts);
            break;
        }
        case Kind::Continue: {
            const auto continueStmtPtr = dynamic_cast<const Parsing::ContinueStmt*>(&stmt);
            generateContinueStmt(*continueStmtPtr, insts);
            break;
        }
        case Kind::Label: {
            const auto labelStmtPtr = dynamic_cast<const Parsing::LabelStmt*>(&stmt);
            generateLabelStmt(*labelStmtPtr, insts);
            break;
        }
        case Kind::Case: {
            const auto caseStmtPtr = dynamic_cast<const Parsing::CaseStmt*>(&stmt);
            generateCaseStmt(*caseStmtPtr, insts);
            break;
        }
        case Kind::Default: {
            const auto defaultStmtPtr = dynamic_cast<const Parsing::DefaultStmt*>(&stmt);
            generateDefaultStmt(*defaultStmtPtr, insts);
            break;
        }
        case Kind::DoWhile: {
            const auto doWhileStmtPtr = dynamic_cast<const Parsing::DoWhileStmt*>(&stmt);
            generateDoWhileStmt(*doWhileStmtPtr, insts);
            break;
        }
        case Kind::While: {
            const auto whileStmtPtr = dynamic_cast<const Parsing::WhileStmt*>(&stmt);
            generateWhileStmt(*whileStmtPtr, insts);
            break;
        }
        case Kind::For: {
            const auto forStmtPtr = dynamic_cast<const Parsing::ForStmt*>(&stmt);
            generateForStmt(*forStmtPtr, insts);
            break;
        }
        case Kind::Switch: {
            const auto switchStmtPtr = dynamic_cast<const Parsing::SwitchStmt*>(&stmt);
            generateSwitchStmt(*switchStmtPtr, insts);
            break;
        }
        case Kind::Null:
            break;
        default:
            throw std::invalid_argument("Unexpected statement type Ir generate");
    }
}

void generateIfStmt(const Parsing::IfStmt& parseIfStmt,
                    std::vector<std::unique_ptr<Instruction>>& insts)
{
    auto condition = generateInst(*parseIfStmt.condition, insts);
    Identifier endLabelIden = makeTemporaryName();
    insts.push_back(std::make_unique<JumpIfZeroInst>(condition, endLabelIden));
    generateStmt(*parseIfStmt.thenStmt, insts);
    insts.push_back(std::make_unique<LabelInst>(endLabelIden));
}

void generateIfElseStmt(const Parsing::IfStmt& parseIfStmt,
                        std::vector<std::unique_ptr<Instruction>>& insts)
{
    auto condition = generateInst(*parseIfStmt.condition, insts);
    Identifier elseStmtLabel = makeTemporaryName();
    insts.push_back(std::make_unique<JumpIfZeroInst>(condition, elseStmtLabel));
    generateStmt(*parseIfStmt.thenStmt, insts);
    Identifier endLabelIden = makeTemporaryName();
    insts.push_back(std::make_unique<JumpInst>(endLabelIden));
    insts.push_back(std::make_unique<LabelInst>(elseStmtLabel));
    generateStmt(*parseIfStmt.elseStmt, insts);
    insts.push_back(std::make_unique<LabelInst>(endLabelIden));
}

void generateGotoStmt(const Parsing::GotoStmt& stmt,
                      std::vector<std::unique_ptr<Instruction>>& insts)
{
    insts.push_back(
        std::make_unique<JumpInst>(Identifier(stmt.identifier + ".label"))
        );
}

void generateCompoundStmt(const Parsing::CompoundStmt& stmt,
                          std::vector<std::unique_ptr<Instruction>>& insts)
{
    generateBlock(*stmt.block, insts);
}

void generateBreakStmt(const Parsing::BreakStmt& stmt,
                       std::vector<std::unique_ptr<Instruction>>& insts)
{
    insts.push_back(
        std::make_unique<JumpInst>(Identifier(stmt.identifier + "break"))
        );
}

void generateContinueStmt(const Parsing::ContinueStmt& stmt,
                          std::vector<std::unique_ptr<Instruction>>& insts)
{
    insts.push_back(
        std::make_unique<JumpInst>(Identifier(stmt.identifier + "continue"))
        );
}

void generateLabelStmt(const Parsing::LabelStmt& labelStmt,
                       std::vector<std::unique_ptr<Instruction>>& insts)
{
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(labelStmt.identifier + ".label"))
        );
    generateStmt(*labelStmt.stmt, insts);
}

void generateCaseStmt(const Parsing::CaseStmt& caseStmt,
                      std::vector<std::unique_ptr<Instruction>>& insts)
{
    const auto constExpr = dynamic_cast<const Parsing::ConstExpr*>(caseStmt.condition.get());
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(caseStmt.identifier + std::to_string(constExpr->value)))
    );
    generateStmt(*caseStmt.body, insts);
}

void generateDefaultStmt(const Parsing::DefaultStmt& defaultStmt,
                         std::vector<std::unique_ptr<Instruction>>& insts)
{
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(defaultStmt.identifier + "default"))
    );
    generateStmt(*defaultStmt.body, insts);
}

void generateDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt,
                         std::vector<std::unique_ptr<Instruction>>& insts)
{
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "start"))
    );
    generateStmt(*doWhileStmt.body, insts);
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "continue"))
    );
    auto condition = generateInst(*doWhileStmt.condition, insts);
    insts.push_back(
        std::make_unique<JumpIfNotZeroInst>(condition, Identifier(doWhileStmt.identifier + "start"))
    );
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "break"))
    );
}

void generateWhileStmt(const Parsing::WhileStmt& whileStmt,
                       std::vector<std::unique_ptr<Instruction>>& insts)
{
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(whileStmt.identifier + "continue"))
    );
    auto condition = generateInst(*whileStmt.condition, insts);
    insts.push_back(
        std::make_unique<JumpIfZeroInst>(condition, Identifier(whileStmt.identifier + "break"))
    );
    generateStmt(*whileStmt.body, insts);
    insts.push_back(
        std::make_unique<JumpInst>(Identifier(whileStmt.identifier + "continue"))
    );
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(whileStmt.identifier + "break"))
    );
}

void generateForStmt(const Parsing::ForStmt& forStmt,
                     std::vector<std::unique_ptr<Instruction>>& insts)
{
    if (forStmt.init)
        generateForInit(*forStmt.init, insts);
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(forStmt.identifier + "start"))
    );
    if (forStmt.condition) {
        auto condition = generateInst(*forStmt.condition, insts);
        insts.push_back(
            std::make_unique<JumpIfZeroInst>(condition, Identifier(forStmt.identifier + "break"))
        );
    }
    generateStmt(*forStmt.body, insts);
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(forStmt.identifier + "continue"))
    );
    if (forStmt.post)
        generateInst(*forStmt.post, insts);
    insts.push_back(
        std::make_unique<JumpInst>(Identifier(forStmt.identifier + "start"))
    );
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(forStmt.identifier + "break"))
    );
}

void generateSwitchStmt(const Parsing::SwitchStmt& stmt,
                        std::vector<std::unique_ptr<Instruction>>& insts)
{
    auto realValue = generateInst(*stmt.condition, insts);
    for (const auto& caseStmt : stmt.cases) {
        const auto constExpr = dynamic_cast<const Parsing::ConstExpr*>(caseStmt.get());
        const auto destination = std::make_shared<ValueVar>(makeTemporaryName());
        insts.push_back(
            std::make_unique<BinaryInst>(BinaryInst::Operation::Equal,
                                         realValue,
                                         std::make_shared<ValueConst>(constExpr->value),
                                         destination)
            );
        insts.push_back(
            std::make_unique<JumpIfNotZeroInst>(destination,
                Identifier(stmt.identifier + std::to_string(constExpr->value)))
        );
    }
    if (stmt.hasDefault)
        insts.push_back(
            std::make_unique<JumpInst>(Identifier(stmt.identifier + "default"))
            );
    else
        insts.push_back(
            std::make_unique<JumpInst>(Identifier(stmt.identifier + "break")));
    generateStmt(*stmt.body, insts);
    insts.push_back(
        std::make_unique<LabelInst>(Identifier(stmt.identifier + "break"))
    );
}

std::shared_ptr<Value> generateInst(const Parsing::Expr& parsingExpr,
                                    std::vector<std::unique_ptr<Instruction>>& instructions)
{
    using ExprKind = Parsing::Expr::Kind;
    switch (parsingExpr.kind) {
        case ExprKind::Var:
            return generateVarInst(parsingExpr);
        case ExprKind::Constant:
            return generateConstInst(parsingExpr);
        case ExprKind::Unary:
            return generateUnaryInst(parsingExpr, instructions);
        case ExprKind::Binary:
            return generateBinaryInst(parsingExpr, instructions);
        case ExprKind::Assignment:
            return generateAssignInst(parsingExpr, instructions);
        case ExprKind::Conditional:
            return generateConditionalInst(parsingExpr, instructions);
        case ExprKind::FunctionCall:
            return generateFuncCallInst(parsingExpr, instructions);
        default:
            assert("Unexpected expression type ir generateInst");
            std::unreachable();
    }
}

std::shared_ptr<Value> unaryPostfixInst(const Parsing::UnaryExpr& unaryExpr,
                                        std::vector<std::unique_ptr<Instruction>>& instructions)
{
    auto originalForReturn = std::make_shared<ValueVar>(makeTemporaryName());
    auto tempNew = std::make_shared<ValueVar>(makeTemporaryName());
    auto original = generateInst(*unaryExpr.operand, instructions);
    instructions.push_back(std::make_unique<CopyInst>(original, originalForReturn));
    auto operation = std::make_shared<ValueConst>(1);
    instructions.push_back(std::make_unique<BinaryInst>(
        getPostPrefixOperation(unaryExpr.op), originalForReturn, operation, tempNew
    ));
    instructions.push_back(std::make_unique<CopyInst>(tempNew, original));
    return originalForReturn;
}

std::shared_ptr<Value> generateunaryPrefixInst(const Parsing::UnaryExpr& unaryExpr,
                                               std::vector<std::unique_ptr<Instruction>>& instructions)
{
    auto original = generateInst(*unaryExpr.operand, instructions);
    auto temp = std::make_shared<ValueVar>(Identifier(makeTemporaryName()));
    instructions.push_back(std::make_unique<BinaryInst>(
        getPostPrefixOperation(unaryExpr.op), original, std::make_shared<ValueConst>(1), temp)
    );
    instructions.push_back(std::make_unique<CopyInst>(temp, original));
    return temp;
}

std::shared_ptr<Value> generateUnaryInst(const Parsing::Expr& parsingExpr,
                                         std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto unaryParsingPtr = dynamic_cast<const Parsing::UnaryExpr*>(&parsingExpr);
    if (isPostfixOp(unaryParsingPtr->op))
        return unaryPostfixInst(*unaryParsingPtr, instructions);
    if (isPrefixOp(unaryParsingPtr->op))
        return generateunaryPrefixInst(*unaryParsingPtr, instructions);
    if (unaryParsingPtr->op == Parsing::UnaryExpr::Operator::Plus)
        return generateInst(*unaryParsingPtr->operand, instructions);
    UnaryInst::Operation operation = convertUnaryOperation(unaryParsingPtr->op);
    auto source = generateInst(*unaryParsingPtr->operand, instructions);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    instructions.push_back(std::make_unique<UnaryInst>(operation, source, destination));
    return destination;
}

std::shared_ptr<Value> generateVarInst(const Parsing::Expr& parsingExpr)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(&parsingExpr);
    const Identifier iden(varExpr->name);
    auto var = std::make_shared<ValueVar>(iden);
    return var;
}

std::shared_ptr<Value> generateBinaryInst(const Parsing::Expr& parsingExpr,
                                          std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto binaryParsing = dynamic_cast<const Parsing::BinaryExpr*>(&parsingExpr);
    BinaryInst::Operation operation = convertBinaryOperation(binaryParsing->op);
    if (operation == BinaryInst::Operation::And)
        return generateBinaryAndInst(*binaryParsing, instructions);
    if (operation == BinaryInst::Operation::Or)
        return generateBinaryOrInst(*binaryParsing, instructions);
    auto source1 = generateInst(*binaryParsing->lhs, instructions);
    auto source2 = generateInst(*binaryParsing->rhs, instructions);

    auto destination = std::make_shared<ValueVar>(makeTemporaryName());

    instructions.push_back(std::make_unique<BinaryInst>(operation, source1, source2, destination));
    return destination;
}

std::shared_ptr<Value> generateAssignInst(const Parsing::Expr& binaryExpr,
                                          std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto assignExpr = dynamic_cast<const Parsing::AssignmentExpr*>(&binaryExpr);
    if (assignExpr->op != Parsing::AssignmentExpr::Operator::Assign)
        return generateCompoundAssignInst(*assignExpr, instructions);
    return generateSimpleAssignInst(*assignExpr, instructions);
}

std::shared_ptr<Value> generateSimpleAssignInst(const Parsing::AssignmentExpr& assignExpr,
                                                std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(assignExpr.lhs.get());
    const Identifier iden(varExpr->name);
    auto destination = std::make_shared<ValueVar>(iden);
    auto result = generateInst(*assignExpr.rhs, instructions);
    instructions.push_back(std::make_unique<CopyInst>(result, destination));
    return destination;
}

std::shared_ptr<Value> generateCompoundAssignInst(const Parsing::AssignmentExpr& assignExpr,
                                                  std::vector<std::unique_ptr<Instruction>>& instructions)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(assignExpr.lhs.get());
    const Identifier iden(varExpr->name);
    auto lhs = std::make_shared<ValueVar>(iden);
    BinaryInst::Operation operation = convertAssiOperation(assignExpr.op);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    auto rhs = generateInst(*assignExpr.rhs, instructions);
    instructions.push_back(std::make_unique<BinaryInst>(operation, lhs, rhs, destination));
    instructions.push_back(std::make_unique<CopyInst>(destination, lhs));
    return lhs;
}

std::shared_ptr<Value> generateBinaryAndInst(const Parsing::BinaryExpr& parsingExpr,
                                             std::vector<std::unique_ptr<Instruction>>& instructions)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName());
    auto lhs = generateInst(*parsingExpr.lhs, instructions);
    Identifier falseLabelIden = makeTemporaryName();
    instructions.push_back(std::make_unique<JumpIfZeroInst>(lhs, falseLabelIden));
    auto rhs = generateInst(*parsingExpr.rhs, instructions);
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

std::shared_ptr<Value> generateBinaryOrInst(const Parsing::BinaryExpr& binaryExpr,
                                            std::vector<std::unique_ptr<Instruction>>& instructions)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName());
    auto lhs = generateInst(*binaryExpr.lhs, instructions);
    Identifier trueLabelIden = makeTemporaryName();
    instructions.push_back(std::make_unique<JumpIfNotZeroInst>(lhs, trueLabelIden));
    auto rhs = generateInst(*binaryExpr.rhs, instructions);
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

std::shared_ptr<Value> generateConstInst(const Parsing::Expr& parsingExpr)
{
    const auto constant = dynamic_cast<const Parsing::ConstExpr*>(&parsingExpr);
    auto result = std::make_unique<ValueConst>(constant->value);
    return result;
}

std::shared_ptr<Value> generateConditionalInst(const Parsing::Expr& stmt,
                                               std::vector<std::unique_ptr<Instruction>>& insts)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName());

    Identifier endLabelIden = makeTemporaryName();
    Identifier falseLabelName = makeTemporaryName();

    const auto conditionalExpr = dynamic_cast<const Parsing::ConditionalExpr*>(&stmt);
    auto condition = generateInst(*conditionalExpr->condition, insts);

    insts.push_back(std::make_unique<JumpIfZeroInst>(condition, falseLabelName));

    auto trueValue = generateInst(*conditionalExpr->first, insts);
    insts.push_back(std::make_unique<CopyInst>(trueValue, result));
    insts.push_back(std::make_unique<JumpInst>(endLabelIden));

    insts.push_back(std::make_unique<LabelInst>(falseLabelName));
    auto right = generateInst(*conditionalExpr->second, insts);
    insts.push_back(std::make_unique<CopyInst>(right, result));

    insts.push_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<Value> generateFuncCallInst(const Parsing::Expr& stmt,
                                            std::vector<std::unique_ptr<Instruction> >& insts)
{
    const auto parseFunction = dynamic_cast<const Parsing::FunCallExpr*>(&stmt);
    std::vector<std::shared_ptr<Value>> arguments;
    arguments.reserve(parseFunction->args.size());
    for (const auto& expr : parseFunction->args)
        arguments.push_back(generateInst(*expr, insts));
    auto dst = std::make_shared<ValueVar>(makeTemporaryName());
    insts.push_back(std::make_unique<FunCallInst>(
        Identifier(parseFunction->name), std::move(arguments), dst
        ));
    return dst;
}

Identifier makeTemporaryName()
{
    static i32 id = 0;
    std::string prefix = "tmp.";
    prefix += std::to_string(id++);
    return {prefix};
}

static bool isGlobal(const Parsing::Declaration::StorageClass storage)
{
    using StorageClass = Parsing::Declaration::StorageClass;
    switch (storage) {
        case StorageClass::GlobalDeclaration:
        case StorageClass::ExternFunction:
        case StorageClass::ExternGlobal:
        case StorageClass::ExternGlobalInitialized:
        case StorageClass::GlobalDefinition:
        case StorageClass::StaticGlobalInitialized:
        case StorageClass::StaticGlobalTentative:
            return true;
        case StorageClass::AutoLocalScope:
        case StorageClass::StaticLocal:
        case StorageClass::ExternLocal:
            return false;
        default:
            std::unreachable();
    }
}

UnaryInst::Operation convertUnaryOperation(const Parsing::UnaryExpr::Operator unaryOperation)
{
    using Operator = Parsing::UnaryExpr::Operator;
    using OperationIr = UnaryInst::Operation;
    switch (unaryOperation) {
        case Operator::Complement:  return OperationIr::Complement;
        case Operator::Negate:      return OperationIr::Negate;
        case Operator::Not:         return OperationIr::Not;
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