#include "ASTParser.hpp"
#include "GenerateIr.hpp"
#include "Types/TypeConversion.hpp"
#include "ASTTypes.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace Ir {
static Identifier makeTemporaryName();
static bool isPostfixOp(Parsing::UnaryExpr::Operator oper);
static bool isPrefixOp(Parsing::UnaryExpr::Operator oper);
static BinaryInst::Operation getPostPrefixOperation(Parsing::UnaryExpr::Operator oper);
static UnaryInst::Operation convertUnaryOperation(Parsing::UnaryExpr::Operator unaryOperation);
static BinaryInst::Operation convertBinaryOperation(Parsing::BinaryExpr::Operator binaryOperation);
static std::string generateCaseLabelName(std::string before);

void GenerateIr::program(const Parsing::Program& parsingProgram, Program& tackyProgram)
{
    for (const std::unique_ptr<Parsing::Declaration>& decl : parsingProgram.declarations) {
        std::unique_ptr<TopLevel> topLevel = topLevelIr(*decl);
        if (topLevel == nullptr)
            continue;
        m_topLevels.emplace_back(std::move(topLevel));
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
    if (!entry.isSet(SymbolTable::State::Defined) && varDecl.type->kind == Type::I32)
        value = std::make_shared<ValueConst>(0);
    else if (!entry.isSet(SymbolTable::State::Defined) && varDecl.type->kind == Type::I64)
        value = std::make_shared<ValueConst>(0l);
    else if (!entry.isSet(SymbolTable::State::Defined) && varDecl.type->kind == Type::U32)
        value = std::make_shared<ValueConst>(0u);
    else if (!entry.isSet(SymbolTable::State::Defined) && varDecl.type->kind == Type::U64)
        value = std::make_shared<ValueConst>(0ul);
    else if (!entry.isSet(SymbolTable::State::Defined) && varDecl.type->kind == Type::Double)
        value = std::make_shared<ValueConst>(0.0);
    else
        value = genInst(*varDecl.init);
    auto variable = std::make_unique<StaticVariable>(
        varDecl.name, value, varDecl.type->kind, varDecl.storage != Storage::Static);
    return variable;
}

std::unique_ptr<TopLevel> GenerateIr::functionIr(const Parsing::FunDecl& parsingFunction)
{
    bool global = !m_symbolTable.lookupFunc(parsingFunction.name).isSet(SymbolTable::State::InternalLinkage);
    auto functionTacky = std::make_unique<Function>(parsingFunction.name, global);
    m_global = true;;
    m_insts = std::move(functionTacky->insts);
    functionTacky->args.reserve(parsingFunction.params.size());
    functionTacky->argTypes.reserve(parsingFunction.params.size());
    auto funcType = static_cast<const Parsing::FuncType*>(parsingFunction.type.get());
    for (i32 i = 0; i < parsingFunction.params.size(); ++i) {
        functionTacky->args.emplace_back(Identifier(parsingFunction.params[i]));
        functionTacky->argTypes.emplace_back(funcType->params[i]->kind);
    }
    genBlock(*parsingFunction.body);
    functionTacky->insts = std::move(m_insts);
    m_global = false;
    return functionTacky;
}

void GenerateIr::genBlock(const Parsing::Block& block)
{
    for (const std::unique_ptr<Parsing::BlockItem>& item : block.body)
        genBlockItem(*item);
}

void GenerateIr::genBlockItem(const Parsing::BlockItem& blockItem)
{
    using Kind = Parsing::BlockItem::Kind;
    switch (blockItem.kind) {
        case Kind::Declaration: {
            const auto decl = dynamic_cast<const Parsing::DeclBlockItem*>(&blockItem);
            genDeclaration(*decl->decl);
            break;
        }
        case Kind::Statement: {
            const auto stmtBlockItem = dynamic_cast<const Parsing::StmtBlockItem*>(&blockItem);
            genStmt(*stmtBlockItem->stmt);
            break;
        }
        default:
            throw std::invalid_argument("Unexpected block item type ir generateBlockItem");
    }
}

void GenerateIr::genForInit(const Parsing::ForInit& forInit)
{
    if (forInit.kind == Parsing::ForInit::Kind::Declaration) {
        const auto decl = dynamic_cast<const Parsing::DeclForInit*>(&forInit);
        genDeclaration(*decl->decl);
        return;
    }
    if (forInit.kind == Parsing::ForInit::Kind::Expression) {
        const auto expr = dynamic_cast<const Parsing::ExprForInit*>(&forInit);
        if (expr->expression)
            genInst(*expr->expression);
    }
}

void GenerateIr::genDeclaration(const Parsing::Declaration& decl)
{
    if (decl.kind == Parsing::Declaration::Kind::VarDecl) {
        const auto varDecl = dynamic_cast<const Parsing::VarDecl*>(&decl);
        if (varDecl->storage == Storage::Static)
            return genStaticLocal(*varDecl);
        if (varDecl->init == nullptr)
            return;
        std::shared_ptr<Value> value = genInst(*varDecl->init);
        auto temporary = std::make_shared<ValueVar>(makeTemporaryName(), varDecl->type->kind);
        m_insts.push_back(std::make_unique<CopyInst>(value, temporary, varDecl->type->kind));
        const Identifier iden(varDecl->name);
        auto var = std::make_shared<ValueVar>(iden, varDecl->type->kind);
        m_insts.push_back(std::make_unique<CopyInst>(temporary, var, varDecl->type->kind));
    }
}

void GenerateIr::genStaticLocal(const Parsing::VarDecl& varDecl)
{
    std::shared_ptr<Value> value;
    const bool defined = varDecl.init != nullptr;
    if (!defined)
        value = std::make_shared<ValueConst>(0);
    else
        value = genInst(*varDecl.init);
    auto variable = std::make_unique<StaticVariable>(
        varDecl.name, value, varDecl.type->kind, false);
    m_topLevels.push_back(std::move(variable));
    m_symbolTable.addVarEntry(varDecl.name,
                              varDecl.name,
                              varDecl.type->kind,
                              true, false, false, defined);
}

void GenerateIr::genStmt(const Parsing::Stmt& stmt)
{
    using Kind = Parsing::Stmt::Kind;
    switch (stmt.kind) {
        case Kind::If: {
            const auto ifStmt = dynamic_cast<const Parsing::IfStmt*>(&stmt);
            genIfStmt(*ifStmt);
            break;
        }
        case Kind::Return: {
            const auto returnStmt = dynamic_cast<const Parsing::ReturnStmt*>(&stmt);
            genReturnStmt(*returnStmt);
            break;
        }
        case Kind::Expression: {
            const auto stmtExpr = dynamic_cast<const Parsing::ExprStmt*>(&stmt);
            genInst(*stmtExpr->expr);
            break;
        }
        case Kind::Goto: {
            const auto gotoStmt = dynamic_cast<const Parsing::GotoStmt*>(&stmt);
            genGotoStmt(*gotoStmt);
            break;
        }
        case Kind::Compound: {
            const auto compoundStmtPtr = dynamic_cast<const Parsing::CompoundStmt*>(&stmt);
            genCompoundStmt(*compoundStmtPtr);
            break;
        }
        case Kind::Break: {
            const auto breakStmtPtr = dynamic_cast<const Parsing::BreakStmt*>(&stmt);
            genBreakStmt(*breakStmtPtr);
            break;
        }
        case Kind::Continue: {
            const auto continueStmtPtr = dynamic_cast<const Parsing::ContinueStmt*>(&stmt);
            genContinueStmt(*continueStmtPtr);
            break;
        }
        case Kind::Label: {
            const auto labelStmtPtr = dynamic_cast<const Parsing::LabelStmt*>(&stmt);
            genLabelStmt(*labelStmtPtr);
            break;
        }
        case Kind::Case: {
            const auto caseStmtPtr = dynamic_cast<const Parsing::CaseStmt*>(&stmt);
            genCaseStmt(*caseStmtPtr);
            break;
        }
        case Kind::Default: {
            const auto defaultStmtPtr = dynamic_cast<const Parsing::DefaultStmt*>(&stmt);
            genDefaultStmt(*defaultStmtPtr);
            break;
        }
        case Kind::DoWhile: {
            const auto doWhileStmtPtr = dynamic_cast<const Parsing::DoWhileStmt*>(&stmt);
            genDoWhileStmt(*doWhileStmtPtr);
            break;
        }
        case Kind::While: {
            const auto whileStmtPtr = dynamic_cast<const Parsing::WhileStmt*>(&stmt);
            genWhileStmt(*whileStmtPtr);
            break;
        }
        case Kind::For: {
            const auto forStmtPtr = dynamic_cast<const Parsing::ForStmt*>(&stmt);
            genForStmt(*forStmtPtr);
            break;
        }
        case Kind::Switch: {
            const auto switchStmtPtr = dynamic_cast<const Parsing::SwitchStmt*>(&stmt);
            genSwitchStmt(*switchStmtPtr);
            break;
        }
        case Kind::Null:
            break;
        default:
            throw std::invalid_argument("Unexpected statement type Ir generate");
    }
}

void GenerateIr::genIfStmt(const Parsing::IfStmt& ifStmt)
{
    if (ifStmt.elseStmt == nullptr)
        genIfBasicStmt(ifStmt);
    else
        genIfElseStmt(ifStmt);
}

void GenerateIr::genIfBasicStmt(const Parsing::IfStmt& ifStmt)
{
    auto condition = genInst(*ifStmt.condition);
    Identifier endLabelIden = makeTemporaryName();
    m_insts.emplace_back(std::make_unique<JumpIfZeroInst>(condition, endLabelIden));
    genStmt(*ifStmt.thenStmt);
    m_insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
}

void GenerateIr::genIfElseStmt(const Parsing::IfStmt& ifStmt)
{
    auto condition = genInst(*ifStmt.condition);
    Identifier elseStmtLabel = makeTemporaryName();
    m_insts.emplace_back(std::make_unique<JumpIfZeroInst>(condition, elseStmtLabel));
    genStmt(*ifStmt.thenStmt);
    Identifier endLabelIden = makeTemporaryName();
    m_insts.emplace_back(std::make_unique<JumpInst>(endLabelIden));
    m_insts.emplace_back(std::make_unique<LabelInst>(elseStmtLabel));
    genStmt(*ifStmt.elseStmt);
    m_insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
}

void GenerateIr::genReturnStmt(const Parsing::ReturnStmt& returnStmt)
{
    auto value = genInst(*returnStmt.expr);
    if (value == nullptr)
        return;
    m_insts.push_back(std::make_unique<ReturnInst>(value, value->type));
}

void GenerateIr::genGotoStmt(const Parsing::GotoStmt& gotoStmt)
{
    m_insts.emplace_back(std::make_unique<JumpInst>(Identifier(gotoStmt.identifier + ".label")));
}

void GenerateIr::genCompoundStmt(const Parsing::CompoundStmt& compoundStmt)
{
    genBlock(*compoundStmt.block);
}

void GenerateIr::genBreakStmt(const Parsing::BreakStmt& breakStmt)
{
    m_insts.emplace_back(std::make_unique<JumpInst>(Identifier(breakStmt.identifier + "break")));
}

void GenerateIr::genContinueStmt(const Parsing::ContinueStmt& continueStmt)
{
    m_insts.emplace_back(std::make_unique<JumpInst>(Identifier(continueStmt.identifier + "continue")));
}

void GenerateIr::genLabelStmt(const Parsing::LabelStmt& labelStmt)
{
    m_insts.emplace_back(std::make_unique<LabelInst>(Identifier(labelStmt.identifier + ".label")));
    genStmt(*labelStmt.stmt);
}

void GenerateIr::genCaseStmt(const Parsing::CaseStmt& caseStmt)
{
    m_insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(generateCaseLabelName(caseStmt.identifier))));
    genStmt(*caseStmt.body);
}

