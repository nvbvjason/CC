#include "ASTParser.hpp"
#include "GenerateIr.hpp"
#include "Types/TypeConversion.hpp"
#include "ASTTypes.hpp"
#include "AstToIrOperators.hpp"
#include "DynCast.hpp"
#include "ASTUtils.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <strings.h>

namespace Ir {
static Identifier makeTemporaryName();
static Identifier makeTemporaryName(const Value* value);
static Identifier makeTemporaryName(const std::string& name);
static std::string generateCaseLabelName(std::string before);
static Type getSubscriptDereferenceType(Parsing::TypeBase* typeBase);

void GenerateIr::program(const Parsing::Program& parsingProgram, Program& tackyProgram)
{
    for (const std::unique_ptr<Parsing::Declaration>& decl : parsingProgram.declarations) {
        std::unique_ptr<TopLevel> topLevel = topLevelIr(*decl);
        if (topLevel == nullptr)
            continue;
        topLevels.emplace_back(std::move(topLevel));
    }
    tackyProgram.topLevels = std::move(topLevels);
    tackyProgram.structs = std::move(irStructs);
    tackyProgram.values = std::move(values);
}

std::unique_ptr<TopLevel> GenerateIr::topLevelIr(const Parsing::Declaration& decl)
{
    using Kind = Parsing::Declaration::Kind;
    switch (decl.kind) {
        case Kind::FuncDecl: {
            const auto funcDeclaration = dynCast<const Parsing::FuncDecl>(&decl);
            if (funcDeclaration->body == nullptr)
                return nullptr;
            return functionIr(*funcDeclaration);
        }
        case Kind::VarDecl: {
            const auto varDecl = dynCast<const Parsing::VarDecl>(&decl);
            return staticVariableIr(*varDecl);
        }
        case Kind::StructuredDecl: {
            const auto structured = dynCast<const Parsing::StructuredDecl>(&decl);
            return structuredDecl(*structured);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<TopLevel> GenerateIr::structuredDecl(const Parsing::StructuredDecl& structuredDecl)
{
    std::vector<IrType> types;
    std::vector<i64> offsets;
    for (const auto& member : structuredDecl.members) {
        types.push_back(convert(*member->type));
        offsets.push_back(0);
    }
    IrStruct structured(std::move(types), std::move(offsets));
    irStructs.emplace(structuredDecl.identifier, structured);
    return nullptr;
}

void GenerateIr::allocateLocal(const Parsing::VarDecl& varDecl)
{
    const i64 size = typeTable.getSize(varDecl.type.get());
    emitAllocate(size, varDecl.name);
}

void GenerateIr::directlyPushConstant32Bit(const Parsing::VarDecl& varDecl, const Value* value)
{
    const Identifier iden(varDecl.name);
    const Value* var = genValueVar(iden, convert(*varDecl.type));
    emitCopy(value, var, convert(*varDecl.type));
}

void GenerateIr::genDeclaration(const Parsing::Declaration& decl)
{
    using InitKind = Parsing::Initializer::Kind;

    if (decl.kind != Parsing::Declaration::Kind::VarDecl)
        return;
    const auto varDecl = dynCast<const Parsing::VarDecl>(&decl);
    if (varDecl->storage == Storage::Static)
        return genStaticLocal(*varDecl);
    if (varDecl->init == nullptr &&
        (varDecl->type->type == Type::Array ||
            varDecl->type->type == Type::Struct ||
            varDecl->type->type == Type::Union)) {
        allocateLocal(*varDecl);
        return;
    }
    if (varDecl->init == nullptr)
        return;
    switch (varDecl->init->kind) {
        case InitKind::Single:
            genSingleDeclaration(*varDecl);
            break;
        case InitKind::Compound:
            genCompoundLocalInit(*varDecl);
            break;
        default:
            std::abort();
    }
}

void GenerateIr::genSingleDeclaration(const Parsing::VarDecl& varDecl)
{
    const auto singleInit = dynCast<Parsing::SingleInitializer>(varDecl.init.get());
    const Value* value = genInstAndConvert(*singleInit->expr);
    if (value->kind == Value::Kind::Constant &&
        (varDecl.type->type == Type::I32 || varDecl.type->type == Type::U32)) {
        directlyPushConstant32Bit(varDecl, value);
        return;
    }
    const auto temporary = genValueVar(makeTemporaryName(), convert(*varDecl.type));
    emitCopy(value, temporary, convert(*varDecl.type));
    const Identifier iden(varDecl.name);
    const Value* var = genValueVar(iden, convert(*varDecl.type));
    emitCopy(temporary, var, convert(*varDecl.type));
}

void GenerateIr::genSingleLocalInit(const std::string& name,
                                    const i64 arraySize,
                                    const i64 alignment,
                                    i64& offset,
                                    const Parsing::SingleInitializer& singleInit)
{
    using ExprKind = Parsing::Expr::Kind;
    const i64 typeSize = singleInit.expr->kind == ExprKind::String ? 8 : typeTable.getSize(singleInit.expr->type.get());
    const Value* value = genInstAndConvert(*singleInit.expr);
    if (singleInit.expr->kind == Parsing::Expr::Kind::String) {
        const Value* var = genValueVar(makeTemporaryName(), convertType(Type::Pointer));
        emitGetAddress(value, var, convertType(Type::Pointer));
        value = var;
    }
    emitCopyToOffset(
        value,
        Identifier(name),
        ReferringTo::Local,
        offset, arraySize,
        alignment, convert(*singleInit.expr->type));
    offset += typeSize;
}

void GenerateIr::genCompoundLocalInit(const Parsing::VarDecl& varDecl)
{
    const auto compoundInit = dynCast<Parsing::CompoundInitializer>(varDecl.init.get());
    const i64 declSize = typeTable.getSize(varDecl.type.get());
    i64 alignment = typeTable.getAlignment(varDecl.type.get());
    if (varDecl.type->kind == Parsing::TypeBase::Kind::Array) {
        const Parsing::TypeBase* innerType = Parsing::getArrayBaseType(*varDecl.type);
        const i64 innerSize = typeTable.getSize(innerType);
        const i64 length = declSize * innerSize;
        if (16 < length)
            alignment = 16;
    }
    i64 offset = 0;
    for (const auto& init : compoundInit->initializers) {
        switch (init->kind) {
            case Parsing::Initializer::Kind::Single: {
                const auto singleInit = dynCast<Parsing::SingleInitializer>(init.get());
                genSingleLocalInit(varDecl.name, declSize, alignment, offset, *singleInit);
                break;
            }
            case Parsing::Initializer::Kind::Zero: {
                const auto zeroInit = dynCast<Parsing::ZeroInitializer>(init.get());
                genZeroLocalInit(varDecl.name,
                                 declSize,
                                 alignment,
                                 zeroInit->size,
                                 offset);
                break;
            }
            default:
                std::abort();
        }
    }
}

void GenerateIr::genZeroLocalInit(const std::string& name,
                                  const i64 arraySize,
                                  const i64 alignment,
                                  const i64 lengthZeroInit,
                                  i64& offset)
{
    size_t i = 0;
    for (; i + 8 <= lengthZeroInit; i += 8) {
        emitCopyToOffset(
            zeroConst8, Identifier(name), ReferringTo::Local, offset, arraySize, alignment, u8Type);
        offset += 8;
    }
    for (; i + 4 <= lengthZeroInit; i += 4) {
        emitCopyToOffset(
            zeroConst4, Identifier(name), ReferringTo::Local, offset, arraySize, alignment, u8Type);
        offset += 4;
    }
    for (; i < lengthZeroInit; ++i) {
        emitCopyToOffset(
    zeroConst1, Identifier(name), ReferringTo::Local, offset, arraySize, alignment, u8Type);
        ++offset;
    }
}

void GenerateIr::genStaticLocal(const Parsing::VarDecl& varDecl)
{
    const bool defined = varDecl.init != nullptr;
    auto variable = genStaticInit(varDecl, defined);
    topLevels.emplace_back(std::move(variable));
    symbolTable.addEntry(varDecl.name,
                           varDecl.name,
                           *varDecl.type,
                           true, false, false, defined);
}

std::unique_ptr<TopLevel> GenerateIr::staticVariableIr(const Parsing::VarDecl& varDecl)
{
    const auto entry = symbolTable.lookupEntry(varDecl.name);
    const bool defined = entry.isDefined();

    if (defined && varDecl.init == nullptr)
        return nullptr;
    if (!defined && varDecl.storage == Storage::Extern)
        return nullptr;
    if (writtenGlobals.contains(varDecl.name))
        return nullptr;

    writtenGlobals.insert(varDecl.name);
    if (varDecl.init == nullptr)
        return genStaticWithoutInit(varDecl);
    if (varDecl.init->kind == Parsing::Initializer::Kind::Compound)
        return genCompoundInit(varDecl);

    const Value* value = genStaticVariableInit(varDecl);
    auto variable = std::make_unique<StaticVariable>(
        varDecl.name, value, convert(*varDecl.type), varDecl.storage != Storage::Static);
    return variable;
}

std::unique_ptr<TopLevel> GenerateIr::genStaticWithoutInit(const Parsing::VarDecl& varDecl)
{
    if (varDecl.type->kind == Parsing::TypeBase::Kind::Pointer ||
        varDecl.type->kind == Parsing::TypeBase::Kind::Var) {
        const auto zeroValue = genZeroValueForType(varDecl.type->type);
        return std::make_unique<StaticVariable>(
            varDecl.name, zeroValue, convert(*varDecl.type), varDecl.storage != Storage::Static);
    }
    std::vector<std::unique_ptr<Initializer>> initializers;
    const i64 size = typeTable.getSize(varDecl.type.get());
    initializers.emplace_back(std::make_unique<ZeroInitializer>(size));
    auto variable = std::make_unique<StaticArray>(
        varDecl.name, std::move(initializers), varDecl.storage != Storage::Static);
    return variable;
}

std::unique_ptr<TopLevel> GenerateIr::genCompoundInit(const Parsing::VarDecl& varDecl)
{
    std::vector<std::unique_ptr<Initializer>> initializers = genStaticCompoundInit(varDecl);
    auto variable = std::make_unique<StaticArray>(
        varDecl.name, std::move(initializers), varDecl.storage != Storage::Static);
    return variable;
}

std::vector<std::unique_ptr<Initializer>> GenerateIr::genStaticCompoundInit(
        const Parsing::VarDecl& varDecl)
{
    std::vector<std::unique_ptr<Initializer>> initializers;
    const auto compoundInit = dynCast<Parsing::CompoundInitializer>(varDecl.init.get());
    for (const auto& stuff : compoundInit->initializers) {
        switch (stuff->kind) {
            case Parsing::Initializer::Kind::Single: {
                const auto singleInit = dynCast<Parsing::SingleInitializer>(stuff.get());
                const Value* value = genInstAndConvert(*singleInit->expr);
                initializers.emplace_back(std::make_unique<ValueInitializer>(value));
                break;
            }
            case Parsing::Initializer::Kind::Zero: {
                const auto zeroInit = dynCast<Parsing::ZeroInitializer>(stuff.get());
                initializers.emplace_back(std::make_unique<ZeroInitializer>(zeroInit->size));
                break;
            }
            default:
                std::abort();
        }
    }
    return initializers;
}

std::unique_ptr<TopLevel> GenerateIr::genStaticInit(const Parsing::VarDecl& varDecl, const bool defined)
{
    if (!defined || varDecl.init == nullptr)
        return genStaticWithoutInit(varDecl);
    if (varDecl.init->kind == Parsing::Initializer::Kind::Compound) {
        auto initializers = genStaticCompoundInit(varDecl);
        return std::make_unique<StaticArray>(varDecl.name, std::move(initializers), false);
    }
    const Value* value = genStaticVariableInit(varDecl);
    return std::make_unique<StaticVariable>(varDecl.name, value, convert(*varDecl.type), false);
}

const Value* GenerateIr::genStaticVariableInit(const Parsing::VarDecl& varDecl)
{
    const auto singleInit = dynCast<Parsing::SingleInitializer>(varDecl.init.get());
    return genInstAndConvert(*singleInit->expr);
}

std::unique_ptr<TopLevel> GenerateIr::functionIr(const Parsing::FuncDecl& parsingFunction)
{
    bool globalFunction = !symbolTable.lookupEntry(parsingFunction.name).hasInternalLinkage();
    auto functionTacky = std::make_unique<Function>(parsingFunction.name, globalFunction);
    inGlobalScope = true;
    insts = std::move(functionTacky->insts);
    insts.reserve(parsingFunction.body->body.size() * 3);
    functionTacky->args.reserve(parsingFunction.params.size());
    functionTacky->argTypes.reserve(parsingFunction.params.size());
    const auto funcType = dynCast<const Parsing::FuncType>(parsingFunction.type.get());
    for (size_t i = 0; i < parsingFunction.params.size(); ++i) {
        functionTacky->args.emplace_back(Identifier(parsingFunction.params[i]));
        const IrType irType = convert(*funcType->params[i]);
        functionTacky->argTypes.emplace_back(irType);
    }
    genBlock(*parsingFunction.body);
    functionTacky->insts = std::move(insts);
    inGlobalScope = false;
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
            return genDeclaration(*decl->decl);
        }
        case Kind::Statement: {
            const auto stmtBlockItem = dynCast<const Parsing::StmtBlockItem>(&blockItem);
            return genStmt(*stmtBlockItem->stmt);
        }
        default:
            std::abort();
    }
}

void GenerateIr::genForInit(const Parsing::ForInit& forInit)
{
    switch (forInit.kind) {
        case Parsing::ForInit::Kind::Declaration: {
            const auto decl = dynCast<const Parsing::DeclForInit>(&forInit);
            genDeclaration(*decl->decl);
            return;
        }
        case Parsing::ForInit::Kind::Expression: {
            const auto expr = dynCast<const Parsing::ExprForInit>(&forInit);
            if (expr->expression)
                genInst(*expr->expression);
            return;
        }
        default:
            std::abort();
    }
}

void GenerateIr::genStmt(const Parsing::Stmt& stmt)
{
    using Kind = Parsing::Stmt::Kind;
    switch (stmt.kind) {
        case Kind::Expression: {
            const auto stmtExpr = dynCast<const Parsing::ExprStmt>(&stmt);
            genInst(*stmtExpr->expr);
            break;
        }
        case Kind::If: {
            const auto ifStmt = dynCast<const Parsing::IfStmt>(&stmt);
            return genIfStmt(*ifStmt);
        }
        case Kind::Return: {
            const auto returnStmt = dynCast<const Parsing::ReturnStmt>(&stmt);
            return genReturnStmt(*returnStmt);
        }
        case Kind::Goto: {
            const auto gotoStmt = dynCast<const Parsing::GotoStmt>(&stmt);
            return genGotoStmt(*gotoStmt);
        }
        case Kind::Compound: {
            const auto compoundStmtPtr = dynCast<const Parsing::CompoundStmt>(&stmt);
            return genCompoundStmt(*compoundStmtPtr);
        }
        case Kind::Break: {
            const auto breakStmtPtr = dynCast<const Parsing::BreakStmt>(&stmt);
            return genBreakStmt(*breakStmtPtr);
        }
        case Kind::Continue: {
            const auto continueStmtPtr = dynCast<const Parsing::ContinueStmt>(&stmt);
            return genContinueStmt(*continueStmtPtr);
        }
        case Kind::Label: {
            const auto labelStmtPtr = dynCast<const Parsing::LabelStmt>(&stmt);
            return genLabelStmt(*labelStmtPtr);
        }
        case Kind::Case: {
            const auto caseStmtPtr = dynCast<const Parsing::CaseStmt>(&stmt);
            return genCaseStmt(*caseStmtPtr);
        }
        case Kind::Default: {
            const auto defaultStmtPtr = dynCast<const Parsing::DefaultStmt>(&stmt);
            return genDefaultStmt(*defaultStmtPtr);
        }
        case Kind::DoWhile: {
            const auto doWhileStmtPtr = dynCast<const Parsing::DoWhileStmt>(&stmt);
            return genDoWhileStmt(*doWhileStmtPtr);
        }
        case Kind::While: {
            const auto whileStmtPtr = dynCast<const Parsing::WhileStmt>(&stmt);
            return genWhileStmt(*whileStmtPtr);
        }
        case Kind::For: {
            const auto forStmtPtr = dynCast<const Parsing::ForStmt>(&stmt);
            return genForStmt(*forStmtPtr);
        }
        case Kind::Switch: {
            const auto switchStmtPtr = dynCast<const Parsing::SwitchStmt>(&stmt);
            return genSwitchStmt(*switchStmtPtr);
        }
        case Kind::Null:
            break;
        default:
            std::abort();
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
    const Value* condition = genInstAndConvert(*ifStmt.condition);
    const Identifier endLabelIden = makeTemporaryName();

    emitJumpIfZero(condition, endLabelIden);
    genStmt(*ifStmt.thenStmt);
    emitLabel(endLabelIden);
}

void GenerateIr::genIfElseStmt(const Parsing::IfStmt& ifStmt)
{
    const Value* condition = genInstAndConvert(*ifStmt.condition);
    const Identifier elseLabelIden = makeTemporaryName();
    const Identifier endLabelIden = makeTemporaryName();

    emitJumpIfZero(condition, elseLabelIden);
    genStmt(*ifStmt.thenStmt);
    emitJump(endLabelIden);
    emitLabel(elseLabelIden);
    genStmt(*ifStmt.elseStmt);
    emitLabel(endLabelIden);
}

void GenerateIr::genReturnStmt(const Parsing::ReturnStmt& returnStmt)
{
    if (returnStmt.expr) {
        const Value* value = genInstAndConvert(*returnStmt.expr);
        if (value == nullptr)
            return;
        emitReturn(value, value->type);
        return;
    }
    emitReturn();
}

void GenerateIr::genGotoStmt(const Parsing::GotoStmt& gotoStmt)
{
    emitJump(Identifier(gotoStmt.identifier + ".label"));
}

void GenerateIr::genCompoundStmt(const Parsing::CompoundStmt& compoundStmt)
{
    genBlock(*compoundStmt.block);
}

void GenerateIr::genBreakStmt(const Parsing::BreakStmt& breakStmt)
{
    emitJump(Identifier(breakStmt.identifier + "break"));
}

void GenerateIr::genContinueStmt(const Parsing::ContinueStmt& continueStmt)
{
    emitJump(Identifier(continueStmt.identifier + "continue"));
}

void GenerateIr::genLabelStmt(const Parsing::LabelStmt& labelStmt)
{
    emitLabel(Identifier(labelStmt.identifier + ".label"));
    genStmt(*labelStmt.stmt);
}

void GenerateIr::genCaseStmt(const Parsing::CaseStmt& caseStmt)
{
    emitLabel(
        Identifier(generateCaseLabelName(caseStmt.identifier)));
    genStmt(*caseStmt.body);
}

void GenerateIr::genDefaultStmt(const Parsing::DefaultStmt& defaultStmt)
{
    emitLabel(Identifier(defaultStmt.identifier + "default"));
    genStmt(*defaultStmt.body);
}

void GenerateIr::genDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt)
{
    emitLabel(Identifier(doWhileStmt.identifier + "start"));
    genStmt(*doWhileStmt.body);
    emitLabel(Identifier(doWhileStmt.identifier + "continue"));
    const Value* condition = genInstAndConvert(*doWhileStmt.condition);
    emitJumpIfNotZero(condition, Identifier(doWhileStmt.identifier + "start"));
    emitLabel(Identifier(doWhileStmt.identifier + "break"));
}

void GenerateIr::genWhileStmt(const Parsing::WhileStmt& whileStmt)
{
    const auto continueIden = Identifier(whileStmt.identifier + "continue");
    const auto breakIden = Identifier(whileStmt.identifier + "break");

    emitLabel(continueIden);
    const auto condition = genInstAndConvert(*whileStmt.condition);
    emitJumpIfZero(condition, breakIden);
    genStmt(*whileStmt.body);
    emitJump(continueIden);
    emitLabel(breakIden);
}

void GenerateIr::genForStmt(const Parsing::ForStmt& forStmt)
{
    if (forStmt.init)
        genForInit(*forStmt.init);
    emitLabel(Identifier(forStmt.identifier + "start"));
    if (forStmt.condition) {
        const auto condition = genInstAndConvert(*forStmt.condition);
        emitJumpIfZero(condition, Identifier(forStmt.identifier + "break"));
    }
    genStmt(*forStmt.body);
    emitLabel(Identifier(forStmt.identifier + "continue"));
    if (forStmt.post)
        genInst(*forStmt.post);
    emitJump(Identifier(forStmt.identifier + "start"));
    emitLabel(Identifier(forStmt.identifier + "break"));
}

void GenerateIr::genSwitchStmt(const Parsing::SwitchStmt& stmt)
{
    const Value* realValue = genInstAndConvert(*stmt.condition);
    const Type conditionType = stmt.condition->type->type;
    for (const std::variant<i32, i64, u32, u64>& caseValue : stmt.cases) {
        const Value* dst = genValueVar(makeTemporaryName(), convertType(conditionType));
        std::string caseLabelName;
        const Value* src2;
        if (conditionType == Type::I32) {
            const i32 value = std::get<i32>(caseValue);
            caseLabelName = generateCaseLabelName(stmt.identifier + std::to_string(value));
            src2 = genConstValue(value);
        }
        if (conditionType == Type::I64) {
            const i64 value = std::get<i64>(caseValue);
            caseLabelName = generateCaseLabelName(stmt.identifier + std::to_string(value));
            src2 = genConstValue(value);
        }
        if (conditionType == Type::U32) {
            const u32 value = std::get<u32>(caseValue);
            caseLabelName = generateCaseLabelName(stmt.identifier + std::to_string(value));
            src2 = genConstValue(value);
        }
        if (conditionType == Type::U64) {
            const u64 value = std::get<u64>(caseValue);
            caseLabelName = generateCaseLabelName(stmt.identifier + std::to_string(value));
            src2 = genConstValue(value);
        }
        emitBinary(BinaryInst::Operation::Equal, realValue->type, realValue, src2, dst);
        emitJumpIfNotZero(dst, Identifier(caseLabelName));
    }
    if (stmt.hasDefault)
        emitJump(Identifier(stmt.identifier + "default"));
    else
        emitJump(Identifier(stmt.identifier + "break"));
    genStmt(*stmt.body);
    emitLabel(Identifier(stmt.identifier + "break"));
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
            return genConstPlainOperand(*constExpr);
        }
        case ExprKind::String: {
            const auto stringExpr = dynCast<const Parsing::StringExpr>(&parsingExpr);
            return genStringPlainOperand(*stringExpr);
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
        case ExprKind::Subscript: {
            const auto subscriptExpr = dynCast<const Parsing::SubscriptExpr>(&parsingExpr);
            return genSubscriptInst(*subscriptExpr);
        }
        case ExprKind::SizeOfExpr: {
            const auto sizeOfExprExpr = dynCast<const Parsing::SizeOfExprExpr>(&parsingExpr);
            return genSizeOfExprInst(*sizeOfExprExpr);
        }
        case ExprKind::SizeOfType: {
            const auto sizeOfTypeExpr = dynCast<const Parsing::SizeOfTypeExpr>(&parsingExpr);
            return genSizeOfTypeInst(*sizeOfTypeExpr);
        }
        case ExprKind::Dot: {
            const auto dotExpr = dynCast<const Parsing::DotExpr>(&parsingExpr);
            return genDotExprInst(*dotExpr);
        }
        case ExprKind::Arrow: {
            const auto arrowExpr = dynCast<const Parsing::ArrowExpr>(&parsingExpr);
            return genArrowExprInst(*arrowExpr);
        }
    }
    std::abort();
}

const Value* GenerateIr::genInstAndConvert(const Parsing::Expr& parsingExpr)
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
            const Value* dst = genValueVar(dstIden, convertType(dereferencedPointer->referredToType));
            emitLoad(dereferencedPointer->ptr, dst, convertType(dereferencedPointer->referredToType));
            return dst;
        }
        case ExprResult::Kind::SubObject: {
            const auto subObject = dynCast<const SubObject>(result.get());
            const Value* dst = genValueVar(makeTemporaryName(subObject->base.value), convert(*parsingExpr.type));
            emitCopyFromOffset(subObject->base, subObject->referringTo, dst, subObject->offset, dst->type);
            return dst;
        }
    }
    std::unreachable();
}

