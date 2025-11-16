#include "ASTParser.hpp"
#include "GenerateIr.hpp"
#include "Types/TypeConversion.hpp"
#include "ASTTypes.hpp"
#include "AstToIrOperators.hpp"
#include "DynCast.hpp"

#include <algorithm>
#include <cassert>
#include <strings.h>

namespace Ir {
static Identifier makeTemporaryName();
static Identifier makeTemporaryName(Value& value);
static Identifier makeTemporaryName(const std::string& name);
static std::string generateCaseLabelName(std::string before);
static std::shared_ptr<Value> genConstValue(const Parsing::ConstExpr& constExpr);
static std::shared_ptr<Value> genZeroValueForType(Type type);

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
            const auto funcDeclaration = dynCast<const Parsing::FuncDeclaration>(&decl);
            if (funcDeclaration->body == nullptr)
                return nullptr;
            return functionIr(*funcDeclaration);
        }
        case Kind::VarDecl: {
            const auto varDecl = dynCast<const Parsing::VarDecl>(&decl);
            return staticVariableIr(*varDecl);
        }
        default:
            std::abort();
    }
}

void GenerateIr::allocateLocalArrayWithoutInitializer(const Parsing::VarDecl& varDecl)
{
    const i64 size = getArraySize(varDecl.type.get());
    const Type type = getArrayType(varDecl.type.get());
    emplaceAllocate(size, varDecl.name, type);
}

void GenerateIr::directlyPushConstant32Bit(const Parsing::VarDecl& varDecl, const std::shared_ptr<Value>& value)
{
    const Identifier iden(varDecl.name);
    const auto var = std::make_shared<ValueVar>(iden, varDecl.type->type);
    emplaceCopy(value, var, varDecl.type->type);
}