void GenerateIr::genDefaultStmt(const Parsing::DefaultStmt& defaultStmt)
{
    m_insts.emplace_back(std::make_unique<LabelInst>(Identifier(defaultStmt.identifier + "default")));
    genStmt(*defaultStmt.body);
}

void GenerateIr::genDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt)
{
    m_insts.emplace_back(std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "start")));
    genStmt(*doWhileStmt.body);
    m_insts.emplace_back(std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "continue")));
    auto condition = genInst(*doWhileStmt.condition);
    m_insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(
        condition, Identifier(doWhileStmt.identifier + "start")));
    m_insts.emplace_back(std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "break")));
}

void GenerateIr::genWhileStmt(const Parsing::WhileStmt& whileStmt)
{
    m_insts.emplace_back(std::make_unique<LabelInst>(Identifier(whileStmt.identifier + "continue")));
    auto condition = genInst(*whileStmt.condition);
    m_insts.emplace_back(std::make_unique<JumpIfZeroInst>(
        condition, Identifier(whileStmt.identifier + "break")));
    genStmt(*whileStmt.body);
    m_insts.emplace_back(std::make_unique<JumpInst>(Identifier(whileStmt.identifier + "continue")));
    m_insts.emplace_back(std::make_unique<LabelInst>(Identifier(whileStmt.identifier + "break")));
}