std::unique_ptr<ExprResult> GenerateIr::genCastInst(const Parsing::CastExpr& castExpr)
{
    const Value* result = genInstAndConvert(*castExpr.innerExpr);
    const IrType towards = convert(*castExpr.type);
    const IrType from = convert(*castExpr.innerExpr->type);
    const Value* dst = castValue(result, towards, from);
    return std::make_unique<PlainOperand>(dst);
}

const Value* GenerateIr::castValue(const Value* result, const IrType towards, const IrType from)
{
    const Value* dst = genValueVar(makeTemporaryName(), towards);
    if (towards == voidType)
        return dst;
    if (towards == doubleType && !isSigned(from))
        emitUIntToDouble(result, dst, towards);
    else if (towards == doubleType && isSigned(from))
        emitIntToDouble(result, dst, towards);
    else if (!isSigned(towards) && from == doubleType)
        emitDoubleToUInt(result, dst, towards);
    else if (isSigned(towards) && from == doubleType)
        emitDoubleToInt(result, dst, towards);
    else if (towards.size == from.size)
        emitCopy(result, dst, towards);
    else if (towards.size < from.size)
        emitTruncate(result, dst, from);
    else if (isSigned(from))
        emitSignExtend(result, dst, towards);
    else
        emitZeroExtend(result, dst, towards);
    return dst;
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryInst(const Parsing::UnaryExpr& unaryExpr)
{
    if (isPostfixOp(unaryExpr.op))
        return genUnaryPostfixInst(unaryExpr);
    if (isPrefixOp(unaryExpr.op))
        return genUnaryPrefixInst(unaryExpr);
    if (unaryExpr.op == Parsing::UnaryExpr::Operator::Plus)
        return genInst(*unaryExpr.innerExpr);
    return genUnaryBasicInst(unaryExpr);
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryBasicInst(const Parsing::UnaryExpr& unaryExpr)
{
    assert(unaryExpr.type->kind == Parsing::TypeBase::Kind::Var);

    const UnaryInst::Operation operation = convertUnaryOperation(unaryExpr.op);
    const Value* src = genInstAndConvert(*unaryExpr.innerExpr);
    const Value* dst = genValueVar(makeTemporaryName(), convert(*unaryExpr.type));

    emitUnary(operation, convert(*unaryExpr.type), src, dst);
    return std::make_unique<PlainOperand>(dst);
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr)
{
    const Value* originalForReturn = genValueVar(makeTemporaryName(), convert(*unaryExpr.type));
    const Value* tempNew = genValueVar(makeTemporaryName(), convert(*unaryExpr.type));
    const auto oper = getPostPrefixOperation(unaryExpr.op);
    const IrType type = convert(*unaryExpr.type);
    const Value* scale = getInrDecScale(unaryExpr, unaryExpr.type->type);
    const std::unique_ptr<ExprResult> original = genInst(*unaryExpr.innerExpr);
    switch (original->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto plainOriginal = dynCast<const PlainOperand>(original.get());
            emitCopy(plainOriginal->value, originalForReturn, type);
            emitBinary(oper, type, originalForReturn, scale, tempNew);
            emitCopy(tempNew, plainOriginal->value, type);
            return std::make_unique<PlainOperand>(originalForReturn);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto derefOriginal = dynCast<const DereferencedPointer>(original.get());
            const Value* derefValue = genValueVar(Identifier(makeTemporaryName()), type);
            emitLoad(derefOriginal->ptr, derefValue, type);
            emitCopy(derefValue, originalForReturn, type);
            emitBinary(oper, type, originalForReturn, scale, tempNew);
            emitStore(tempNew, derefOriginal->ptr, type);
            return std::make_unique<PlainOperand>(originalForReturn);
        }
        case ExprResult::Kind::SubObject:
            std::abort();
    }
    return std::make_unique<PlainOperand>(originalForReturn);
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr)
{
    const IrType type = convert(*unaryExpr.type);
    const Value* scale = getInrDecScale(unaryExpr, unaryExpr.type->type);
    const Value* temp = genValueVar(Identifier(makeTemporaryName()), type);
    const auto operation = getPostPrefixOperation(unaryExpr.op);
    const std::unique_ptr<ExprResult> original = genInst(*unaryExpr.innerExpr);
    switch (original->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto originalPlain = dynCast<const PlainOperand>(original.get());
            emitBinary(operation, type, originalPlain->value, scale, temp);
            emitCopy(temp, originalPlain->value, type);
            return std::make_unique<PlainOperand>(temp);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto derefPtr = dynCast<const DereferencedPointer>(original.get());
            const Value* derefValue = genValueVar(Identifier(makeTemporaryName()), type);
            emitLoad(derefPtr->ptr, derefValue, type);
            emitBinary(operation, type, derefValue, scale, temp);
            emitStore(temp, derefPtr->ptr, type);
            return std::make_unique<PlainOperand>(temp);
        }
        case ExprResult::Kind::SubObject:
            std::abort();
    }
    std::abort();
}