void GenerateIr::genDeclaration(const Parsing::Declaration& decl)
{
    using InitKind = Parsing::Initializer::Kind;

    if (decl.kind != Parsing::Declaration::Kind::VarDecl)
        return;
    const auto varDecl = dynCast<const Parsing::VarDecl>(&decl);
    if (varDecl->storage == Storage::Static)
        return genStaticLocal(*varDecl);
    if (varDecl->init == nullptr && varDecl->type->type == Type::Array) {
        allocateLocalArrayWithoutInitializer(*varDecl);
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
    const std::shared_ptr<Value> value = genInstAndConvert(*singleInit->expr);
    if (value->kind == Value::Kind::Constant &&
        (varDecl.type->type == Type::I32 || varDecl.type->type == Type::U32)) {
        directlyPushConstant32Bit(varDecl, value);
        return;
    }
    const auto temporary = std::make_shared<ValueVar>(makeTemporaryName(), varDecl.type->type);
    emplaceCopy(value, temporary, varDecl.type->type);
    const Identifier iden(varDecl.name);
    const auto var = std::make_shared<ValueVar>(iden, varDecl.type->type);
    emplaceCopy(temporary, var, varDecl.type->type);
}

void GenerateIr::genZeroLocalInit(const std::string& name,
                                  const Type type,
                                  const i64 arraySize,
                                  const i64 alignment,
                                  const i64 lengthZeroInit,
                                  i64& offset,
                                  const std::shared_ptr<Value>& zeroConst)
{
    const i64 typeSize = getTypeSize(type);
    for (size_t i = 0; i < lengthZeroInit; ++i) {
        emplaceCopyToOffset(
            zeroConst, Identifier(name), offset, arraySize, alignment, type);
        offset += typeSize;
    }
}

void GenerateIr::genSingleLocalInit(const std::string& name,
                                    const Type type,
                                    const i64 arraySize,
                                    const i64 alignment,
                                    i64& offset,
                                    const Parsing::SingleInitializer& singleInit)
{
    const i64 typeSize = getTypeSize(type);
    std::shared_ptr<Value> value = genInstAndConvert(*singleInit.expr);
    if (singleInit.expr->kind == Parsing::Expr::Kind::String) {
        const std::shared_ptr<ValueVar> var = std::make_shared<ValueVar>(makeTemporaryName(), Type::Pointer);
        emplaceGetAddress(value, var, Type::Pointer);
        value = var;
    }
    emplaceCopyToOffset(value, Identifier(name), offset, arraySize, alignment, type);
    offset += typeSize;
}

void GenerateIr::genCompoundLocalInit(const Parsing::VarDecl& varDecl)
{
    const auto compoundInit = dynCast<Parsing::CompoundInitializer>(varDecl.init.get());
    const auto arrayType = dynCast<Parsing::ArrayType>(varDecl.type.get());
    const Type type = getArrayType(varDecl.type.get());
    const i64 arraySize = getArraySize(arrayType);
    const i64 alignment = getArrayAlignment(arraySize, type);
    i64 offset = 0;
    const auto zeroConst = genZeroValueForType(type);
    for (const auto& init : compoundInit->initializers) {
        switch (init->kind) {
            case Parsing::Initializer::Kind::Single: {
                const auto singleInit = dynCast<Parsing::SingleInitializer>(init.get());
                genSingleLocalInit(varDecl.name, type, arraySize, alignment, offset, *singleInit);
                break;
            }
            case Parsing::Initializer::Kind::Zero: {
                const auto zeroInit = dynCast<Parsing::ZeroInitializer>(init.get());
                genZeroLocalInit(varDecl.name,
                                 type,
                                 arraySize,
                                 alignment,
                                 zeroInit->size,
                                 offset,
                                 zeroConst);
                break;
            }
            default:
                std::abort();
        }
    }
}

void GenerateIr::genStaticLocal(const Parsing::VarDecl& varDecl)
{
    const bool defined = varDecl.init != nullptr;
    auto variable = genStaticInit(varDecl, defined);
    m_topLevels.emplace_back(std::move(variable));
    m_symbolTable.addEntry(varDecl.name,
                           varDecl.name,
                           *varDecl.type,
                           true, false, false, defined);
}

std::unique_ptr<TopLevel> GenerateIr::staticVariableIr(const Parsing::VarDecl& varDecl)
{
    const auto entry = m_symbolTable.lookup(varDecl.name);
    const bool defined = entry.isDefined();

    if (defined && varDecl.init == nullptr)
        return nullptr;
    if (!defined && varDecl.storage == Storage::Extern)
        return nullptr;
    if (m_writtenGlobals.contains(varDecl.name))
        return nullptr;

    m_writtenGlobals.insert(varDecl.name);
    if (varDecl.type->kind == Parsing::TypeBase::Kind::Array)
        return genStaticArray(varDecl, defined);

    const std::shared_ptr<Value> value = genStaticVariableInit(varDecl, defined);
    auto variable = std::make_unique<StaticVariable>(
        varDecl.name, value, varDecl.type->type, varDecl.storage != Storage::Static);
    return variable;
}

std::unique_ptr<TopLevel> GenerateIr::genStaticArray(const Parsing::VarDecl& varDecl, const bool defined)
{
    const Type innerType = getArrayType(varDecl.type.get());
    std::vector<std::unique_ptr<Initializer>> initializers = genStaticArrayInit(varDecl, defined);
    auto variable = std::make_unique<StaticArray>(
        varDecl.name, std::move(initializers), innerType, varDecl.storage != Storage::Static);
    return variable;
}

std::vector<std::unique_ptr<Initializer>> GenerateIr::genStaticArrayInit(
        const Parsing::VarDecl& varDecl, const bool defined)
{
    std::vector<std::unique_ptr<Initializer>> initializers;
    if (!defined) {
        const i64 size = getArraySize(varDecl.type.get());
        initializers.emplace_back(std::make_unique<ZeroInitializer>(size));
        return initializers;
    }
    const auto compoundInit = dynCast<Parsing::CompoundInitializer>(varDecl.init.get());
    for (const auto& stuff : compoundInit->initializers) {
        switch (stuff->kind) {
            case Parsing::Initializer::Kind::Single: {
                const auto singleInit = dynCast<Parsing::SingleInitializer>(stuff.get());
                const auto value = genInstAndConvert(*singleInit->expr);
                initializers.emplace_back(std::make_unique<ValueInitializer>(value));
                break;
            }
            case Parsing::Initializer::Kind::Zero: {
                const auto zeroInit = dynCast<Parsing::ZeroInitializer>(stuff.get());
                initializers.emplace_back(std::make_unique<ZeroInitializer>(zeroInit->size));
                break;
            }
            case Parsing::Initializer::Kind::String: {
                std::abort();
            }
            default:
                std::abort();
        }
    }
    return initializers;
}

std::unique_ptr<TopLevel> GenerateIr::genStaticInit(const Parsing::VarDecl& varDecl, const bool defined)
{
    if (varDecl.type->type == Type::Array) {
        const Type innerType = getArrayType(varDecl.type.get());
        auto initializers = genStaticArrayInit(varDecl, defined);
        return std::make_unique<StaticArray>(varDecl.name, std::move(initializers), innerType, false);
    }
    std::shared_ptr<Value> value = genStaticVariableInit(varDecl, defined);
    return std::make_unique<StaticVariable>(varDecl.name, value, varDecl.type->type, false);
}

std::shared_ptr<Value> GenerateIr::genStaticVariableInit(const Parsing::VarDecl& varDecl, const bool defined)
{
    if (defined) {
        const auto singleInit = dynCast<Parsing::SingleInitializer>(varDecl.init.get());
        return genInstAndConvert(*singleInit->expr);
    }
    return genZeroValueForType(varDecl.type->type);
}

std::shared_ptr<Value> genZeroValueForType(const Type type)
{
    switch (type) {
        case Type::Char:    return std::make_shared<ValueConst>(static_cast<char>(0));
        case Type::I8:      return std::make_shared<ValueConst>(static_cast<i8>(0));
        case Type::U8:      return std::make_shared<ValueConst>(static_cast<u8>(0));
        case Type::I32:     return std::make_shared<ValueConst>(0);
        case Type::U32:     return std::make_shared<ValueConst>(0u);
        case Type::I64:     return std::make_shared<ValueConst>(static_cast<i64>(0l));
        case Type::U64:     return std::make_shared<ValueConst>(static_cast<u64>(0ul));
        case Type::Pointer: return std::make_shared<ValueConst>(static_cast<u64>(0ul));
        case Type::Double:  return std::make_shared<ValueConst>(0.0);
        default:
            abort();
    }
}

std::unique_ptr<TopLevel> GenerateIr::functionIr(const Parsing::FuncDeclaration& parsingFunction)
{
    bool global = !m_symbolTable.lookup(parsingFunction.name).hasInternalLinkage();
    auto functionTacky = std::make_unique<Function>(parsingFunction.name, global);
    m_global = true;
    m_insts = std::move(functionTacky->insts);
    functionTacky->args.reserve(parsingFunction.params.size());
    functionTacky->argTypes.reserve(parsingFunction.params.size());
    const auto funcType = dynCast<const Parsing::FuncType>(parsingFunction.type.get());
    for (size_t i = 0; i < parsingFunction.params.size(); ++i) {
        functionTacky->args.emplace_back(Identifier(parsingFunction.params[i]));
        functionTacky->argTypes.emplace_back(funcType->params[i]->type);
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
    const std::shared_ptr<Value> condition = genInstAndConvert(*ifStmt.condition);
    const Identifier endLabelIden = makeTemporaryName();

    emplaceJumpIfZero(condition, endLabelIden);
    genStmt(*ifStmt.thenStmt);
    emplaceLabel(endLabelIden);
}

void GenerateIr::genIfElseStmt(const Parsing::IfStmt& ifStmt)
{
    const std::shared_ptr<Value> condition = genInstAndConvert(*ifStmt.condition);
    const Identifier elseStmtLabel = makeTemporaryName();
    const Identifier endLabelIden = makeTemporaryName();

    emplaceJumpIfZero(condition, elseStmtLabel);
    genStmt(*ifStmt.thenStmt);
    emplaceJump(endLabelIden);
    emplaceLabel(elseStmtLabel);
    genStmt(*ifStmt.elseStmt);
    emplaceLabel(endLabelIden);
}

void GenerateIr::genReturnStmt(const Parsing::ReturnStmt& returnStmt)
{
    const std::shared_ptr<Value> value = genInstAndConvert(*returnStmt.expr);
    if (value == nullptr)
        return;
    emplaceReturn(value, value->type);
}

void GenerateIr::genGotoStmt(const Parsing::GotoStmt& gotoStmt)
{
    emplaceJump(Identifier(gotoStmt.identifier + ".label"));
}

void GenerateIr::genCompoundStmt(const Parsing::CompoundStmt& compoundStmt)
{
    genBlock(*compoundStmt.block);
}

void GenerateIr::genBreakStmt(const Parsing::BreakStmt& breakStmt)
{
    emplaceJump(Identifier(breakStmt.identifier + "break"));
}

void GenerateIr::genContinueStmt(const Parsing::ContinueStmt& continueStmt)
{
    emplaceJump(Identifier(continueStmt.identifier + "continue"));
}

void GenerateIr::genLabelStmt(const Parsing::LabelStmt& labelStmt)
{
    emplaceLabel(Identifier(labelStmt.identifier + ".label"));
    genStmt(*labelStmt.stmt);
}

void GenerateIr::genCaseStmt(const Parsing::CaseStmt& caseStmt)
{
    emplaceLabel(
        Identifier(generateCaseLabelName(caseStmt.identifier)));
    genStmt(*caseStmt.body);
}

void GenerateIr::genDefaultStmt(const Parsing::DefaultStmt& defaultStmt)
{
    emplaceLabel(Identifier(defaultStmt.identifier + "default"));
    genStmt(*defaultStmt.body);
}

void GenerateIr::genDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt)
{
    emplaceLabel(Identifier(doWhileStmt.identifier + "start"));
    genStmt(*doWhileStmt.body);
    emplaceLabel(Identifier(doWhileStmt.identifier + "continue"));
    const std::shared_ptr<Value> condition = genInstAndConvert(*doWhileStmt.condition);
    emplaceJumpIfNotZero(condition, Identifier(doWhileStmt.identifier + "start"));
    emplaceLabel(Identifier(doWhileStmt.identifier + "break"));
}

void GenerateIr::genWhileStmt(const Parsing::WhileStmt& whileStmt)
{
    const auto continueIden = Identifier(whileStmt.identifier + "continue");
    const auto breakIden = Identifier(whileStmt.identifier + "break");

    emplaceLabel(continueIden);
    const auto condition = genInstAndConvert(*whileStmt.condition);
    emplaceJumpIfZero(condition, breakIden);
    genStmt(*whileStmt.body);
    emplaceJump(continueIden);
    emplaceLabel(breakIden);
}

void GenerateIr::genForStmt(const Parsing::ForStmt& forStmt)
{
    if (forStmt.init)
        genForInit(*forStmt.init);
    emplaceLabel(Identifier(forStmt.identifier + "start"));
    if (forStmt.condition) {
        const auto condition = genInstAndConvert(*forStmt.condition);
        emplaceJumpIfZero(condition, Identifier(forStmt.identifier + "break"));
    }
    genStmt(*forStmt.body);
    emplaceLabel(Identifier(forStmt.identifier + "continue"));
    if (forStmt.post)
        genInst(*forStmt.post);
    emplaceJump(Identifier(forStmt.identifier + "start"));
    emplaceLabel(Identifier(forStmt.identifier + "break"));
}

void GenerateIr::genSwitchStmt(const Parsing::SwitchStmt& stmt)
{
    const std::shared_ptr<Value> realValue = genInstAndConvert(*stmt.condition);
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
        emplaceBinary(BinaryInst::Operation::Equal, realValue, src2, dst, realValue->type);
        emplaceJumpIfNotZero(dst, Identifier(caseLabelName));
    }
    if (stmt.hasDefault)
        emplaceJump(Identifier(stmt.identifier + "default"));
    else
        emplaceJump(Identifier(stmt.identifier + "break"));
    genStmt(*stmt.body);
    emplaceLabel(Identifier(stmt.identifier + "break"));
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
            return genSubscript(*subscriptExpr);
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
            emplaceLoad(dereferencedPointer->ptr, dst, dereferencedPointer->referredToType);
            return dst;
        }
    }
    std::unreachable();
}

