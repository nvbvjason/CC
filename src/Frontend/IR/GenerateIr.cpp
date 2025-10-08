#include "ASTParser.hpp"
#include "GenerateIr.hpp"
#include "Types/TypeConversion.hpp"
#include "ASTTypes.hpp"
#include "AstToIrOperators.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <sys/stat.h>

#include "DynCast.hpp"

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
            const auto funcDecl = dynCast<const Parsing::FunDecl>(&decl);
            if (funcDecl->body == nullptr)
                return nullptr;
            return functionIr(*funcDecl);
        }
        case Kind::VarDecl: {
            const auto varDecl = dynCast<const Parsing::VarDecl>(&decl);
            return staticVariableIr(*varDecl);
        }
    }
    std::unreachable();
}

std::unique_ptr<TopLevel> GenerateIr::staticVariableIr(const Parsing::VarDecl& varDecl)
{
    const auto entry = m_symbolTable.lookup(varDecl.name);
    const bool defined = entry.isSet(SymbolTable::State::Defined);
    if (defined && varDecl.init == nullptr)
        return nullptr;
    if (!defined && varDecl.storage == Storage::Extern)
        return nullptr;
    if (m_writtenGlobals.contains(varDecl.name))
        return nullptr;
    m_writtenGlobals.insert(varDecl.name);
    std::shared_ptr<Value> value = genStaticVariableInit(varDecl, defined);
    auto variable = std::make_unique<StaticVariable>(
        varDecl.name, value, varDecl.type->type, varDecl.storage != Storage::Static);
    return variable;
}

std::shared_ptr<Value> GenerateIr::genStaticVariableInit(const Parsing::VarDecl& varDecl, const bool defined)
{
    if (defined)
        return genInstAndConvert(*varDecl.init);
    switch (varDecl.type->type) {
        case Type::I32:
            return std::make_shared<ValueConst>(0);
        case Type::I64:
            return std::make_shared<ValueConst>(static_cast<i64>(0l));
        case Type::U32:
            return std::make_shared<ValueConst>(0u);
        case Type::U64:
        case Type::Pointer:
            return std::make_shared<ValueConst>(static_cast<u64>(0ul));
        case Type::Double:
            return std::make_shared<ValueConst>(0.0);
        default:
            abort();
    }
}

std::unique_ptr<TopLevel> GenerateIr::functionIr(const Parsing::FunDecl& parsingFunction)
{
    using State = SymbolTable::State;
    bool global = !m_symbolTable.lookup(parsingFunction.name).isSet(State::InternalLinkage);
    auto functionTacky = std::make_unique<Function>(parsingFunction.name, global);
    m_global = true;;
    insts = std::move(functionTacky->insts);
    functionTacky->args.reserve(parsingFunction.params.size());
    functionTacky->argTypes.reserve(parsingFunction.params.size());
    const auto funcType = dynCast<const Parsing::FuncType>(parsingFunction.type.get());
    for (size_t i = 0; i < parsingFunction.params.size(); ++i) {
        functionTacky->args.emplace_back(Identifier(parsingFunction.params[i]));
        functionTacky->argTypes.emplace_back(funcType->params[i]->type);
    }
    genBlock(*parsingFunction.body);
    functionTacky->insts = std::move(insts);
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
            const auto decl = dynCast<const Parsing::DeclBlockItem>(&blockItem);
            genDeclaration(*decl->decl);
            break;
        }
        case Kind::Statement: {
            const auto stmtBlockItem = dynCast<const Parsing::StmtBlockItem>(&blockItem);
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
        const auto decl = dynCast<const Parsing::DeclForInit>(&forInit);
        genDeclaration(*decl->decl);
        return;
    }
    if (forInit.kind == Parsing::ForInit::Kind::Expression) {
        const auto expr = dynCast<const Parsing::ExprForInit>(&forInit);
        if (expr->expression)
            genInst(*expr->expression);
        return;
    }
    std::unreachable();
}

void GenerateIr::genDeclaration(const Parsing::Declaration& decl)
{
    if (decl.kind != Parsing::Declaration::Kind::VarDecl)
        return;
    const auto varDecl = dynCast<const Parsing::VarDecl>(&decl);
    if (varDecl->storage == Storage::Static)
        return genStaticLocal(*varDecl);
    if (varDecl->init == nullptr)
        return;
    std::shared_ptr<Value> value = genInstAndConvert(*varDecl->init);
    auto temporary = std::make_shared<ValueVar>(makeTemporaryName(), varDecl->type->type);
    insts.push_back(std::make_unique<CopyInst>(value, temporary, varDecl->type->type));
    const Identifier iden(varDecl->name);
    auto var = std::make_shared<ValueVar>(iden, varDecl->type->type);
    insts.push_back(std::make_unique<CopyInst>(temporary, var, varDecl->type->type));
}