std::unique_ptr<ExprResult> GenerateIr::genVarInst(const Parsing::VarExpr& varExpr)
{
    const Identifier iden(varExpr.name);
    const Value* var = genValueVar(iden, convert(*varExpr.type), varExpr.referringTo);
    return std::make_unique<PlainOperand>(var);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryInst(const Parsing::BinaryExpr& binaryExpr)
{
    if (binaryExpr.lhs->type->type == Type::Pointer || binaryExpr.rhs->type->type == Type::Pointer)
        return genBinaryPtrInst(binaryExpr);
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::And)
        return genBinaryAndInst(binaryExpr);
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::Or)
        return genBinaryOrInst(binaryExpr);
    return genBinarySimpleInst(binaryExpr);
}

std::unique_ptr<ExprResult> GenerateIr::genBinarySimpleInst(const Parsing::BinaryExpr& binaryExpr)
{
    const Value* lhs = genInstAndConvert(*binaryExpr.lhs);
    const Value* rhs = genInstAndConvert(*binaryExpr.rhs);

    const auto dst = genValueVar(makeTemporaryName(), convert(*binaryExpr.type));
    const BinaryInst::Operation operation = convertBinaryOperation(binaryExpr.op);
    emitBinary(operation, convert(*binaryExpr.type), lhs, rhs, dst);
    return std::make_unique<PlainOperand>(dst);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryAndInst(const Parsing::BinaryExpr& binaryExpr)
{
    const Value* result = genValueVar(makeTemporaryName(), convert(*binaryExpr.type));
    const Value* lhs = genInstAndConvert(*binaryExpr.lhs);
    const Identifier falseLabelIden = makeTemporaryName();

    emitJumpIfZero(lhs, falseLabelIden);
    const Value* rhs = genInstAndConvert(*binaryExpr.rhs);
    emitJumpIfZero(rhs, falseLabelIden);
    const Value* oneVal = genConstValue(1);
    emitCopy(oneVal, result, convert(*binaryExpr.type));
    const Identifier endLabelIden = makeTemporaryName();
    emitJump(endLabelIden);
    emitLabel(falseLabelIden);
    const Value* zeroVal = genConstValue(0);
    emitCopy(zeroVal, result, convert(*binaryExpr.type));
    emitLabel(endLabelIden);
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryOrInst(const Parsing::BinaryExpr& binaryExpr)
{
    const Value* result = genValueVar(makeTemporaryName(), convert(*binaryExpr.type));
    const Value* lhs = genInstAndConvert(*binaryExpr.lhs);
    const Identifier trueLabelIden = makeTemporaryName();

    emitJumpIfNotZero(lhs, trueLabelIden);
    const Value* rhs = genInstAndConvert(*binaryExpr.rhs);
    emitJumpIfNotZero(rhs, trueLabelIden);
    const auto zeroVal = genConstValue(0);
    emitCopy(zeroVal, result, convert(*binaryExpr.type));
    const Identifier endLabelIden = makeTemporaryName();
    emitJump(endLabelIden);
    emitLabel(trueLabelIden);
    const Value* oneVal = genConstValue(1);
    emitCopy(oneVal, result, convert(*binaryExpr.type));
    emitLabel(endLabelIden);
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryPtrInst(const Parsing::BinaryExpr& binaryExpr)
{
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::And)
        return genBinaryAndInst(binaryExpr);
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::Or)
        return genBinaryOrInst(binaryExpr);
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::Subtract &&
        binaryExpr.lhs->type->type == Type::Pointer &&
        binaryExpr.rhs->type->type == Type::Pointer)
        return genBinaryPtrSubInst(binaryExpr);
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::Add ||
        binaryExpr.op == Parsing::BinaryExpr::Operator::Subtract)
        return genBinaryPtrAddInst(binaryExpr);
    return genBinarySimpleInst(binaryExpr);
}