void GenerateIr::genForStmt(const Parsing::ForStmt& forStmt)
{
    if (forStmt.init)
        genForInit(*forStmt.init);
    m_insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(forStmt.identifier + "start"))
    );
    if (forStmt.condition) {
        auto condition = genInst(*forStmt.condition);
        m_insts.emplace_back(std::make_unique<JumpIfZeroInst>(
            condition, Identifier(forStmt.identifier + "break"))
        );
    }
    genStmt(*forStmt.body);
    m_insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(forStmt.identifier + "continue"))
    );
    if (forStmt.post)
        genInst(*forStmt.post);
    m_insts.emplace_back(std::make_unique<JumpInst>(
        Identifier(forStmt.identifier + "start"))
    );
    m_insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(forStmt.identifier + "break"))
    );
}

void GenerateIr::genSwitchStmt(const Parsing::SwitchStmt& stmt)
{
    auto realValue = genInst(*stmt.condition);
    const Type conditionType = stmt.condition->type->kind;
    for (const std::variant<i32, i64, u32, u64>& caseValue : stmt.cases) {
        const auto destination = std::make_shared<ValueVar>(makeTemporaryName(), conditionType);
        std::string caseLabelName;
        std::shared_ptr<ValueConst> src2;
        if (conditionType == Type::I32) {
            const i32 value = std::get<i32>(caseValue);
            caseLabelName = generateCaseLabelName(stmt.identifier + std::to_string(value));
            src2 = std::make_shared<ValueConst>(value);
        }
        if (conditionType == Type::I64) {
            const i64 value = std::get<i64>(caseValue);
            caseLabelName = generateCaseLabelName(stmt.identifier + std::to_string(value));
            src2 = std::make_shared<ValueConst>(value);
        }
        if (conditionType == Type::U32) {
            const u32 value = std::get<u32>(caseValue);
            caseLabelName = generateCaseLabelName(stmt.identifier + std::to_string(value));
            src2 = std::make_shared<ValueConst>(value);
        }
        if (conditionType == Type::U64) {
            const u64 value = std::get<u32>(caseValue);
            caseLabelName = generateCaseLabelName(stmt.identifier + std::to_string(value));
            src2 = std::make_shared<ValueConst>(value);
        }
        m_insts.emplace_back(std::make_unique<BinaryInst>(
            BinaryInst::Operation::Equal, realValue, src2,
            destination, realValue->type));
        m_insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(
            destination, Identifier(caseLabelName)));
    }
    if (stmt.hasDefault)
        m_insts.emplace_back(std::make_unique<JumpInst>(
            Identifier(stmt.identifier + "default")));
    else
        m_insts.emplace_back(std::make_unique<JumpInst>(Identifier(
                stmt.identifier + "break")));
    genStmt(*stmt.body);
    m_insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(stmt.identifier + "break")));
}