void GenerateIr::genStaticLocal(const Parsing::VarDecl& varDecl)
{
    const bool defined = varDecl.init != nullptr;
    std::shared_ptr<Value> value = genStaticVariableInit(varDecl, defined);
    auto variable = std::make_unique<StaticVariable>(
        varDecl.name, value, varDecl.type->type, false);
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
            const auto ifStmt = dynCast<const Parsing::IfStmt>(&stmt);
            genIfStmt(*ifStmt);
            break;
        }
        case Kind::Return: {
            const auto returnStmt = dynCast<const Parsing::ReturnStmt>(&stmt);
            genReturnStmt(*returnStmt);
            break;
        }
        case Kind::Expression: {
            const auto stmtExpr = dynCast<const Parsing::ExprStmt>(&stmt);
            genInst(*stmtExpr->expr);
            break;
        }
        case Kind::Goto: {
            const auto gotoStmt = dynCast<const Parsing::GotoStmt>(&stmt);
            genGotoStmt(*gotoStmt);
            break;
        }
        case Kind::Compound: {
            const auto compoundStmtPtr = dynCast<const Parsing::CompoundStmt>(&stmt);
            genCompoundStmt(*compoundStmtPtr);
            break;
        }
        case Kind::Break: {
            const auto breakStmtPtr = dynCast<const Parsing::BreakStmt>(&stmt);
            genBreakStmt(*breakStmtPtr);
            break;
        }
        case Kind::Continue: {
            const auto continueStmtPtr = dynCast<const Parsing::ContinueStmt>(&stmt);
            genContinueStmt(*continueStmtPtr);
            break;
        }
        case Kind::Label: {
            const auto labelStmtPtr = dynCast<const Parsing::LabelStmt>(&stmt);
            genLabelStmt(*labelStmtPtr);
            break;
        }
        case Kind::Case: {
            const auto caseStmtPtr = dynCast<const Parsing::CaseStmt>(&stmt);
            genCaseStmt(*caseStmtPtr);
            break;
        }
        case Kind::Default: {
            const auto defaultStmtPtr = dynCast<const Parsing::DefaultStmt>(&stmt);
            genDefaultStmt(*defaultStmtPtr);
            break;
        }
        case Kind::DoWhile: {
            const auto doWhileStmtPtr = dynCast<const Parsing::DoWhileStmt>(&stmt);
            genDoWhileStmt(*doWhileStmtPtr);
            break;
        }
        case Kind::While: {
            const auto whileStmtPtr = dynCast<const Parsing::WhileStmt>(&stmt);
            genWhileStmt(*whileStmtPtr);
            break;
        }
        case Kind::For: {
            const auto forStmtPtr = dynCast<const Parsing::ForStmt>(&stmt);
            genForStmt(*forStmtPtr);
            break;
        }
        case Kind::Switch: {
            const auto switchStmtPtr = dynCast<const Parsing::SwitchStmt>(&stmt);
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
    std::shared_ptr<Value> condition = genInstAndConvert(*ifStmt.condition);
    Identifier endLabelIden = makeTemporaryName();
    insts.emplace_back(std::make_unique<JumpIfZeroInst>(condition, endLabelIden));
    genStmt(*ifStmt.thenStmt);
    insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
}

void GenerateIr::genIfElseStmt(const Parsing::IfStmt& ifStmt)
{
    std::shared_ptr<Value>  condition = genInstAndConvert(*ifStmt.condition);
    Identifier elseStmtLabel = makeTemporaryName();
    insts.emplace_back(std::make_unique<JumpIfZeroInst>(condition, elseStmtLabel));
    genStmt(*ifStmt.thenStmt);
    Identifier endLabelIden = makeTemporaryName();
    insts.emplace_back(std::make_unique<JumpInst>(endLabelIden));
    insts.emplace_back(std::make_unique<LabelInst>(elseStmtLabel));
    genStmt(*ifStmt.elseStmt);
    insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
}