void GenerateIr::binaryPtrSubInst(const Value* lhs,
                                  const Value* rhs,
                                  const Value* dst,
                                  const i64 scale)
{
    const Value* diff = genValueVar(makeTemporaryName(), lhs->type);
    emitBinary(BinaryInst::Operation::Subtract, i64Type, lhs, rhs, diff);
    const Value* size = genConstValue(scale);
    emitBinary(BinaryInst::Operation::Divide, i64Type, diff, size, dst);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryPtrSubInst(const Parsing::BinaryExpr& binaryExpr)
{
    const Value* lhs = genInstAndConvert(*binaryExpr.lhs);
    const Value* rhs = genInstAndConvert(*binaryExpr.rhs);
    const Value* dst = genValueVar(makeTemporaryName(), lhs->type);
    const i64 scale = getPointerReferenceTypeSize(binaryExpr.lhs->type.get());
    binaryPtrSubInst(lhs, rhs, dst, scale);
    return std::make_unique<PlainOperand>(dst);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryPtrAddInst(const Parsing::BinaryExpr& binaryExpr)
{
    const Value* ptr = genInstAndConvert(*binaryExpr.lhs);
    const Value* index = genInstAndConvert(*binaryExpr.rhs);
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::Subtract) {
        const auto dst = genValueVar(Identifier(makeTemporaryName()), pointerType);
        emitUnary(UnaryInst::Operation::Negate, pointerType, index, dst);
        index = dst;
    }
    const i64 scale = getPointerReferenceTypeSize(binaryExpr.lhs->type.get());
    const Value* result = genValueVar(makeTemporaryName(), pointerType);
    emitAddPtr(ptr, index, result, scale);
    return std::make_unique<PlainOperand>(result);
}

void GenerateIr::genCompoundAssignWithoutDeref(
    const Parsing::AssignmentExpr& assignmentExpr,
    const Value* rhs,
    const Value* lhs)
{
    auto compoundResult = genValueVar(makeTemporaryName(lhs), lhs->type);
    emitCopy(lhs, compoundResult, lhs->type);
    const BinaryInst::Operation operation = convertBinaryOperation(assignmentExpr.op);
    const Type leftType = assignmentExpr.lhs->type->type;
    const Type rightType = assignmentExpr.rhs->type->type;
    const Type commonType = getCommonType(leftType, rightType);
    if (commonType == Type::Pointer) {
        if (rightType == Type::Pointer && operation == BinaryInst::Operation::Subtract) {
            const i64 scale = getPointerReferenceTypeSize(assignmentExpr.lhs->type.get());
            binaryPtrSubInst(compoundResult, rhs, lhs, scale);
            return;
        }
        if (operation == BinaryInst::Operation::Subtract) {
            const Value* dst = genValueVar(Identifier(makeTemporaryName()), pointerType);
            emitUnary(UnaryInst::Operation::Negate, pointerType, rhs, dst);
            rhs = dst;
        }
        const i64 scale = getPointerReferenceTypeSize(assignmentExpr.lhs->type.get());
        emitAddPtr(compoundResult, rhs, lhs, scale);
        return;
    }
    const IrType irLeftType = convertType(leftType);
    const IrType irRightType = convertType(rightType);
    const IrType irCommonType = convertType(commonType);
    if (commonType != leftType && !isBitShift(assignmentExpr.op))
        compoundResult = castValue(compoundResult, irCommonType, irLeftType);
    if (commonType != rightType && !isBitShift(assignmentExpr.op))
        rhs = castValue(rhs, irCommonType, irRightType);
    if (irCommonType != lhs->type && !isBitShift(assignmentExpr.op)) {
        emitBinary(operation, irCommonType, compoundResult, rhs, compoundResult);
        compoundResult = castValue(compoundResult, lhs->type, irCommonType);
        emitCopy(compoundResult, lhs, lhs->type);
    }
    else
        emitBinary(operation, lhs->type, compoundResult, rhs, lhs);
}

std::unique_ptr<ExprResult> GenerateIr::genAssignInst(const Parsing::AssignmentExpr& assignmentExpr)
{
    const std::unique_ptr<ExprResult> lhs = genInst(*assignmentExpr.lhs);
    const Value* rhs = genInstAndConvert(*assignmentExpr.rhs);
    switch (lhs->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto plainLhs = dynCast<const PlainOperand>(lhs.get());
            if (assignmentExpr.op != Parsing::AssignmentExpr::Operator::Assign)
                genCompoundAssignWithoutDeref(assignmentExpr, rhs, plainLhs->value);
            else
                emitCopy(rhs, plainLhs->value, convert(*assignmentExpr.type));
            return std::make_unique<PlainOperand>(plainLhs->value);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto derefLhs = dynCast<const DereferencedPointer>(lhs.get());
            if (assignmentExpr.op != Parsing::AssignmentExpr::Operator::Assign) {
                const Value* tempLhs = genValueVar(makeTemporaryName(derefLhs->ptr), convert(*assignmentExpr.type));
                emitLoad(derefLhs->ptr, tempLhs, convert(*assignmentExpr.type));
                genCompoundAssignWithoutDeref(assignmentExpr, rhs, tempLhs);
                emitStore(tempLhs, derefLhs->ptr, convert(*assignmentExpr.type));
                return std::make_unique<PlainOperand>(tempLhs);
            }
            emitStore(rhs, derefLhs->ptr, convert(*assignmentExpr.type));
            return std::make_unique<PlainOperand>(derefLhs->ptr);
        }
        case ExprResult::Kind::SubObject: {
            const auto subObj = dynCast<const SubObject>(lhs.get());
            emitCopyToOffset(rhs ,subObj->base, subObj->referringTo, subObj->offset, 0, 0, rhs->type);
            return std::make_unique<PlainOperand>(rhs);
        }
    }
    std::unreachable();
}