std::shared_ptr<Value> GenerateIr::genInst(const Parsing::Expr& parsingExpr)
{
    using ExprKind = Parsing::Expr::Kind;
    switch (parsingExpr.kind) {
        case ExprKind::Cast:
            return genCastInst(parsingExpr);
        case ExprKind::Var:
            return genVarInst(parsingExpr);
        case ExprKind::Constant:
            return genConstInst(parsingExpr);
        case ExprKind::Unary:
            return genUnaryInst(parsingExpr);
        case ExprKind::Binary:
            return genBinaryInst(parsingExpr);
        case ExprKind::Assignment:
            return genAssignInst(parsingExpr);
        case ExprKind::Conditional:
            return genTernaryInst(parsingExpr);
        case ExprKind::FunctionCall:
            return genFuncCallInst(parsingExpr);
        default:
            assert("Unexpected expression type ir generateInst");
    }
    std::unreachable();
}

std::shared_ptr<Value> GenerateIr::genCastInst(const Parsing::Expr& parsingExpr)
{
    const auto castExpr = static_cast<const Parsing::CastExpr*>(&parsingExpr);
    auto result = genInst(*castExpr->expr);
    const Type type = parsingExpr.type->kind;
    const Type innerType = castExpr->expr->type->kind;
    const Identifier iden(makeTemporaryName());
    auto dst = std::make_shared<ValueVar>(iden, type);
    if (type == Type::Double && !isSigned(innerType))
        m_insts.emplace_back(std::make_unique<UIntToDoubleInst>(result, dst, type));
    else if (type == Type::Double && isSigned(innerType))
        m_insts.emplace_back(std::make_unique<IntToDoubleInst>(result, dst, type));
    else if (!isSigned(type) && innerType == Type::Double)
        m_insts.emplace_back(std::make_unique<DoubleToUIntInst>(result, dst, type));
    else if (isSigned(type) && innerType == Type::Double)
        m_insts.emplace_back(std::make_unique<DoubleToIntInst>(result, dst, type));
    else if (getSize(type) == getSize(innerType))
        m_insts.emplace_back(std::make_unique<CopyInst>(result, dst, type));
    else if (getSize(type) < getSize(innerType))
        m_insts.emplace_back(std::make_unique<TruncateInst>(result, dst, innerType));
    else if (isSigned(innerType))
        m_insts.emplace_back(std::make_unique<SignExtendInst>(result, dst, type));
    else
        m_insts.emplace_back(std::make_unique<ZeroExtendInst>(result, dst, type));
    return dst;
}