void GenerateIr::genReturnStmt(const Parsing::ReturnStmt& returnStmt)
{
    std::shared_ptr<Value>  value = genInstAndConvert(*returnStmt.expr);
    if (value == nullptr)
        return;
    insts.emplace_back(std::make_unique<ReturnInst>(value, value->type));
}

void GenerateIr::genGotoStmt(const Parsing::GotoStmt& gotoStmt)
{
    insts.emplace_back(std::make_unique<JumpInst>(Identifier(gotoStmt.identifier + ".label")));
}

void GenerateIr::genCompoundStmt(const Parsing::CompoundStmt& compoundStmt)
{
    genBlock(*compoundStmt.block);
}

void GenerateIr::genBreakStmt(const Parsing::BreakStmt& breakStmt)
{
    insts.emplace_back(std::make_unique<JumpInst>(Identifier(breakStmt.identifier + "break")));
}

void GenerateIr::genContinueStmt(const Parsing::ContinueStmt& continueStmt)
{
    insts.emplace_back(std::make_unique<JumpInst>(Identifier(continueStmt.identifier + "continue")));
}

void GenerateIr::genLabelStmt(const Parsing::LabelStmt& labelStmt)
{
    insts.emplace_back(std::make_unique<LabelInst>(Identifier(labelStmt.identifier + ".label")));
    genStmt(*labelStmt.stmt);
}

void GenerateIr::genCaseStmt(const Parsing::CaseStmt& caseStmt)
{
    insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(generateCaseLabelName(caseStmt.identifier))));
    genStmt(*caseStmt.body);
}

void GenerateIr::genDefaultStmt(const Parsing::DefaultStmt& defaultStmt)
{
    insts.emplace_back(std::make_unique<LabelInst>(Identifier(defaultStmt.identifier + "default")));
    genStmt(*defaultStmt.body);
}

void GenerateIr::genDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt)
{
    insts.emplace_back(std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "start")));
    genStmt(*doWhileStmt.body);
    insts.emplace_back(std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "continue")));
    std::shared_ptr<Value> condition = genInstAndConvert(*doWhileStmt.condition);
    insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(
        condition, Identifier(doWhileStmt.identifier + "start")));
    insts.emplace_back(std::make_unique<LabelInst>(Identifier(doWhileStmt.identifier + "break")));
}

void GenerateIr::genWhileStmt(const Parsing::WhileStmt& whileStmt)
{
    insts.emplace_back(std::make_unique<LabelInst>(Identifier(whileStmt.identifier + "continue")));
    auto condition = genInstAndConvert(*whileStmt.condition);
    insts.emplace_back(std::make_unique<JumpIfZeroInst>(
        condition, Identifier(whileStmt.identifier + "break")));
    genStmt(*whileStmt.body);
    insts.emplace_back(std::make_unique<JumpInst>(Identifier(whileStmt.identifier + "continue")));
    insts.emplace_back(std::make_unique<LabelInst>(Identifier(whileStmt.identifier + "break")));
}

void GenerateIr::genForStmt(const Parsing::ForStmt& forStmt)
{
    if (forStmt.init)
        genForInit(*forStmt.init);
    insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(forStmt.identifier + "start"))
    );
    if (forStmt.condition) {
        auto condition = genInstAndConvert(*forStmt.condition);
        insts.emplace_back(std::make_unique<JumpIfZeroInst>(
            condition, Identifier(forStmt.identifier + "break"))
        );
    }
    genStmt(*forStmt.body);
    insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(forStmt.identifier + "continue"))
    );
    if (forStmt.post)
        genInst(*forStmt.post);
    insts.emplace_back(std::make_unique<JumpInst>(
        Identifier(forStmt.identifier + "start"))
    );
    insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(forStmt.identifier + "break"))
    );
}

void GenerateIr::genSwitchStmt(const Parsing::SwitchStmt& stmt)
{
    std::shared_ptr<Value>  realValue = genInstAndConvert(*stmt.condition);
    const Type conditionType = stmt.condition->type->type;
    for (const std::variant<i32, i64, u32, u64>& caseValue : stmt.cases) {
        const auto dst = std::make_shared<ValueVar>(makeTemporaryName(), conditionType);
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
        insts.emplace_back(std::make_unique<BinaryInst>(
            BinaryInst::Operation::Equal, realValue, src2, dst, realValue->type));
        insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(
            dst, Identifier(caseLabelName)));
    }
    if (stmt.hasDefault)
        insts.emplace_back(std::make_unique<JumpInst>(
            Identifier(stmt.identifier + "default")));
    else
        insts.emplace_back(std::make_unique<JumpInst>(Identifier(
                stmt.identifier + "break")));
    genStmt(*stmt.body);
    insts.emplace_back(std::make_unique<LabelInst>(
        Identifier(stmt.identifier + "break")));
}