std::unique_ptr<ExprResult> GenerateIr::genConstPlainOperand(const Parsing::ConstExpr& constExpr)
{
    const Value* result = genConstValue(constExpr);
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genStringPlainOperand(const Parsing::StringExpr& stringExpr)
{
    const auto it = constStrings.find(stringExpr.value);
    if (it != constStrings.end()) {
        const Value* valueVar = genValueVar(Identifier(it->second), pointerType, ReferringTo::Static);
        return std::make_unique<PlainOperand>(valueVar);
    }
    const Identifier iden = makeTemporaryName("string.");
    constStrings.emplace_hint(it, stringExpr.value, iden.value);
    topLevels.emplace_back(std::make_unique<StaticConstant>(iden, stringExpr.value, false, true));
    const Value* valueVar = genValueVar(iden, pointerType, ReferringTo::Static);
    return std::make_unique<PlainOperand>(valueVar);
}

std::unique_ptr<ExprResult> GenerateIr::genTernaryInst(const Parsing::TernaryExpr& ternaryExpr)
{
    auto result = genValueVar(makeTemporaryName(), convert(*ternaryExpr.type));
    const Identifier endLabelIden = makeTemporaryName();
    const Identifier falseLabelName = makeTemporaryName();
    const auto conditionalExpr = dynCast<const Parsing::TernaryExpr>(&ternaryExpr);

    const Value* condition = genInstAndConvert(*conditionalExpr->condition);
    emitJumpIfZero(condition, falseLabelName);

    const Value* trueValue = genInstAndConvert(*conditionalExpr->trueExpr);
    if (trueValue->type != voidType)
        emitCopy(trueValue, result, trueValue->type);
    emitJump(endLabelIden);

    emitLabel(falseLabelName);
    const Value* falseValue = genInstAndConvert(*conditionalExpr->falseExpr);
    if (falseValue->type != voidType)
        emitCopy(falseValue, result, falseValue->type);

    emitLabel(endLabelIden);
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genFuncCallInst(const Parsing::FuncCallExpr& funcCallExpr)
{
    std::vector<const Value*> arguments;
    arguments.reserve(funcCallExpr.args.size());
    for (const auto& expr : funcCallExpr.args) {
        const Value* arg = genInstAndConvert(*expr);
        arguments.emplace_back(arg);
    }
    if (funcCallExpr.type->type == Type::Void) {
        const Value* dst = genValueVar(makeTemporaryName(), voidType);
        emitFunCall(Identifier(funcCallExpr.name), std::move(arguments), voidType);
        return std::make_unique<PlainOperand>(dst);
    }
    if (funcCallExpr.type->type != Type::Pointer) {
        const auto returnType = dynCast<const Parsing::VarType>(funcCallExpr.type.get());
        const Value* dst = genValueVar(makeTemporaryName(), convert(*funcCallExpr.type));
        emitFunCall(
            Identifier(funcCallExpr.name),
            std::move(arguments),
            dst,
            convert(*returnType));
        return std::make_unique<PlainOperand>(dst);
    }
    auto dst = genValueVar(makeTemporaryName(), convert(*funcCallExpr.type));
    emitFunCall(Identifier(funcCallExpr.name), std::move(arguments), dst, pointerType);
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
            const Value* dst = genValueVar(dstIden, pointerType);
            emitGetAddress(plainOperand->value, dst, pointerType);
            return std::make_unique<PlainOperand>(dst);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto dereferencedPointer = dynCast<const DereferencedPointer>(inner.get());
            return std::make_unique<PlainOperand>(dereferencedPointer->ptr);
        }
        case ExprResult::Kind::SubObject: {
            const auto subObject = dynCast<const SubObject>(inner.get());
            const Value* src = genValueVar(subObject->base, pointerType, subObject->referringTo);
            const Value* dstPtr = genValueVar(makeTemporaryName(), pointerType);
            const Value* dst = genValueVar(makeTemporaryName(), pointerType);
            const Value* constOne = genConstValue(1l);
            emitGetAddress(src, dstPtr, pointerType);
            emitAddPtr(dstPtr, constOne, dst, subObject->offset);
            return std::make_unique<PlainOperand>(dst);
        }
    }
    std::unreachable();
}