std::shared_ptr<Value> GenerateIr::genUnaryInst(const Parsing::Expr& parsingExpr)
{
    const auto unaryParsingPtr = dynamic_cast<const Parsing::UnaryExpr*>(&parsingExpr);
    if (isPostfixOp(unaryParsingPtr->op))
        return genUnaryPostfixInst(*unaryParsingPtr);
    if (isPrefixOp(unaryParsingPtr->op))
        return genUnaryPrefixInst(*unaryParsingPtr);
    if (unaryParsingPtr->op == Parsing::UnaryExpr::Operator::Plus)
        return genInst(*unaryParsingPtr->operand);
    return genUnaryBasicInst(*unaryParsingPtr);
}

std::shared_ptr<Value> GenerateIr::genUnaryBasicInst(const Parsing::UnaryExpr& unaryExpr)
{
    UnaryInst::Operation operation = convertUnaryOperation(unaryExpr.op);
    auto source = genInst(*unaryExpr.operand);
    auto destination = std::make_shared<ValueVar>(makeTemporaryName(), unaryExpr.type->kind);
    m_insts.emplace_back(std::make_unique<UnaryInst>(
        operation, source, destination, unaryExpr.type->kind));
    return destination;
}

std::shared_ptr<Value> GenerateIr::genUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr)
{
    auto originalForReturn = std::make_shared<ValueVar>(makeTemporaryName(), unaryExpr.type->kind);
    auto tempNew = std::make_shared<ValueVar>(makeTemporaryName(), unaryExpr.type->kind);
    auto original = genInst(*unaryExpr.operand);
    m_insts.emplace_back(std::make_unique<CopyInst>(original, originalForReturn, unaryExpr.type->kind));
    std::shared_ptr<ValueConst> one;
    if (unaryExpr.type->kind == Type::Double)
        one = std::make_shared<ValueConst>(1.0);
    else
        one = std::make_shared<ValueConst>(1);
    m_insts.emplace_back(std::make_unique<BinaryInst>(
        getPostPrefixOperation(unaryExpr.op), originalForReturn, one,
        tempNew, unaryExpr.type->kind));
    m_insts.emplace_back(std::make_unique<CopyInst>(tempNew, original, unaryExpr.type->kind));
    return originalForReturn;
}