std::unique_ptr<ExprResult> GenerateIr::genCastInst(const Parsing::CastExpr& castExpr)
{
    const std::shared_ptr<Value> result = genInstAndConvert(*castExpr.innerExpr);
    const Type type = castExpr.type->type;
    const Type innerType = castExpr.innerExpr->type->type;
    auto dst = castValue(result, type, innerType);
    return std::make_unique<PlainOperand>(dst);
}

std::shared_ptr<ValueVar> GenerateIr::castValue(
    const std::shared_ptr<Value>& result, const Type towards, const Type from)
{
    auto dst = std::make_shared<ValueVar>(makeTemporaryName(), towards);
    if (towards == Type::Double && !isSigned(from))
        emplaceUIntToDouble(result, dst, towards);
    else if (towards == Type::Double && isSigned(from))
        emplaceIntToDouble(result, dst, towards);
    else if (!isSigned(towards) && from == Type::Double)
        emplaceDoubleToUInt(result, dst, towards);
    else if (isSigned(towards) && from == Type::Double)
        emplaceDoubleToInt(result, dst, towards);
    else if (getTypeSize(towards) == getTypeSize(from))
        emplaceCopy(result, dst, towards);
    else if (getTypeSize(towards) < getTypeSize(from))
        emplaceTruncate(result, dst, from);
    else if (isSigned(from))
        emplaceSignExtend(result, dst, towards);
    else
        emplaceZeroExtend(result, dst, towards);
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
    const UnaryInst::Operation operation = convertUnaryOperation(unaryExpr.op);
    const std::shared_ptr<Value> src = genInstAndConvert(*unaryExpr.innerExpr);
    auto dst = std::make_shared<ValueVar>(makeTemporaryName(), unaryExpr.type->type);

    emplaceUnary(operation, src, dst, unaryExpr.type->type);
    return std::make_unique<PlainOperand>(dst);
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr)
{
    auto originalForReturn = std::make_shared<ValueVar>(makeTemporaryName(), unaryExpr.type->type);
    const auto tempNew = std::make_shared<ValueVar>(makeTemporaryName(), unaryExpr.type->type);
    const auto oper = getPostPrefixOperation(unaryExpr.op);
    const Type type = unaryExpr.type->type;
    const std::shared_ptr<ValueConst> scale = getInrDecScale(unaryExpr, type);
    const std::shared_ptr original = genInst(*unaryExpr.innerExpr);
    switch (original->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto plainOriginal = dynCast<const PlainOperand>(original.get());
            emplaceCopy(plainOriginal->value, originalForReturn, type);
            emplaceBinary(oper, originalForReturn, scale, tempNew, type);
            emplaceCopy(tempNew, plainOriginal->value, type);
            return std::make_unique<PlainOperand>(originalForReturn);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto derefOriginal = dynCast<const DereferencedPointer>(original.get());
            const auto derefValue = std::make_shared<ValueVar>(Identifier(makeTemporaryName()), type);
            emplaceLoad(derefOriginal->ptr, derefValue, type);
            emplaceCopy(derefValue, originalForReturn, type);
            emplaceBinary(oper, originalForReturn, scale, tempNew, type);
            emplaceStore(tempNew, derefOriginal->ptr, type);
            return std::make_unique<PlainOperand>(originalForReturn);
        }
    }
    return std::make_unique<PlainOperand>(originalForReturn);
}

