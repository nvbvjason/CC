#include "ASTParser.hpp"
#include "GenerateIr.hpp"
#include "CodeGen/AsmAST.hpp"

#include <cassert>
#include <stdexcept>

namespace Ir {
static Identifier makeTemporaryName();
static bool isPostfixOp(Parsing::UnaryExpr::Operator oper);
static bool isPrefixOp(Parsing::UnaryExpr::Operator oper);
static BinaryInst::Operation getPostPrefixOperation(Parsing::UnaryExpr::Operator oper);
static UnaryInst::Operation convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);
static BinaryInst::Operation convertBinaryOperation(Parsing::BinaryExpr::Operator binaryOperation);
static BinaryInst::Operation convertAssiOperation(Parsing::AssignmentExpr::Operator binaryOperation);

void GenerateIr::program(const Parsing::Program& parsingProgram, Program& tackyProgram)
{
    for (const std::unique_ptr<Parsing::Declaration>& decl : parsingProgram.declarations) {
        std::unique_ptr<TopLevel> topLevel = topLevelIr(*decl);
        if (topLevel == nullptr)
            continue;
        m_topLevels.push_back(std::move(topLevel));
    }
    tackyProgram.topLevels = std::move(m_topLevels);
}

std::unique_ptr<TopLevel> GenerateIr::topLevelIr(const Parsing::Declaration& decl)
{
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
            return staticVariableIr(*varDecl);
        }
    }
    std::unreachable();
}

std::unique_ptr<TopLevel> GenerateIr::staticVariableIr(const Parsing::VarDecl& varDecl)
{
    const auto entry = m_symbolTable.lookupVar(varDecl.name);
    if (varDecl.init == nullptr && entry.isSet(SymbolTable::State::Defined))
        return nullptr;
    if (!entry.isSet(SymbolTable::State::Defined) && varDecl.storage == Storage::Extern)
        return nullptr;
    if (m_writtenGlobals.contains(varDecl.name))
        return nullptr;
    m_writtenGlobals.insert(varDecl.name);
    std::shared_ptr<Value> value;
    if (!entry.isSet(SymbolTable::State::Defined))
        value = std::make_shared<ValueConst>(0);
    else
        value = generateInst(*varDecl.init);
    auto variable = std::make_unique<StaticVariable>(varDecl.name, value, varDecl.storage != Storage::Static);
    return variable;
}

std::unique_ptr<TopLevel> GenerateIr::functionIr(const Parsing::FunDecl& parsingFunction)
{
    bool global = !m_symbolTable.lookupFunc(parsingFunction.name).isSet(SymbolTable::State::InternalLinkage);
    auto functionTacky = std::make_unique<Function>(parsingFunction.name, global);
    m_global = true;;
    m_instructions = std::move(functionTacky->insts);
    for (const auto& param : parsingFunction.params)
        functionTacky->args.push_back(Identifier(param));
    generateBlock(*parsingFunction.body);
    functionTacky->insts = std::move(m_instructions);
    m_global = false;
    return functionTacky;
}

void GenerateIr::generateBlock(const Parsing::Block& block)
{
    for (const std::unique_ptr<Parsing::BlockItem>& item : block.body)
        generateBlockItem(*item);
}

void GenerateIr::generateBlockItem(const Parsing::BlockItem& blockItem)
{
    using Kind = Parsing::BlockItem::Kind;
    switch (blockItem.kind) {
        case Kind::Declaration: {
            const auto decl = dynamic_cast<const Parsing::DeclBlockItem*>(&blockItem);
            generateDeclaration(*decl->decl);
            break;
        }
        case Kind::Statement: {
            const auto stmtBlockItem = dynamic_cast<const Parsing::StmtBlockItem*>(&blockItem);
            generateStmt(*stmtBlockItem->stmt);
            break;
        }
        default:
            throw std::invalid_argument("Unexpected block item type ir generateBlockItem");
    }
}