std::shared_ptr<Value> GenerateIr::genUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr)
{
    auto original = genInst(*unaryExpr.operand);
    std::shared_ptr<ValueConst> one;
    if (unaryExpr.type->kind == Type::Double)
        one = std::make_shared<ValueConst>(1.0);
    else
        one = std::make_shared<ValueConst>(1);
    auto temp = std::make_shared<ValueVar>(Identifier(makeTemporaryName()), unaryExpr.type->kind);
    m_insts.emplace_back(std::make_unique<BinaryInst>(
        getPostPrefixOperation(unaryExpr.op), original, one, temp, unaryExpr.type->kind)
    );
    m_insts.emplace_back(std::make_unique<CopyInst>(temp, original, unaryExpr.type->kind));
    return temp;
}

std::shared_ptr<Value> GenerateIr::genVarInst(const Parsing::Expr& parsingExpr)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(&parsingExpr);
    const Identifier iden(varExpr->name);
    auto var = std::make_shared<ValueVar>(iden, parsingExpr.type->kind);
    var->referingTo = varExpr->referingTo;
    return var;
}

std::shared_ptr<Value> GenerateIr::genBinaryInst(const Parsing::Expr& parsingExpr)
{
    const auto binaryParsing = dynamic_cast<const Parsing::BinaryExpr*>(&parsingExpr);
    BinaryInst::Operation operation = convertBinaryOperation(binaryParsing->op);
    if (operation == BinaryInst::Operation::And)
        return genBinaryAndInst(*binaryParsing);
    if (operation == BinaryInst::Operation::Or)
        return genBinaryOrInst(*binaryParsing);
    auto source1 = genInst(*binaryParsing->lhs);
    auto source2 = genInst(*binaryParsing->rhs);

    auto destination = std::make_shared<ValueVar>(makeTemporaryName(), parsingExpr.type->kind);

    m_insts.emplace_back(std::make_unique<BinaryInst>(
        operation, source1, source2, destination, binaryParsing->type->kind));
    return destination;
}

std::shared_ptr<Value> GenerateIr::genAssignInst(const Parsing::Expr& binaryExpr)
{
    const auto assignExpr = dynamic_cast<const Parsing::AssignmentExpr*>(&binaryExpr);
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(assignExpr->lhs.get());
    const Identifier iden(varExpr->name);
    auto destination = std::make_shared<ValueVar>(iden, assignExpr->type->kind);
    destination->referingTo = varExpr->referingTo;
    auto result = genInst(*assignExpr->rhs);
    m_insts.emplace_back(std::make_unique<CopyInst>(
        result, destination, assignExpr->type->kind));
    return destination;
}

