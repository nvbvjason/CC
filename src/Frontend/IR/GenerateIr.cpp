#include "ASTParser.hpp"
#include "GenerateIr.hpp"
#include "Types/TypeConversion.hpp"
#include "ASTTypes.hpp"
#include "AstToIrOperators.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace Ir {
static Identifier makeTemporaryName();
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
    const auto entry = m_symbolTable.lookup(varDecl.name);
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
        value = std::make_shared<ValueConst>(static_cast<i64>(0l));
    else if (!entry.isSet(SymbolTable::State::Defined) && varDecl.type->kind == Type::U32)
        value = std::make_shared<ValueConst>(0u);
    else if (!entry.isSet(SymbolTable::State::Defined) && varDecl.type->kind == Type::U64)
        value = std::make_shared<ValueConst>(static_cast<u64>(0ul));
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
    using State = SymbolTable::State;
    bool global = !m_symbolTable.lookup(parsingFunction.name).isSet(State::InternalLinkage);
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
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        const auto decl = static_cast<const Parsing::DeclForInit*>(&forInit);
        genDeclaration(*decl->decl);
        return;
    }
    if (forInit.kind == Parsing::ForInit::Kind::Expression) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        const auto expr = static_cast<const Parsing::ExprForInit*>(&forInit);
        if (expr->expression)
            genInst(*expr->expression);
    }
}

void GenerateIr::genDeclaration(const Parsing::Declaration& decl)
{
    if (decl.kind != Parsing::Declaration::Kind::VarDecl)
        return;
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
    m_symbolTable.addEntry(varDecl.name,
                              varDecl.name,
                              *varDecl.type,
                              true, false, false, defined);
}

void GenerateIr::genStmt(const Parsing::Stmt& stmt)
{
    using Kind = Parsing::Stmt::Kind;
    switch (stmt.kind) {
        case Kind::If: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto ifStmt = static_cast<const Parsing::IfStmt*>(&stmt);
            genIfStmt(*ifStmt);
            break;
        }
        case Kind::Return: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto returnStmt = static_cast<const Parsing::ReturnStmt*>(&stmt);
            genReturnStmt(*returnStmt);
            break;
        }
        case Kind::Expression: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto stmtExpr = static_cast<const Parsing::ExprStmt*>(&stmt);
            genInst(*stmtExpr->expr);
            break;
        }
        case Kind::Goto: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto gotoStmt = static_cast<const Parsing::GotoStmt*>(&stmt);
            genGotoStmt(*gotoStmt);
            break;
        }
        case Kind::Compound: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto compoundStmtPtr = static_cast<const Parsing::CompoundStmt*>(&stmt);
            genCompoundStmt(*compoundStmtPtr);
            break;
        }
        case Kind::Break: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto breakStmtPtr = static_cast<const Parsing::BreakStmt*>(&stmt);
            genBreakStmt(*breakStmtPtr);
            break;
        }
        case Kind::Continue: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto continueStmtPtr = static_cast<const Parsing::ContinueStmt*>(&stmt);
            genContinueStmt(*continueStmtPtr);
            break;
        }
        case Kind::Label: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto labelStmtPtr = static_cast<const Parsing::LabelStmt*>(&stmt);
            genLabelStmt(*labelStmtPtr);
            break;
        }
        case Kind::Case: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto caseStmtPtr = static_cast<const Parsing::CaseStmt*>(&stmt);
            genCaseStmt(*caseStmtPtr);
            break;
        }
        case Kind::Default: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto defaultStmtPtr = static_cast<const Parsing::DefaultStmt*>(&stmt);
            genDefaultStmt(*defaultStmtPtr);
            break;
        }
        case Kind::DoWhile: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto doWhileStmtPtr = static_cast<const Parsing::DoWhileStmt*>(&stmt);
            genDoWhileStmt(*doWhileStmtPtr);
            break;
        }
        case Kind::While: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto whileStmtPtr = static_cast<const Parsing::WhileStmt*>(&stmt);
            genWhileStmt(*whileStmtPtr);
            break;
        }
        case Kind::For: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto forStmtPtr = static_cast<const Parsing::ForStmt*>(&stmt);
            genForStmt(*forStmtPtr);
            break;
        }
        case Kind::Switch: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto switchStmtPtr = static_cast<const Parsing::SwitchStmt*>(&stmt);
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
        case ExprKind::Cast: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto castExpr = static_cast<const Parsing::CastExpr*>(&parsingExpr);
            return genCastInst(*castExpr);
        }
        case ExprKind::Var: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto varExpr = static_cast<const Parsing::VarExpr*>(&parsingExpr);
            return genVarInst(*varExpr);
        }
        case ExprKind::Constant: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto constExpr = static_cast<const Parsing::ConstExpr*>(&parsingExpr);
            return genConstInst(*constExpr);
        }
        case ExprKind::Unary: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto unaryParsingPtr = static_cast<const Parsing::UnaryExpr*>(&parsingExpr);
            return genUnaryInst(*unaryParsingPtr);
        }
        case ExprKind::Binary: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto binaryParsingPtr = static_cast<const Parsing::BinaryExpr*>(&parsingExpr);
            return genBinaryInst(*binaryParsingPtr);
        }
        case ExprKind::Assignment: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto assignmentExpr = static_cast<const Parsing::AssignmentExpr*>(&parsingExpr);
            return genAssignInst(*assignmentExpr);
        }
        case ExprKind::Ternary: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto ternaryExpr = static_cast<const Parsing::TernaryExpr*>(&parsingExpr);
            return genTernaryInst(*ternaryExpr);
        }
        case ExprKind::FunctionCall: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto funcCallExpr = static_cast<const Parsing::FunCallExpr*>(&parsingExpr);
            return genFuncCallInst(*funcCallExpr);
        }
        case ExprKind::Dereference: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto dereferenceExpr = static_cast<const Parsing::DereferenceExpr*>(&parsingExpr);
            return genDereferenceInst(*dereferenceExpr);
        }
        case ExprKind::AddrOf: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto addrOfExpr = static_cast<const Parsing::AddrOffExpr*>(&parsingExpr);
            return genAddrOfInst(*addrOfExpr);
        }
        default:
            assert("Unexpected expression type ir generateInst");
    }
    std::unreachable();
}