void GenerateIr::generateForInit(const Parsing::ForInit& forInit)
{
    if (forInit.kind == Parsing::ForInit::Kind::Declaration) {
        auto decl = dynamic_cast<const Parsing::DeclForInit*>(&forInit);
        generateDeclaration(*decl->decl);
        return;
    }
    if (forInit.kind == Parsing::ForInit::Kind::Expression) {
        auto expr = dynamic_cast<const Parsing::ExprForInit*>(&forInit);
        if (expr->expression)
            generateInst(*expr->expression);
    }
}

void GenerateIr::generateDeclaration(const Parsing::Declaration& declaration)
{
    if (declaration.kind == Parsing::Declaration::Kind::VarDecl) {
        const auto varDecl = dynamic_cast<const Parsing::VarDecl*>(&declaration);
        if (varDecl->init == nullptr)
            return;
        if (varDecl->storage == Storage::Static)
            return generateDeclarationStaticLocal(*varDecl);
        std::shared_ptr<Value> value = generateInst(*varDecl->init);
        auto temporary = std::make_shared<ValueVar>(makeTemporaryName());
        m_instructions.push_back(std::make_unique<CopyInst>(value, temporary));
        const Identifier iden(varDecl->name);
        auto var = std::make_shared<ValueVar>(iden);
        m_instructions.push_back(std::make_unique<CopyInst>(temporary, var));
    }
}

void GenerateIr::generateDeclarationStaticLocal(const Parsing::VarDecl& varDecl)
{
    std::shared_ptr<Value> value;
    const bool defined = varDecl.init != nullptr;
    if (!defined)
        value = std::make_shared<ValueConst>(0);
    else
        value = generateInst(*varDecl.init);
    auto variable = std::make_unique<StaticVariable>(
        varDecl.name, value, false);
    m_topLevels.push_back(std::move(variable));
     m_symbolTable.addVarEntry(varDecl.name, varDecl.name, true, false, false, defined);
}

void GenerateIr::generateStmt(const Parsing::Stmt& stmt)
{
    using Kind = Parsing::Stmt::Kind;
    switch (stmt.kind) {
        case Kind::If: {
            const auto parseIffStmt = dynamic_cast<const Parsing::IfStmt*>(&stmt);
            if (parseIffStmt->elseStmt == nullptr)
                generateIfStmt(*parseIffStmt);
            else
                generateIfElseStmt(*parseIffStmt);
            break;
        }
        case Kind::Return: {
            const auto returnStmt = dynamic_cast<const Parsing::ReturnStmt*>(&stmt);
            auto value = generateInst(*returnStmt->expr);
            if (value == nullptr)
                break;
            m_instructions.push_back(std::make_unique<ReturnInst>(value));
            break;
        }
        case Kind::Expression: {
            const auto stmtExpr = dynamic_cast<const Parsing::ExprStmt*>(&stmt);
            generateInst(*stmtExpr->expr);
            break;
        }
        case Kind::Goto: {
            const auto gotoStmt = dynamic_cast<const Parsing::GotoStmt*>(&stmt);
            generateGotoStmt(*gotoStmt);
            break;
        }
        case Kind::Compound: {
            const auto compoundStmtPtr = dynamic_cast<const Parsing::CompoundStmt*>(&stmt);
            generateCompoundStmt(*compoundStmtPtr);
            break;
        }
        case Kind::Break: {
            const auto breakStmtPtr = dynamic_cast<const Parsing::BreakStmt*>(&stmt);
            generateBreakStmt(*breakStmtPtr);
            break;
        }
        case Kind::Continue: {
            const auto continueStmtPtr = dynamic_cast<const Parsing::ContinueStmt*>(&stmt);
            generateContinueStmt(*continueStmtPtr);
            break;
        }
        case Kind::Label: {
            const auto labelStmtPtr = dynamic_cast<const Parsing::LabelStmt*>(&stmt);
            generateLabelStmt(*labelStmtPtr);
            break;
        }
        case Kind::Case: {
            const auto caseStmtPtr = dynamic_cast<const Parsing::CaseStmt*>(&stmt);
            generateCaseStmt(*caseStmtPtr);
            break;
        }
        case Kind::Default: {
            const auto defaultStmtPtr = dynamic_cast<const Parsing::DefaultStmt*>(&stmt);
            generateDefaultStmt(*defaultStmtPtr);
            break;
        }
        case Kind::DoWhile: {
            const auto doWhileStmtPtr = dynamic_cast<const Parsing::DoWhileStmt*>(&stmt);
            generateDoWhileStmt(*doWhileStmtPtr);
            break;
        }
        case Kind::While: {
            const auto whileStmtPtr = dynamic_cast<const Parsing::WhileStmt*>(&stmt);
            generateWhileStmt(*whileStmtPtr);
            break;
        }
        case Kind::For: {
            const auto forStmtPtr = dynamic_cast<const Parsing::ForStmt*>(&stmt);
            generateForStmt(*forStmtPtr);
            break;
        }
        case Kind::Switch: {
            const auto switchStmtPtr = dynamic_cast<const Parsing::SwitchStmt*>(&stmt);
            generateSwitchStmt(*switchStmtPtr);
            break;
        }
        case Kind::Null:
            break;
        default:
            throw std::invalid_argument("Unexpected statement type Ir generate");
    }
}