std::unique_ptr<ExprResult> GenerateIr::genInst(const Parsing::Expr& parsingExpr)
{
    using ExprKind = Parsing::Expr::Kind;
    switch (parsingExpr.kind) {
        case ExprKind::Cast: {
            const auto castExpr = dynCast<const Parsing::CastExpr>(&parsingExpr);
            return genCastInst(*castExpr);
        }
        case ExprKind::Var: {
            const auto varExpr = dynCast<const Parsing::VarExpr>(&parsingExpr);
            return genVarInst(*varExpr);
        }
        case ExprKind::Constant: {
            const auto constExpr = dynCast<const Parsing::ConstExpr>(&parsingExpr);
            return genConstInst(*constExpr);
        }
        case ExprKind::Unary: {
            const auto unaryParsing = dynCast<const Parsing::UnaryExpr>(&parsingExpr);
            return genUnaryInst(*unaryParsing);
        }
        case ExprKind::Binary: {
            const auto binaryParsing = dynCast<const Parsing::BinaryExpr>(&parsingExpr);
            return genBinaryInst(*binaryParsing);
        }
        case ExprKind::Assignment: {
            const auto assignmentExpr = dynCast<const Parsing::AssignmentExpr>(&parsingExpr);
            return genAssignInst(*assignmentExpr);
        }
        case ExprKind::Ternary: {
            const auto ternaryExpr = dynCast<const Parsing::TernaryExpr>(&parsingExpr);
            return genTernaryInst(*ternaryExpr);
        }
        case ExprKind::FunctionCall: {
            const auto funcCallExpr = dynCast<const Parsing::FuncCallExpr>(&parsingExpr);
            return genFuncCallInst(*funcCallExpr);
        }
        case ExprKind::Dereference: {
            const auto dereferenceExpr = dynCast<const Parsing::DereferenceExpr>(&parsingExpr);
            return genDereferenceInst(*dereferenceExpr);
        }
        case ExprKind::AddrOf: {
            const auto addrOfExpr = dynCast<const Parsing::AddrOffExpr>(&parsingExpr);
            return genAddrOfInst(*addrOfExpr);
        }
        default:
            assert("Unexpected expression type ir generateInst");
    }
    std::unreachable();
}

std::shared_ptr<Value> GenerateIr::genInstAndConvert(const Parsing::Expr& parsingExpr)
{
    const std::unique_ptr<ExprResult> result = genInst(parsingExpr);
    switch (result->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto plainOperand = dynCast<const PlainOperand>(result.get());
            return plainOperand->value;
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto dereferencedPointer = dynCast<const DereferencedPointer>(result.get());
            Identifier dstIden = makeTemporaryName();
            std::shared_ptr<Value> dst = std::make_shared<ValueVar>(dstIden, dereferencedPointer->referredToType);
            insts.emplace_back(std::make_unique<LoadInst>(
                dereferencedPointer->ptr, dst, dereferencedPointer->referredToType));
            return dst;
        }
    }
    std::unreachable();
}