std::shared_ptr<Value> GenerateIr::genBinaryAndInst(const Parsing::BinaryExpr& binaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), binaryExpr.type->kind);
    auto lhs = genInst(*binaryExpr.lhs);
    Identifier falseLabelIden = makeTemporaryName();
    m_insts.emplace_back(std::make_unique<JumpIfZeroInst>(lhs, falseLabelIden));
    auto rhs = genInst(*binaryExpr.rhs);
    m_insts.emplace_back(std::make_unique<JumpIfZeroInst>(rhs, falseLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    m_insts.emplace_back(std::make_unique<CopyInst>(oneVal, result, binaryExpr.type->kind));
    Identifier endLabelIden = makeTemporaryName();
    m_insts.emplace_back(std::make_unique<JumpInst>(endLabelIden));
    m_insts.emplace_back(std::make_unique<LabelInst>(falseLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    m_insts.emplace_back(std::make_unique<CopyInst>(zeroVal, result, binaryExpr.type->kind));
    m_insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<Value> GenerateIr::genBinaryOrInst(const Parsing::BinaryExpr& binaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), binaryExpr.type->kind);
    auto lhs = genInst(*binaryExpr.lhs);
    Identifier trueLabelIden = makeTemporaryName();
    m_insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(lhs, trueLabelIden));
    auto rhs = genInst(*binaryExpr.rhs);
    m_insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(rhs, trueLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    m_insts.emplace_back(std::make_unique<CopyInst>(zeroVal, result, binaryExpr.type->kind));
    Identifier endLabelIden = makeTemporaryName();
    m_insts.emplace_back(std::make_unique<JumpInst>(endLabelIden));
    m_insts.emplace_back(std::make_unique<LabelInst>(trueLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    m_insts.emplace_back(std::make_unique<CopyInst>(oneVal, result, binaryExpr.type->kind));
    m_insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<Value> GenerateIr::genConstInst(const Parsing::Expr& parsingExpr)
{
    const auto constant = dynamic_cast<const Parsing::ConstExpr*>(&parsingExpr);
    if (constant->type->kind == Type::I32)
        return std::make_unique<ValueConst>(std::get<i32>(constant->value));
    if (constant->type->kind == Type::U32)
        return std::make_unique<ValueConst>(std::get<u32>(constant->value));
    if (constant->type->kind == Type::U64)
        return std::make_unique<ValueConst>(std::get<u64>(constant->value));
    if (constant->type->kind == Type::I64)
        return std::make_unique<ValueConst>(std::get<i64>(constant->value));
    return std::make_unique<ValueConst>(std::get<double>(constant->value));
}

std::shared_ptr<Value> GenerateIr::genTernaryInst(const Parsing::Expr& ternary)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), ternary.type->kind);

    Identifier endLabelIden = makeTemporaryName();
    Identifier falseLabelName = makeTemporaryName();

    const auto conditionalExpr = dynamic_cast<const Parsing::TernaryExpr*>(&ternary);
    auto condition = genInst(*conditionalExpr->condition);

    m_insts.emplace_back(std::make_unique<JumpIfZeroInst>(condition, falseLabelName));

    auto trueValue = genInst(*conditionalExpr->trueExpr);
    m_insts.emplace_back(std::make_unique<CopyInst>(trueValue, result, trueValue->type));
    m_insts.emplace_back(std::make_unique<JumpInst>(endLabelIden));

    m_insts.emplace_back(std::make_unique<LabelInst>(falseLabelName));
    auto falseValue = genInst(*conditionalExpr->falseExpr);
    m_insts.emplace_back(std::make_unique<CopyInst>(falseValue, result, falseValue->type));

    m_insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
    return result;
}

std::shared_ptr<Value> GenerateIr::genFuncCallInst(const Parsing::Expr& parsingExpr)
{
    const auto parseFunction = dynamic_cast<const Parsing::FunCallExpr*>(&parsingExpr);
    std::vector<std::shared_ptr<Value>> arguments;
    arguments.reserve(parseFunction->args.size());
    for (const auto& expr : parseFunction->args)
        arguments.emplace_back(genInst(*expr));
    const auto funcType = static_cast<const Parsing::FuncType*>(parseFunction->type.get());
    auto dst = std::make_shared<ValueVar>(makeTemporaryName(), parsingExpr.type->kind);
    m_insts.emplace_back(std::make_unique<FunCallInst>(
        Identifier(parseFunction->name), std::move(arguments), dst, funcType->kind));
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
        case Parse::Modulo:         return Ir::Remainder;

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

static std::string generateCaseLabelName(std::string before)
{
    std::ranges::replace(before, '-', '_');
    return before;
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