void GenerateIr::generateIfStmt(const Parsing::IfStmt& parseIfStmt)
{
    auto condition = generateInst(*parseIfStmt.condition);
    Identifier endLabelIden = makeTemporaryName();
    m_instructions.push_back(std::make_unique<JumpIfZeroInst>(condition, endLabelIden));
    generateStmt(*parseIfStmt.thenStmt);
    m_instructions.push_back(std::make_unique<LabelInst>(endLabelIden));
}

void GenerateIr::generateIfElseStmt(const Parsing::IfStmt& parseIfStmt)
{
    auto condition = generateInst(*parseIfStmt.condition);
    Identifier elseStmtLabel = makeTemporaryName();
    m_instructions.push_back(std::make_unique<JumpIfZeroInst>(condition, elseStmtLabel));
    generateStmt(*parseIfStmt.thenStmt);
    Identifier endLabelIden = makeTemporaryName();
    m_instructions.push_back(std::make_unique<JumpInst>(endLabelIden));
    m_instructions.push_back(std::make_unique<LabelInst>(elseStmtLabel));
    generateStmt(*parseIfStmt.elseStmt);
    m_instructions.push_back(std::make_unique<LabelInst>(endLabelIden));
}

void GenerateIr::generateGotoStmt(const Parsing::GotoStmt& stmt)
{
    m_instructions.push_back(
        std::make_unique<JumpInst>(Identifier(stmt.identifier + ".label"))
        );
}

void GenerateIr::generateCompoundStmt(const Parsing::CompoundStmt& stmt)
{
    generateBlock(*stmt.block);
}

void GenerateIr::generateBreakStmt(const Parsing::BreakStmt& stmt)
{
    m_instructions.push_back(
        std::make_unique<JumpInst>(Identifier(stmt.identifier + "break"))
        );
}

void GenerateIr::generateContinueStmt(const Parsing::ContinueStmt& stmt)
{
    m_instructions.push_back(
        std::make_unique<JumpInst>(Identifier(stmt.identifier + "continue"))
        );
}

void GenerateIr::generateLabelStmt(const Parsing::LabelStmt& labelStmt)
{
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(labelStmt.identifier + ".label"))
        );
    generateStmt(*labelStmt.stmt);
}

void GenerateIr::generateCaseStmt(const Parsing::CaseStmt& caseStmt)
{
    const auto constExpr = dynamic_cast<const Parsing::ConstExpr*>(caseStmt.condition.get());
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(caseStmt.identifier + std::to_string(constExpr->value)))
    );
    generateStmt(*caseStmt.body);
}

void GenerateIr::generateDefaultStmt(const Parsing::DefaultStmt& defaultStmt)
{
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(defaultStmt.identifier + "default"))
    );
    generateStmt(*defaultStmt.body);
}