std::unique_ptr<ExprResult> GenerateIr::genUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr)
{
    const Type type = unaryExpr.type->type;
    const std::shared_ptr<ValueConst> scale = getInrDecScale(unaryExpr, type);
    const auto temp = std::make_shared<ValueVar>(Identifier(makeTemporaryName()), type);
    const auto operation = getPostPrefixOperation(unaryExpr.op);
    const std::unique_ptr<ExprResult> original = genInst(*unaryExpr.innerExpr);
    switch (original->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto originalPlain = dynCast<const PlainOperand>(original.get());
            emplaceBinary(operation, originalPlain->value, scale, temp, type);
            emplaceCopy(temp, originalPlain->value, type);
            return std::make_unique<PlainOperand>(temp);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto derefPtr = dynCast<const DereferencedPointer>(original.get());
            const auto derefValue = std::make_shared<ValueVar>(Identifier(makeTemporaryName()), type);
            emplaceLoad(derefPtr->ptr, derefValue, type);
            emplaceBinary(operation, derefValue, scale, temp, type);
            emplaceStore(temp, derefPtr->ptr, type);
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
    if (var->type == Type::Array) {
        var->size = getArraySize(varExpr.type.get());
        var->type = Type::Pointer;
    }
    return std::make_unique<PlainOperand>(var);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryInst(const Parsing::BinaryExpr& binaryExpr)
{
    if (binaryExpr.lhs->type->type == Type::Pointer || binaryExpr.lhs->type->type == Type::Pointer)
        return genBinaryPtrInst(binaryExpr);
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::And)
        return genBinaryAndInst(binaryExpr);
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::Or)
        return genBinaryOrInst(binaryExpr);
    return genBinarySimpleInst(binaryExpr);
}