std::unique_ptr<ExprResult> GenerateIr::genCastInst(const Parsing::CastExpr& castExpr)
{
    std::shared_ptr<Value> result = genInstAndConvert(*castExpr.expr);
    const Type type = castExpr.type->type;
    const Type innerType = castExpr.expr->type->type;
    const Identifier iden(makeTemporaryName());
    auto dst = std::make_shared<ValueVar>(iden, type);
    if (type == Type::Double && !isSigned(innerType))
        insts.emplace_back(std::make_unique<UIntToDoubleInst>(result, dst, type));
    else if (type == Type::Double && isSigned(innerType))
        insts.emplace_back(std::make_unique<IntToDoubleInst>(result, dst, type));
    else if (!isSigned(type) && innerType == Type::Double)
        insts.emplace_back(std::make_unique<DoubleToUIntInst>(result, dst, type));
    else if (isSigned(type) && innerType == Type::Double)
        insts.emplace_back(std::make_unique<DoubleToIntInst>(result, dst, type));
    else if (getSize(type) == getSize(innerType))
        insts.emplace_back(std::make_unique<CopyInst>(result, dst, type));
    else if (getSize(type) < getSize(innerType))
        insts.emplace_back(std::make_unique<TruncateInst>(result, dst, innerType));
    else if (isSigned(innerType))
        insts.emplace_back(std::make_unique<SignExtendInst>(result, dst, type));
    else
        insts.emplace_back(std::make_unique<ZeroExtendInst>(result, dst, type));
    return std::make_unique<PlainOperand>(dst);
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryInst(const Parsing::UnaryExpr& unaryExpr)
{
    if (isPostfixOp(unaryExpr.op))
        return genUnaryPostfixInst(unaryExpr);
    if (isPrefixOp(unaryExpr.op))
        return genUnaryPrefixInst(unaryExpr);
    if (unaryExpr.op == Parsing::UnaryExpr::Operator::Plus)
        return genInst(*unaryExpr.operand);
    return genUnaryBasicInst(unaryExpr);
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryBasicInst(const Parsing::UnaryExpr& unaryExpr)
{
    UnaryInst::Operation operation = convertUnaryOperation(unaryExpr.op);
    std::shared_ptr<Value> src = genInstAndConvert(*unaryExpr.operand);
    auto dst = std::make_shared<ValueVar>(makeTemporaryName(), unaryExpr.type->type);
    insts.emplace_back(std::make_unique<UnaryInst>(operation, src, dst, unaryExpr.type->type));
    return std::make_unique<PlainOperand>(dst);
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr)
{
    auto originalForReturn = std::make_shared<ValueVar>(makeTemporaryName(), unaryExpr.type->type);
    auto tempNew = std::make_shared<ValueVar>(makeTemporaryName(), unaryExpr.type->type);
    const auto oper = getPostPrefixOperation(unaryExpr.op);
    const Type type = unaryExpr.type->type;
    std::shared_ptr<ValueConst> one;
    if (type == Type::Double)
        one = std::make_shared<ValueConst>(1.0);
    else
        one = std::make_shared<ValueConst>(1);
    const std::shared_ptr<ExprResult> original = genInst(*unaryExpr.operand);
    switch (original->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto plainOriginal = dynCast<const PlainOperand>(original.get());
            insts.emplace_back(std::make_unique<CopyInst>(plainOriginal->value, originalForReturn, type));
            insts.emplace_back(std::make_unique<BinaryInst>(
                oper, originalForReturn, one, tempNew, type));
            insts.emplace_back(std::make_unique<CopyInst>(tempNew, plainOriginal->value, type));
            return std::make_unique<PlainOperand>(originalForReturn);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto derefOriginal = dynCast<const DereferencedPointer>(original.get());
            const auto derefValue = std::make_shared<ValueVar>(Identifier(makeTemporaryName()), type);
            insts.emplace_back(std::make_unique<LoadInst>(derefOriginal->ptr, derefValue, type));
            insts.emplace_back(std::make_unique<CopyInst>(derefValue, originalForReturn, type));
            insts.emplace_back(std::make_unique<BinaryInst>(
                oper, originalForReturn, one, tempNew, type));
            insts.emplace_back(std::make_unique<StoreInst>(tempNew, derefOriginal->ptr, type));
            return std::make_unique<PlainOperand>(originalForReturn);
        }
    }
    return std::make_unique<PlainOperand>(originalForReturn);
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr)
{
    const Type type = unaryExpr.type->type;
    std::shared_ptr<ValueConst> one;
    if (type == Type::Double)
        one = std::make_shared<ValueConst>(1.0);
    else
        one = std::make_shared<ValueConst>(1);
    const auto temp = std::make_shared<ValueVar>(Identifier(makeTemporaryName()), type);
    const auto operation = getPostPrefixOperation(unaryExpr.op);
    const std::unique_ptr<ExprResult> original = genInst(*unaryExpr.operand);
    switch (original->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto originalPlain = dynCast<const PlainOperand>(original.get());
            insts.emplace_back(std::make_unique<BinaryInst>(
                operation, originalPlain->value, one, temp, type));
            insts.emplace_back(std::make_unique<CopyInst>(
                temp, originalPlain->value, type));
            return std::make_unique<PlainOperand>(temp);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto derefPtr = dynCast<const DereferencedPointer>(original.get());
            const auto derefValue = std::make_shared<ValueVar>(Identifier(makeTemporaryName()), type);
            insts.emplace_back(std::make_unique<LoadInst>(derefPtr->ptr, derefValue, type));
            insts.emplace_back(std::make_unique<BinaryInst>(
                operation, derefValue, one, temp, type));
            insts.emplace_back(std::make_unique<StoreInst>(
                temp, derefPtr->ptr, type));
            return std::make_unique<PlainOperand>(temp);
        }
    }
    std::abort();
}