void GenerateIr::generateDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt)
{
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "start"))
    );
    generateStmt(*doWhileStmt.body);
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "continue"))
    );
    auto condition = generateInst(*doWhileStmt.condition);
    m_instructions.push_back(
        std::make_unique<JumpIfNotZeroInst>(condition, Identifier(doWhileStmt.identifier + "start"))
    );
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "break"))
    );
}

void GenerateIr::generateWhileStmt(const Parsing::WhileStmt& whileStmt)
{
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(whileStmt.identifier + "continue"))
    );
    auto condition = generateInst(*whileStmt.condition);
    m_instructions.push_back(
        std::make_unique<JumpIfZeroInst>(condition, Identifier(whileStmt.identifier + "break"))
    );
    generateStmt(*whileStmt.body);
    m_instructions.push_back(
        std::make_unique<JumpInst>(Identifier(whileStmt.identifier + "continue"))
    );
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(whileStmt.identifier + "break"))
    );
}

void GenerateIr::generateForStmt(const Parsing::ForStmt& forStmt)
{
    if (forStmt.init)
        generateForInit(*forStmt.init);
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(forStmt.identifier + "start"))
    );
    if (forStmt.condition) {
        auto condition = generateInst(*forStmt.condition);
        m_instructions.push_back(
            std::make_unique<JumpIfZeroInst>(condition, Identifier(forStmt.identifier + "break"))
        );
    }
    generateStmt(*forStmt.body);
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(forStmt.identifier + "continue"))
    );
    if (forStmt.post)
        generateInst(*forStmt.post);
    m_instructions.push_back(
        std::make_unique<JumpInst>(Identifier(forStmt.identifier + "start"))
    );
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(forStmt.identifier + "break"))
    );
}

void GenerateIr::generateSwitchStmt(const Parsing::SwitchStmt& stmt)
{
    auto realValue = generateInst(*stmt.condition);
    for (const auto& caseStmt : stmt.cases) {
        const auto constExpr = dynamic_cast<const Parsing::ConstExpr*>(caseStmt.get());
        const auto destination = std::make_shared<ValueVar>(makeTemporaryName());
        m_instructions.push_back(
            std::make_unique<BinaryInst>(BinaryInst::Operation::Equal,
                                         realValue,
                                         std::make_shared<ValueConst>(constExpr->value),
                                         destination)
            );
        m_instructions.push_back(
            std::make_unique<JumpIfNotZeroInst>(destination,
                Identifier(stmt.identifier + std::to_string(constExpr->value)))
        );
    }
    if (stmt.hasDefault)
        m_instructions.push_back(
            std::make_unique<JumpInst>(Identifier(stmt.identifier + "default"))
            );
    else
        m_instructions.push_back(
            std::make_unique<JumpInst>(Identifier(stmt.identifier + "break")));
    generateStmt(*stmt.body);
    m_instructions.push_back(
        std::make_unique<LabelInst>(Identifier(stmt.identifier + "break"))
    );
}

std::shared_ptr<Value> GenerateIr::generateInst(const Parsing::Expr& parsingExpr)
{
    using ExprKind = Parsing::Expr::Kind;
    switch (parsingExpr.kind) {
        case ExprKind::Var:
            return generateVarInst(parsingExpr);
        case ExprKind::Constant:
            return generateConstInst(parsingExpr);
        case ExprKind::Unary:
            return generateUnaryInst(parsingExpr);
        case ExprKind::Binary:
            return generateBinaryInst(parsingExpr);
        case ExprKind::Assignment:
            return generateAssignInst(parsingExpr);
        case ExprKind::Conditional:
            return generateConditionalInst(parsingExpr);
        case ExprKind::FunctionCall:
            return generateFuncCallInst(parsingExpr);
        default:
            assert("Unexpected expression type ir generateInst");
            std::unreachable();
    }
}

std::shared_ptr<Value> GenerateIr::generateUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr)
{
    auto originalForReturn = std::make_shared<ValueVar>(makeTemporaryName());
    auto tempNew = std::make_shared<ValueVar>(makeTemporaryName());
    auto original = generateInst(*unaryExpr.operand);
    m_instructions.push_back(std::make_unique<CopyInst>(original, originalForReturn));
    auto operation = std::make_shared<ValueConst>(1);
    m_instructions.push_back(std::make_unique<BinaryInst>(
        getPostPrefixOperation(unaryExpr.op), originalForReturn, operation, tempNew
    ));
    m_instructions.push_back(std::make_unique<CopyInst>(tempNew, original));
    return originalForReturn;
}