std::unique_ptr<ExprResult> GenerateIr::genBinarySimpleInst(const Parsing::BinaryExpr& binaryExpr)
{
    const std::shared_ptr<Value> lhs = genInstAndConvert(*binaryExpr.lhs);
    const std::shared_ptr<Value> rhs = genInstAndConvert(*binaryExpr.rhs);

    auto dst = std::make_shared<ValueVar>(makeTemporaryName(), binaryExpr.type->type);
    const BinaryInst::Operation operation = convertBinaryOperation(binaryExpr.op);
    emplaceBinary(operation, lhs, rhs, dst, binaryExpr.type->type);
    return std::make_unique<PlainOperand>(dst);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryAndInst(const Parsing::BinaryExpr& binaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), binaryExpr.type->type);
    const std::shared_ptr<Value> lhs = genInstAndConvert(*binaryExpr.lhs);
    const Identifier falseLabelIden = makeTemporaryName();

    emplaceJumpIfZero(lhs, falseLabelIden);
    const std::shared_ptr<Value> rhs = genInstAndConvert(*binaryExpr.rhs);
    emplaceJumpIfZero(rhs, falseLabelIden);
    const auto oneVal = std::make_shared<ValueConst>(1);
    emplaceCopy(oneVal, result, binaryExpr.type->type);
    const Identifier endLabelIden = makeTemporaryName();
    emplaceJump(endLabelIden);
    emplaceLabel(falseLabelIden);
    const auto zeroVal = std::make_shared<ValueConst>(0);
    emplaceCopy(zeroVal, result, binaryExpr.type->type);
    emplaceLabel(endLabelIden);
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryOrInst(const Parsing::BinaryExpr& binaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), binaryExpr.type->type);
    const std::shared_ptr<Value> lhs = genInstAndConvert(*binaryExpr.lhs);
    const Identifier trueLabelIden = makeTemporaryName();

    emplaceJumpIfNotZero(lhs, trueLabelIden);
    const std::shared_ptr<Value> rhs = genInstAndConvert(*binaryExpr.rhs);
    emplaceJumpIfNotZero(rhs, trueLabelIden);
    const auto zeroVal = std::make_shared<ValueConst>(0);
    emplaceCopy(zeroVal, result, binaryExpr.type->type);
    const Identifier endLabelIden = makeTemporaryName();
    emplaceJump(endLabelIden);
    emplaceLabel(trueLabelIden);
    const auto oneVal = std::make_shared<ValueConst>(1);
    emplaceCopy(oneVal, result, binaryExpr.type->type);
    emplaceLabel(endLabelIden);
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