std::shared_ptr<Value> GenerateIr::genCastInst(const Parsing::CastExpr& castExpr)
{
    auto result = genInst(*castExpr.expr);
    const Type type = castExpr.type->kind;
    const Type innerType = castExpr.expr->type->kind;
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

std::shared_ptr<Value> GenerateIr::genUnaryInst(const Parsing::UnaryExpr& unaryExpr)
{
    if (isPostfixOp(unaryExpr.op))
        return genUnaryPostfixInst(unaryExpr);
    if (isPrefixOp(unaryExpr.op))
        return genUnaryPrefixInst(unaryExpr);
    if (unaryExpr.op == Parsing::UnaryExpr::Operator::Plus)
        return genInst(*unaryExpr.operand);
    return genUnaryBasicInst(unaryExpr);
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

std::shared_ptr<Value> GenerateIr::genVarInst(const Parsing::VarExpr& varExpr)
{
    const Identifier iden(varExpr.name);
    auto var = std::make_shared<ValueVar>(iden, varExpr.type->kind);
    var->referingTo = varExpr.referingTo;
    return var;
}

std::shared_ptr<Value> GenerateIr::genBinaryInst(const Parsing::BinaryExpr& binaryExpr)
{
    BinaryInst::Operation operation = convertBinaryOperation(binaryExpr.op);
    if (operation == BinaryInst::Operation::And)
        return genBinaryAndInst(binaryExpr);
    if (operation == BinaryInst::Operation::Or)
        return genBinaryOrInst(binaryExpr);
    std::shared_ptr<Value> source1 = genInst(*binaryExpr.lhs);
    std::shared_ptr<Value> source2 = genInst(*binaryExpr.rhs);

    auto destination = std::make_shared<ValueVar>(makeTemporaryName(), binaryExpr.type->kind);

    m_insts.emplace_back(std::make_unique<BinaryInst>(
        operation, source1, source2, destination, binaryExpr.type->kind));
    return destination;
}

std::shared_ptr<Value> GenerateIr::genAssignInst(const Parsing::AssignmentExpr& assignmentExpr)
{
    const auto varExpr = dynamic_cast<const Parsing::VarExpr*>(assignmentExpr.lhs.get());
    const Identifier iden(varExpr->name);
    auto destination = std::make_shared<ValueVar>(iden, assignmentExpr.type->kind);
    destination->referingTo = varExpr->referingTo;
    auto result = genInst(*assignmentExpr.rhs);
    m_insts.emplace_back(std::make_unique<CopyInst>(
        result, destination, assignmentExpr.type->kind));
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

std::shared_ptr<Value> GenerateIr::genConstInst(const Parsing::ConstExpr& constExpr)
{
    if (constExpr.type->kind == Type::I32)
        return std::make_unique<ValueConst>(std::get<i32>(constExpr.value));
    if (constExpr.type->kind == Type::U32)
        return std::make_unique<ValueConst>(std::get<u32>(constExpr.value));
    if (constExpr.type->kind == Type::U64)
        return std::make_unique<ValueConst>(std::get<u64>(constExpr.value));
    if (constExpr.type->kind == Type::I64)
        return std::make_unique<ValueConst>(std::get<i64>(constExpr.value));
    return std::make_unique<ValueConst>(std::get<double>(constExpr.value));
}

std::shared_ptr<Value> GenerateIr::genTernaryInst(const Parsing::TernaryExpr& ternaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), ternaryExpr.type->kind);

    Identifier endLabelIden = makeTemporaryName();
    Identifier falseLabelName = makeTemporaryName();

    const auto conditionalExpr = dynamic_cast<const Parsing::TernaryExpr*>(&ternaryExpr);
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

std::shared_ptr<Value> GenerateIr::genFuncCallInst(const Parsing::FunCallExpr& funcCallExpr)
{
    std::vector<std::shared_ptr<Value>> arguments;
    arguments.reserve(funcCallExpr.args.size());
    for (const auto& expr : funcCallExpr.args)
        arguments.emplace_back(genInst(*expr));
    const auto funcType = static_cast<const Parsing::FuncType*>(funcCallExpr.type.get());
    auto dst = std::make_shared<ValueVar>(makeTemporaryName(), funcCallExpr.type->kind);
    m_insts.emplace_back(std::make_unique<FunCallInst>(
        Identifier(funcCallExpr.name), std::move(arguments), dst, funcType->kind));
    return dst;
}

std::shared_ptr<Value> GenerateIr::genAddrOfInst(const Parsing::AddrOffExpr& addrOffExpr)
{
    if (addrOffExpr.reference->kind == Parsing::Expr::Kind::Dereference) {
        const auto deferenceExpr = static_cast<const Parsing::DereferenceExpr*>(addrOffExpr.reference.get());
        return genInst(*deferenceExpr->reference);
    }
}

std::shared_ptr<Value> GenerateIr::genDereferenceInst(const Parsing::DereferenceExpr& dereferenceExpr)
{

}

Identifier makeTemporaryName()
{
    static i32 id = 0;
    std::string prefix = "tmp.";
    prefix += std::to_string(id++);
    return {prefix};
}

static std::string generateCaseLabelName(std::string before)
{
    std::ranges::replace(before, '-', '_');
    return before;
}
} // IR