std::unique_ptr<ExprResult> GenerateIr::genVarInst(const Parsing::VarExpr& varExpr)
{
    const Identifier iden(varExpr.name);
    auto var = std::make_shared<ValueVar>(iden, varExpr.type->type);
    var->referingTo = varExpr.referingTo;
    return std::make_unique<PlainOperand>(var);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryInst(const Parsing::BinaryExpr& binaryExpr)
{
    BinaryInst::Operation operation = convertBinaryOperation(binaryExpr.op);
    if (operation == BinaryInst::Operation::And)
        return genBinaryAndInst(binaryExpr);
    if (operation == BinaryInst::Operation::Or)
        return genBinaryOrInst(binaryExpr);
    std::shared_ptr<Value> lhs = genInstAndConvert(*binaryExpr.lhs);
    std::shared_ptr<Value> rhs = genInstAndConvert(*binaryExpr.rhs);

    auto destination = std::make_shared<ValueVar>(makeTemporaryName(), binaryExpr.type->type);

    insts.emplace_back(std::make_unique<BinaryInst>(
        operation, lhs, rhs, destination, binaryExpr.type->type));
    return std::make_unique<PlainOperand>(destination);
}

std::unique_ptr<ExprResult> GenerateIr::genAssignInst(const Parsing::AssignmentExpr& assignmentExpr)
{
    const std::unique_ptr<ExprResult> lhs = genInst(*assignmentExpr.lhs);
    const std::shared_ptr<Value> rhs = genInstAndConvert(*assignmentExpr.rhs);
    switch (lhs->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto plainLhs = dynCast<const PlainOperand>(lhs.get());
            insts.emplace_back(std::make_unique<CopyInst>(
                rhs, plainLhs->value, assignmentExpr.type->type));
            return std::make_unique<PlainOperand>(plainLhs->value);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto derefLhs = dynCast<const DereferencedPointer>(lhs.get());
            insts.emplace_back(std::make_unique<StoreInst>(
                rhs, derefLhs->ptr, assignmentExpr.type->type));
            return std::make_unique<PlainOperand>(rhs);
        }
    }
    std::unreachable();
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryAndInst(const Parsing::BinaryExpr& binaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), binaryExpr.type->type);
    std::shared_ptr<Value> lhs = genInstAndConvert(*binaryExpr.lhs);
    Identifier falseLabelIden = makeTemporaryName();
    insts.emplace_back(std::make_unique<JumpIfZeroInst>(lhs, falseLabelIden));
    std::shared_ptr<Value> rhs = genInstAndConvert(*binaryExpr.rhs);
    insts.emplace_back(std::make_unique<JumpIfZeroInst>(rhs, falseLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    insts.emplace_back(std::make_unique<CopyInst>(oneVal, result, binaryExpr.type->type));
    Identifier endLabelIden = makeTemporaryName();
    insts.emplace_back(std::make_unique<JumpInst>(endLabelIden));
    insts.emplace_back(std::make_unique<LabelInst>(falseLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    insts.emplace_back(std::make_unique<CopyInst>(zeroVal, result, binaryExpr.type->type));
    insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryOrInst(const Parsing::BinaryExpr& binaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), binaryExpr.type->type);
    std::shared_ptr<Value> lhs = genInstAndConvert(*binaryExpr.lhs);
    Identifier trueLabelIden = makeTemporaryName();
    insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(lhs, trueLabelIden));
    std::shared_ptr<Value> rhs = genInstAndConvert(*binaryExpr.rhs);
    insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(rhs, trueLabelIden));
    auto zeroVal = std::make_shared<ValueConst>(0);
    insts.emplace_back(std::make_unique<CopyInst>(zeroVal, result, binaryExpr.type->type));
    Identifier endLabelIden = makeTemporaryName();
    insts.emplace_back(std::make_unique<JumpInst>(endLabelIden));
    insts.emplace_back(std::make_unique<LabelInst>(trueLabelIden));
    auto oneVal = std::make_shared<ValueConst>(1);
    insts.emplace_back(std::make_unique<CopyInst>(oneVal, result, binaryExpr.type->type));
    insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genConstInst(const Parsing::ConstExpr& constExpr)
{
    std::shared_ptr<ValueConst> result = nullptr;
    if (constExpr.type->type == Type::I32)
        result = std::make_shared<ValueConst>(std::get<i32>(constExpr.value));
    if (constExpr.type->type == Type::U32)
        result = std::make_shared<ValueConst>(std::get<u32>(constExpr.value));
    if (constExpr.type->type == Type::U64)
        result = std::make_shared<ValueConst>(std::get<u64>(constExpr.value));
    if (constExpr.type->type == Type::I64)
        result = std::make_shared<ValueConst>(std::get<i64>(constExpr.value));
    if (constExpr.type->type == Type::Double)
        result = std::make_shared<ValueConst>(std::get<double>(constExpr.value));
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genTernaryInst(const Parsing::TernaryExpr& ternaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), ternaryExpr.type->type);

    Identifier endLabelIden = makeTemporaryName();
    Identifier falseLabelName = makeTemporaryName();

    const auto conditionalExpr = dynCast<const Parsing::TernaryExpr>(&ternaryExpr);
    std::shared_ptr<Value> condition = genInstAndConvert(*conditionalExpr->condition);

    insts.emplace_back(std::make_unique<JumpIfZeroInst>(condition, falseLabelName));

    std::shared_ptr<Value> trueValue = genInstAndConvert(*conditionalExpr->trueExpr);
    insts.emplace_back(std::make_unique<CopyInst>(trueValue, result, trueValue->type));
    insts.emplace_back(std::make_unique<JumpInst>(endLabelIden));

    insts.emplace_back(std::make_unique<LabelInst>(falseLabelName));
    std::shared_ptr<Value> falseValue = genInstAndConvert(*conditionalExpr->falseExpr);
    insts.emplace_back(std::make_unique<CopyInst>(falseValue, result, falseValue->type));

    insts.emplace_back(std::make_unique<LabelInst>(endLabelIden));
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genFuncCallInst(const Parsing::FuncCallExpr& funcCallExpr)
{
    std::vector<std::shared_ptr<Value>> arguments;
    arguments.reserve(funcCallExpr.args.size());
    for (const auto& expr : funcCallExpr.args)
        arguments.emplace_back(genInstAndConvert(*expr));
    const auto returnType = static_cast<const Parsing::VarType*>(funcCallExpr.type.get());
    auto dst = std::make_shared<ValueVar>(makeTemporaryName(), funcCallExpr.type->type);
    insts.emplace_back(std::make_unique<FunCallInst>(
        Identifier(funcCallExpr.name), std::move(arguments), dst, returnType->type));
    return std::make_unique<PlainOperand>(dst);
}

std::unique_ptr<ExprResult> GenerateIr::genAddrOfInst(const Parsing::AddrOffExpr& addrOffExpr)
{
    if (addrOffExpr.reference->kind == Parsing::Expr::Kind::Dereference) {
        const auto deferenceExpr = dynCast<const Parsing::DereferenceExpr>(addrOffExpr.reference.get());
        return genInst(*deferenceExpr->reference);
    }
    const std::unique_ptr<ExprResult> inner = genInst(*addrOffExpr.reference);
    switch (inner->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto plainOperand = dynCast<const PlainOperand>(inner.get());
            Identifier dstIden = makeTemporaryName();
            std::shared_ptr<Value> dst = std::make_shared<ValueVar>(dstIden, Type::Pointer);
            insts.emplace_back(std::make_unique<GetAddressInst>(plainOperand->value, dst, Type::Pointer));
            return std::make_unique<PlainOperand>(dst);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto dereferencedPointed = dynCast<const DereferencedPointer>(inner.get());
            return std::make_unique<PlainOperand>(dereferencedPointed->ptr);
        }
    }
    std::unreachable();
}

std::unique_ptr<ExprResult> GenerateIr::genDereferenceInst(const Parsing::DereferenceExpr& dereferenceExpr)
{
    std::shared_ptr<Value> result = genInstAndConvert(*dereferenceExpr.reference);
    return std::make_unique<DereferencedPointer>(result, dereferenceExpr.type->type);
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