void GenerateIr::binaryPtrSubInst(const std::shared_ptr<Value>& lhs,
                                  const std::shared_ptr<Value>& rhs,
                                  const std::shared_ptr<Value>& dst,
                                  const i64 scale)
{
    const auto diff = std::make_shared<ValueVar>(makeTemporaryName(), lhs->type);
    emplaceBinary(BinaryInst::Operation::Subtract, lhs, rhs, diff, Type::I64);
    const auto size = std::make_shared<ValueConst>(scale);
    emplaceBinary(BinaryInst::Operation::Divide, diff, size, dst, Type::I64);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryPtrSubInst(const Parsing::BinaryExpr& binaryExpr)
{
    const std::shared_ptr<Value> lhs = genInstAndConvert(*binaryExpr.lhs);
    const std::shared_ptr<Value> rhs = genInstAndConvert(*binaryExpr.rhs);
    auto dst = std::make_shared<ValueVar>(makeTemporaryName(), lhs->type);
    const i64 scale = getReferencedTypeSize(binaryExpr.lhs->type.get());
    binaryPtrSubInst(lhs, rhs, dst, scale);
    return std::make_unique<PlainOperand>(dst);
}

std::unique_ptr<ExprResult> GenerateIr::genBinaryPtrAddInst(const Parsing::BinaryExpr& binaryExpr)
{
    const std::shared_ptr<Value> ptr = genInstAndConvert(*binaryExpr.lhs);
    std::shared_ptr<Value> index = genInstAndConvert(*binaryExpr.rhs);
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::Subtract) {
        const auto dst = std::make_shared<ValueVar>(Identifier(makeTemporaryName()), Type::Pointer);
        emplaceUnary(UnaryInst::Operation::Negate, index, dst, Type::Pointer);
        index = dst;
    }
    const i64 scale = getReferencedTypeSize(binaryExpr.lhs->type.get());
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), Type::Pointer);
    emplaceAddPtr(ptr, index, result, scale);
    return std::make_unique<PlainOperand>(result);
}

void GenerateIr::genCompoundAssignWithoutDeref(
    const Parsing::AssignmentExpr& assignmentExpr, std::shared_ptr<Value>& rhs, const std::shared_ptr<Value>& lhs)
{
    auto temp = std::make_shared<ValueVar>(makeTemporaryName(*lhs), lhs->type);
    emplaceCopy(lhs, temp, lhs->type);
    const BinaryInst::Operation operation = convertBinaryOperation(assignmentExpr.op);
    const Type leftType = lhs->type;
    const Type rightType = rhs->type;
    const Type commonType = getCommonType(leftType, rightType);
    if (commonType == Type::Pointer) {
        if (rightType == Type::Pointer && operation == BinaryInst::Operation::Subtract) {
            const i64 scale = getReferencedTypeSize(assignmentExpr.lhs->type.get());
            binaryPtrSubInst(temp, rhs, lhs, scale);
            return;
        }
        if (operation == BinaryInst::Operation::Subtract) {
            const auto dst = std::make_shared<ValueVar>(Identifier(makeTemporaryName()), Type::Pointer);
            emplaceUnary(UnaryInst::Operation::Negate, rhs, dst, Type::Pointer);
            rhs = dst;
        }
        const i64 scale = getReferencedTypeSize(assignmentExpr.lhs->type.get());
        emplaceAddPtr(temp, rhs, lhs, scale);
        return;
    }
    if (commonType != leftType && !isBitShift(assignmentExpr.op))
        temp = castValue(temp, commonType, leftType);
    if (commonType != rightType && !isBitShift(assignmentExpr.op))
        rhs = castValue(rhs, commonType, rightType);
    if (commonType != lhs->type && !isBitShift(assignmentExpr.op)) {
        emplaceBinary(operation, temp, rhs, temp, commonType);
        temp = castValue(temp, lhs->type, commonType);
        emplaceCopy(temp, lhs, lhs->type);
    }
    else
        emplaceBinary(operation, temp, rhs, lhs, lhs->type);
}