i64 GenerateIr::getPointerReferenceTypeSize(const Parsing::TypeBase* typeBase) const
{
    const auto pointerTypeParsing = dynCast<const Parsing::PointerType>(typeBase);
    return typeTable.getSize(pointerTypeParsing->referenced.get());
}

std::unique_ptr<ExprResult> GenerateIr::genSubscriptInst(const Parsing::SubscriptExpr& subscriptExpr)
{
    const Value* ptr = genInstAndConvert(*subscriptExpr.referencing);
    const Value* index = genInstAndConvert(*subscriptExpr.index);
    Parsing::TypeBase* referencedType = subscriptExpr.referencing->type.get();
    const i64 scale = getPointerReferenceTypeSize(referencedType);
    auto result = genValueVar(makeTemporaryName(), pointerType);
    emitAddPtr(ptr, index, result, scale);
    return std::make_unique<DereferencedPointer>(
        result, getSubscriptDereferenceType(subscriptExpr.referencing->type.get()));
}

std::unique_ptr<ExprResult> GenerateIr::genDereferenceInst(const Parsing::DereferenceExpr& dereferenceExpr)
{
    const Value* result = genInstAndConvert(*dereferenceExpr.reference);
    return std::make_unique<DereferencedPointer>(result, dereferenceExpr.type->type);
}