std::shared_ptr<Value> GenerateIr::generateUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr)
{
    auto original = generateInst(*unaryExpr.operand);
    auto temp = std::make_shared<ValueVar>(Identifier(makeTemporaryName()));
    m_instructions.push_back(std::make_unique<BinaryInst>(
        getPostPrefixOperation(unaryExpr.op), original, std::make_shared<ValueConst>(1), temp)
    );
    m_instructions.push_back(std::make_unique<CopyInst>(temp, original));
    return temp;
}

std::shared_ptr<Value> GenerateIr::generateUnaryInst(const Parsing::Expr& parsingExpr)
{
    const auto unaryParsingPtr = dynamic_cast<const Parsing::UnaryExpr*>(&parsingExpr);
    if (isPostfixOp(unaryParsingPtr->op))
        return generateUnaryPostfixInst(*unaryParsingPtr);
    if (isPrefixOp(unaryParsingPtr->op))
        return generateUnaryPrefixInst(*unaryParsingPtr);
    if (unaryParsingPtr->op == Parsing::UnaryExpr::Operator::Plus)
        return generateInst(*unaryParsingPtr->operand);
    UnaryInst::Operation operation = convertUnaryOperation(unaryParsingPtr->op);
    auto source = generateInst(*unaryParsingPtr->operand);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    m_instructions.push_back(std::make_unique<UnaryInst>(operation, source, destination));
    return destination;
}

std::shared_ptr<Value> GenerateIr::generateVarInst(const Parsing::Expr& parsingExpr)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(&parsingExpr);
    const Identifier iden(varExpr->name);
    auto var = std::make_shared<ValueVar>(iden);
    return var;
}

std::shared_ptr<Value> GenerateIr::generateBinaryInst(const Parsing::Expr& parsingExpr)
{
    const auto binaryParsing = dynamic_cast<const Parsing::BinaryExpr*>(&parsingExpr);
    BinaryInst::Operation operation = convertBinaryOperation(binaryParsing->op);
    if (operation == BinaryInst::Operation::And)
        return generateBinaryAndInst(*binaryParsing);
    if (operation == BinaryInst::Operation::Or)
        return generateBinaryOrInst(*binaryParsing);
    auto source1 = generateInst(*binaryParsing->lhs);
    auto source2 = generateInst(*binaryParsing->rhs);

    auto destination = std::make_shared<ValueVar>(makeTemporaryName());

    m_instructions.push_back(std::make_unique<BinaryInst>(operation, source1, source2, destination));
    return destination;
}

std::shared_ptr<Value> GenerateIr::generateAssignInst(const Parsing::Expr& binaryExpr)
{
    const auto assignExpr = dynamic_cast<const Parsing::AssignmentExpr*>(&binaryExpr);
    if (assignExpr->op != Parsing::AssignmentExpr::Operator::Assign)
        return generateCompoundAssignInst(*assignExpr);
    return generateSimpleAssignInst(*assignExpr);
}

std::shared_ptr<Value> GenerateIr::generateSimpleAssignInst(const Parsing::AssignmentExpr& assignExpr)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(assignExpr.lhs.get());
    const Identifier iden(varExpr->name);
    auto destination = std::make_shared<ValueVar>(iden);
    auto result = generateInst(*assignExpr.rhs);
    m_instructions.push_back(std::make_unique<CopyInst>(result, destination));
    return destination;
}

std::shared_ptr<Value> GenerateIr::generateCompoundAssignInst(const Parsing::AssignmentExpr& assignExpr)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(assignExpr.lhs.get());
    const Identifier iden(varExpr->name);
    auto lhs = std::make_shared<ValueVar>(iden);
    BinaryInst::Operation operation = convertAssiOperation(assignExpr.op);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName());
    auto rhs = generateInst(*assignExpr.rhs);
    m_instructions.push_back(std::make_unique<BinaryInst>(operation, lhs, rhs, destination));
    m_instructions.push_back(std::make_unique<CopyInst>(destination, lhs));
    return lhs;
}