std::unique_ptr<ExprResult> GenerateIr::genAssignInst(const Parsing::AssignmentExpr& assignmentExpr)
{
    const std::unique_ptr<ExprResult> lhs = genInst(*assignmentExpr.lhs);
    std::shared_ptr<Value> rhs = genInstAndConvert(*assignmentExpr.rhs);
    switch (lhs->kind) {
        case ExprResult::Kind::PlainOperand: {
            const auto plainLhs = dynCast<const PlainOperand>(lhs.get());
            if (assignmentExpr.op != Parsing::AssignmentExpr::Operator::Assign) {
                genCompoundAssignWithoutDeref(assignmentExpr, rhs, plainLhs->value);
                return std::make_unique<PlainOperand>(plainLhs->value);
            }
            emplaceCopy(rhs, plainLhs->value, assignmentExpr.type->type);
            return std::make_unique<PlainOperand>(plainLhs->value);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto derefLhs = dynCast<const DereferencedPointer>(lhs.get());
            if (assignmentExpr.op != Parsing::AssignmentExpr::Operator::Assign) {
                auto tempLhs = std::make_shared<ValueVar>(
                    makeTemporaryName(*derefLhs->ptr), assignmentExpr.type->type);
                emplaceLoad(derefLhs->ptr, tempLhs, assignmentExpr.type->type);
                genCompoundAssignWithoutDeref(assignmentExpr, rhs, tempLhs);
                emplaceStore(tempLhs, derefLhs->ptr, assignmentExpr.type->type);
                return std::make_unique<PlainOperand>(tempLhs);
            }
            emplaceStore(rhs, derefLhs->ptr, assignmentExpr.type->type);
            return std::make_unique<PlainOperand>(derefLhs->ptr);
        }
    }
    std::unreachable();
}

std::unique_ptr<ExprResult> GenerateIr::genConstPlainOperand(const Parsing::ConstExpr& constExpr)
{
    const std::shared_ptr<Value> result = genConstValue(constExpr);
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genStringPlainOperand(const Parsing::StringExpr& stringExpr)
{
    const Identifier iden = makeTemporaryName("string.");
    m_topLevels.emplace_back(std::make_unique<StaticConstant>(iden, stringExpr.value, false, true));
    auto valueVar = std::make_shared<ValueVar>(iden, Type::Pointer);
    valueVar->referingTo = ReferingTo::Static;
    return std::make_unique<PlainOperand>(valueVar);
}

std::shared_ptr<Value> genConstValue(const Parsing::ConstExpr& constExpr)
{
    switch (constExpr.type->type) {
        case Type::I8:          return std::make_shared<ValueConst>(std::get<i8>(constExpr.value));
        case Type::U8:          return std::make_shared<ValueConst>(std::get<u8>(constExpr.value));
        case Type::Char:        return std::make_shared<ValueConst>(std::get<char>(constExpr.value));
        case Type::I32:         return std::make_shared<ValueConst>(std::get<i32>(constExpr.value));
        case Type::U32:         return std::make_shared<ValueConst>(std::get<u32>(constExpr.value));
        case Type::I64:         return std::make_shared<ValueConst>(std::get<i64>(constExpr.value));
        case Type::U64:         return std::make_shared<ValueConst>(std::get<u64>(constExpr.value));
        case Type::Double:      return std::make_shared<ValueConst>(std::get<double>(constExpr.value));
        default:
            std::abort();
    }
}

std::unique_ptr<ExprResult> GenerateIr::genTernaryInst(const Parsing::TernaryExpr& ternaryExpr)
{
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), ternaryExpr.type->type);
    const Identifier endLabelIden = makeTemporaryName();
    const Identifier falseLabelName = makeTemporaryName();
    const auto conditionalExpr = dynCast<const Parsing::TernaryExpr>(&ternaryExpr);

    const std::shared_ptr<Value> condition = genInstAndConvert(*conditionalExpr->condition);
    emplaceJumpIfZero(condition, falseLabelName);

    const std::shared_ptr<Value> trueValue = genInstAndConvert(*conditionalExpr->trueExpr);
    emplaceCopy(trueValue, result, trueValue->type);
    emplaceJump(endLabelIden);

    emplaceLabel(falseLabelName);
    const std::shared_ptr<Value> falseValue = genInstAndConvert(*conditionalExpr->falseExpr);
    emplaceCopy(falseValue, result, falseValue->type);

    emplaceLabel(endLabelIden);
    return std::make_unique<PlainOperand>(result);
}