std::unique_ptr<ExprResult> GenerateIr::genSizeOfExprInst(const Parsing::SizeOfExprExpr& sizeOfExprExpr)
{
    if (sizeOfExprExpr.innerExpr->kind == Parsing::Expr::Kind::Constant
        && sizeOfExprExpr.innerExpr->type->kind == Parsing::TypeBase::Kind::Var) {
        const auto varType = dynCast<const Parsing::VarType>(sizeOfExprExpr.innerExpr->type.get());
        if (varType->type == Type::Char) {
            const Value* valueSize = genConstValue(4l);
            return std::make_unique<PlainOperand>(valueSize);
        }
    }
    const i64 size = typeTable.getSize(sizeOfExprExpr.innerExpr->type.get());
    const Value* valueSize = genConstValue(size);
    return std::make_unique<PlainOperand>(valueSize);
}

std::unique_ptr<ExprResult> GenerateIr::genSizeOfTypeInst(const Parsing::SizeOfTypeExpr& sizeOfTypeExpr)
{
    const i64 size = typeTable.getSize(sizeOfTypeExpr.sizeType.get());
    const Value* valueSize = genConstValue(size);
    return std::make_unique<PlainOperand>(valueSize);
}

std::unique_ptr<ExprResult> GenerateIr::genDotExprInst(const Parsing::DotExpr& dotExpr)
{
    const auto structuredType = dynCast<const Parsing::StructuredType>(dotExpr.structuredExpr->type.get());
    const auto entry = typeTable.getEntry(structuredType->identifier);
    const i64 memberOffset = entry->memberMap.find(dotExpr.member)->second.offset;
    const auto result = genInst(*dotExpr.structuredExpr);
    switch (result->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto plain = dynCast<const PlainOperand>(result.get());
            const auto variable = dynCast<const ValueVar>(plain->value);
            return std::make_unique<SubObject>(variable->value, variable->referringTo, memberOffset);
        }
        case ExprResult::Kind::SubObject: {
            const auto subObject = dynCast<SubObject>(result.get());
            return std::make_unique<SubObject>(
                subObject->base, subObject->referringTo, subObject->offset + memberOffset);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto deref = dynCast<const DereferencedPointer>(result.get());
            const Value* dstPtr = genValueVar(makeTemporaryName(), pointerType);
            const Value* index = genConstValue(memberOffset);
            emitAddPtr(deref->ptr, index, dstPtr, 1l);
            return std::make_unique<DereferencedPointer>(dstPtr, dotExpr.type->type);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<ExprResult> GenerateIr::genArrowExprInst(const Parsing::ArrowExpr& arrowExpr)
{
    const auto result = genInstAndConvert(*arrowExpr.pointerExpr);
    const auto pointerTypeRef = dynCast<const Parsing::PointerType>(arrowExpr.pointerExpr->type.get());
    const auto structuredType = dynCast<const Parsing::StructuredType>(pointerTypeRef->referenced.get());
    const auto entry = typeTable.getEntry(structuredType->identifier);
    const i64 memberOffset = entry->memberMap.find(arrowExpr.identifier)->second.offset;
    const Value* dstPtr = genValueVar(makeTemporaryName(), result->type);
    const Value* index = genConstValue(memberOffset);
    emitAddPtr(result, index, dstPtr, 1l);
    return std::make_unique<DereferencedPointer>(dstPtr, arrowExpr.type->type);
}

const Value* GenerateIr::getInrDecScale(const Parsing::UnaryExpr& unaryExpr, const Type type)
{
    if (type == Type::Pointer) {
        const i64 size = getPointerReferenceTypeSize(unaryExpr.innerExpr->type.get());
        values.emplace_back(std::make_unique<ValueConst>(size));
    }
    else if (type == Type::Double)
        values.emplace_back(std::make_unique<ValueConst>(1.0));
    else
        values.emplace_back(std::make_unique<ValueConst>(1));
    return values.back().get();
}

Identifier makeTemporaryName()
{
    return makeTemporaryName("");
}

Identifier makeTemporaryName(const Value* value)
{
    if (value->kind == Value::Kind::Variable) {
        const auto val = dynCast<const ValueVar>(value);
        return makeTemporaryName(val->value.value);
    }
    return makeTemporaryName("");
}

Identifier makeTemporaryName(const std::string& name)
{
    static i64 id = 0;
    std::string prefix = name;
    prefix += '.';
    prefix += std::to_string(id++);
    return {prefix};
}

static std::string generateCaseLabelName(std::string before)
{
    std::ranges::replace(before, '-', '_');
    return before;
}

const Value* GenerateIr::genZeroValueForType(const Type type)
{
    switch (type) {
        case Type::I8:      return genConstValue(static_cast<char>(0));
        case Type::U8:      return genConstValue(static_cast<u8>(0u));
        case Type::Char:    return genConstValue(static_cast<char>(0u));
        case Type::I32:     return genConstValue(static_cast<i32>(0));
        case Type::U32:     return genConstValue(0u);
        case Type::I64:     return genConstValue(static_cast<i64>(0l));
        case Type::U64:     return genConstValue(static_cast<u64>(0ul));
        case Type::Pointer: return genConstValue(static_cast<u64>(0ul));
        case Type::Double:  return genConstValue(0.0);
        default:
            std::abort();
    }
}

const Value* GenerateIr::genValueVar(const Identifier& iden, const IrType& type)
{
    values.emplace_back(std::make_unique<ValueVar>(iden, type));
    return values.back().get();
}

const Value* GenerateIr::genValueVar(const Identifier& iden, const IrType& type, const ReferringTo referringTo)
{
    values.emplace_back(std::make_unique<ValueVar>(iden, type, referringTo));
    return values.back().get();
}

const Value* GenerateIr::genConstValue(const i8 constValue)
{
    values.emplace_back(std::make_unique<ValueConst>(constValue));
    return values.back().get();
}

const Value* GenerateIr::genConstValue(const u8 constValue)
{
    values.emplace_back(std::make_unique<ValueConst>(constValue));
    return values.back().get();
}

const Value* GenerateIr::genConstValue(const char constValue)
{
    values.emplace_back(std::make_unique<ValueConst>(constValue));
    return values.back().get();
}

const Value* GenerateIr::genConstValue(const i32 constValue)
{
    values.emplace_back(std::make_unique<ValueConst>(constValue));
    return values.back().get();
}

const Value* GenerateIr::genConstValue(const u32 constValue)
{
    values.emplace_back(std::make_unique<ValueConst>(constValue));
    return values.back().get();
}

const Value* GenerateIr::genConstValue(const i64 constValue)
{
    values.emplace_back(std::make_unique<ValueConst>(constValue));
    return values.back().get();
}

const Value* GenerateIr::genConstValue(const u64 constValue)
{
    values.emplace_back(std::make_unique<ValueConst>(constValue));
    return values.back().get();
}

const Value* GenerateIr::genConstValue(const double constValue)
{
    values.emplace_back(std::make_unique<ValueConst>(constValue));
    return values.back().get();
}

Type getSubscriptDereferenceType(Parsing::TypeBase* typeBase)
{
    switch (typeBase->kind) {
        case Parsing::TypeBase::Kind::Pointer: {
            const auto ptrType = dynCast<Parsing::PointerType>(typeBase);
            return ptrType->referenced->type;
        }
        case Parsing::TypeBase::Kind::Array: {
            const auto arrayType = dynCast<Parsing::ArrayType>(typeBase);
            return arrayType->elementType->type;
        }
        default:
            std::abort();
    }
}

const Value* GenerateIr::genConstValue(const Parsing::ConstExpr& constExpr)
{
    switch (constExpr.type->type) {
        case Type::I8: {
            values.emplace_back(std::make_unique<ValueConst>(std::get<i8>(constExpr.value)));
            break;
        }
        case Type::U8: {
            values.emplace_back(std::make_unique<ValueConst>(std::get<u8>(constExpr.value)));
            break;
        }
        case Type::Char: {
            values.emplace_back(std::make_unique<ValueConst>(std::get<char>(constExpr.value)));
            break;
        }
        case Type::I32: {
            values.emplace_back(std::make_unique<ValueConst>(std::get<i32>(constExpr.value)));
            break;
        }
        case Type::U32: {
            values.emplace_back(std::make_unique<ValueConst>(std::get<u32>(constExpr.value)));
            break;
        }
        case Type::I64: {
            values.emplace_back(std::make_unique<ValueConst>(std::get<i64>(constExpr.value)));
            break;
        }
        case Type::U64: {
            values.emplace_back(std::make_unique<ValueConst>(std::get<u64>(constExpr.value)));
            break;
        }
        case Type::Double: {
            values.emplace_back(std::make_unique<ValueConst>(std::get<double>(constExpr.value)));
            break;
        }
        default:
            std::abort();
    }
    return values.back().get();
}
} // IR