std::shared_ptr<Value> GenerateIr::generateBinaryAndInst(const Parsing::BinaryExpr& parsingExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName());
    auto lhs = generateInst(*parsingExpr.lhs);
    Identifier falseLabelIden = makeTemporaryName();
    m_instructions.push_back(std::make_unique<JumpIfZeroInst>(lhs, falseLabelIden));
    auto rhs = generateInst(*parsingExpr.rhs);
    m_instructions.push_back(std::make_unique<JumpIfZeroInst>(rhs, falseLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    m_instructions.push_back(std::make_unique<CopyInst>(oneVal, result));
    Identifier endLabelIden = makeTemporaryName();
    m_instructions.push_back(std::make_unique<JumpInst>(endLabelIden));
    m_instructions.push_back(std::make_unique<LabelInst>(falseLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    m_instructions.push_back(std::make_unique<CopyInst>(zeroVal, result));
    m_instructions.push_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<Value> GenerateIr::generateBinaryOrInst(const Parsing::BinaryExpr& binaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName());
    auto lhs = generateInst(*binaryExpr.lhs);
    Identifier trueLabelIden = makeTemporaryName();
    m_instructions.push_back(std::make_unique<JumpIfNotZeroInst>(lhs, trueLabelIden));
    auto rhs = generateInst(*binaryExpr.rhs);
    m_instructions.push_back(std::make_unique<JumpIfNotZeroInst>(rhs, trueLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    m_instructions.push_back(std::make_unique<CopyInst>(zeroVal, result));
    Identifier endLabelIden = makeTemporaryName();
    m_instructions.push_back(std::make_unique<JumpInst>(endLabelIden));
    m_instructions.push_back(std::make_unique<LabelInst>(trueLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    m_instructions.push_back(std::make_unique<CopyInst>(oneVal, result));
    m_instructions.push_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<Value> GenerateIr::generateConstInst(const Parsing::Expr& parsingExpr)
{
    const auto constant = dynamic_cast<const Parsing::ConstExpr*>(&parsingExpr);
    auto result = std::make_unique<ValueConst>(constant->value);
    return result;
}

std::shared_ptr<Value> GenerateIr::generateConditionalInst(const Parsing::Expr& stmt)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName());

    Identifier endLabelIden = makeTemporaryName();
    Identifier falseLabelName = makeTemporaryName();

    const auto conditionalExpr = dynamic_cast<const Parsing::ConditionalExpr*>(&stmt);
    auto condition = generateInst(*conditionalExpr->condition);

    m_instructions.push_back(std::make_unique<JumpIfZeroInst>(condition, falseLabelName));

    auto trueValue = generateInst(*conditionalExpr->first);
    m_instructions.push_back(std::make_unique<CopyInst>(trueValue, result));
    m_instructions.push_back(std::make_unique<JumpInst>(endLabelIden));

    m_instructions.push_back(std::make_unique<LabelInst>(falseLabelName));
    auto right = generateInst(*conditionalExpr->second);
    m_instructions.push_back(std::make_unique<CopyInst>(right, result));

    m_instructions.push_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<Value> GenerateIr::generateFuncCallInst(const Parsing::Expr& stmt)
{
    const auto parseFunction = dynamic_cast<const Parsing::FunCallExpr*>(&stmt);
    std::vector<std::shared_ptr<Value>> arguments;
    arguments.reserve(parseFunction->args.size());
    for (const auto& expr : parseFunction->args)
        arguments.push_back(generateInst(*expr));
    auto dst = std::make_shared<ValueVar>(makeTemporaryName());
    m_instructions.push_back(std::make_unique<FunCallInst>(
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

BinaryInst::Operation getPostPrefixOperation(const Parsing::UnaryExpr::Operator oper)
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