std::unique_ptr<ExprResult> GenerateIr::genFuncCallInst(const Parsing::FuncCallExpr& funcCallExpr)
{
    std::vector<std::shared_ptr<Value>> arguments;
    arguments.reserve(funcCallExpr.args.size());
    for (const auto& expr : funcCallExpr.args) {
        const std::shared_ptr<Value> arg = genInstAndConvert(*expr);
        arguments.emplace_back(arg);
    }
    if (funcCallExpr.type->type != Type::Pointer) {
        const auto returnType = dynCast<const Parsing::VarType>(funcCallExpr.type.get());
        auto dst = std::make_shared<ValueVar>(makeTemporaryName(), funcCallExpr.type->type);
        emplaceFunCall(Identifier(funcCallExpr.name), std::move(arguments), dst, returnType->type);
        return std::make_unique<PlainOperand>(dst);
    }
    auto dst = std::make_shared<ValueVar>(makeTemporaryName(), funcCallExpr.type->type);
    emplaceFunCall(Identifier(funcCallExpr.name), std::move(arguments), dst, Type::Pointer);
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
            emplaceGetAddress(plainOperand->value, dst, Type::Pointer);
            return std::make_unique<PlainOperand>(dst);
        }
        case ExprResult::Kind::DereferencedPointer: {
            const auto dereferencedPointer = dynCast<const DereferencedPointer>(inner.get());
            return std::make_unique<PlainOperand>(dereferencedPointer->ptr);
        }
    }
    std::unreachable();
}

i64 getReferencedTypeSize(Parsing::TypeBase* typeBase)
{
    std::vector<i64> scales;
    while (typeBase->kind != Parsing::TypeBase::Kind::Var) {
        switch (typeBase->kind) {
            case Parsing::TypeBase::Kind::Pointer: {
                const auto ptrType = dynCast<const Parsing::PointerType>(typeBase);
                typeBase = ptrType->referenced.get();
                if (typeBase->type == Type::Pointer) {
                    i64 scale = 8;
                    for (const i64 i : scales)
                        scale *= i;
                    return scale;
                }
                break;
            }
            case Parsing::TypeBase::Kind::Array: {
                const auto arrayType = dynCast<Parsing::ArrayType>(typeBase);
                scales.emplace_back(arrayType->size);
                typeBase = arrayType->elementType.get();
                break;
            }
            default:
                std::abort();
        }
    }
    i64 scale = getTypeSize(typeBase->type);
    for (const i64 i : scales)
        scale *= i;
    return scale;
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

std::unique_ptr<ExprResult> GenerateIr::genSubscript(const Parsing::SubscriptExpr& subscriptExpr)
{
    const std::shared_ptr<Value> ptr = genInstAndConvert(*subscriptExpr.referencing);
    const std::shared_ptr<Value> index = genInstAndConvert(*subscriptExpr.index);
    Parsing::TypeBase* referencedType = subscriptExpr.referencing->type.get();
    const i64 scale = getReferencedTypeSize(referencedType);
    auto result = std::make_shared<ValueVar>(makeTemporaryName(), Type::Pointer);
    emplaceAddPtr(ptr, index, result, scale);
    return std::make_unique<DereferencedPointer>(
        result, getSubscriptDereferenceType(subscriptExpr.referencing->type.get()));
}

std::unique_ptr<ExprResult> GenerateIr::genDereferenceInst(const Parsing::DereferenceExpr& dereferenceExpr)
{
    std::shared_ptr<Value> result = genInstAndConvert(*dereferenceExpr.reference);
    return std::make_unique<DereferencedPointer>(result, dereferenceExpr.type->type);
}

Identifier makeTemporaryName()
{
    return makeTemporaryName("");
}

Identifier makeTemporaryName(Value& value)
{
    if (value.kind == Value::Kind::Variable) {
        const auto val = dynCast<ValueVar>(&value);
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

std::shared_ptr<ValueConst> getInrDecScale(const Parsing::UnaryExpr& unaryExpr, Type type)
{
    if (type == Type::Pointer)
        return std::make_shared<ValueConst>(getReferencedTypeSize(unaryExpr.innerExpr->type.get()));
    if (type == Type::Double)
        return std::make_shared<ValueConst>(1.0);
    return std::make_shared<ValueConst>(1);
}
} // IR