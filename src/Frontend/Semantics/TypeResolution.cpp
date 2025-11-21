#include "TypeResolution.hpp"
#include "ASTIr.hpp"
#include "ASTUtils.hpp"
#include "AstToIrOperators.hpp"
#include "TypeConversion.hpp"

namespace {
using Storage = Parsing::Declaration::StorageClass;
}

namespace Semantics {
std::vector<Error> TypeResolution::validate(Parsing::Program& program)
{
    m_errors = std::vector<Error>();
    ASTTraverser::visit(program);
    return std::move(m_errors);
}

void TypeResolution::visit(Parsing::FuncDecl& funDecl)
{
    const auto it = m_functions.find(funDecl.name);
    if (it != m_functions.end() && !incompatibleFunctionDeclarations(it->second, funDecl)) {
        addError("Function declaration does not exist", funDecl.location);
        return;
    }
    const auto funcType = dynCast<Parsing::FuncType>(funDecl.type.get());
    std::vector<std::unique_ptr<Parsing::TypeBase>> paramsTypes;
    if (funcType->returnType->type == Type::Array) {
        addError("Function cannot return array", funDecl.location);
        return;
    }
    const auto type = dynCast<Parsing::FuncType>(funDecl.type.get());
    FuncEntry funcEntry(
        type->params,
        Parsing::deepCopy(*type->returnType),
        funDecl.storage,
        funDecl.body != nullptr);
    m_functions.emplace_hint(it, funDecl.name, std::move(funcEntry));

    if (funDecl.body) {
        m_definedFunctions.insert(funDecl.name);
        validateCompleteTypesFunc(funDecl, *funcType);
        m_global = false;
        funDecl.body->accept(*this);
        m_global = true;
    }
}

void TypeResolution::validateCompleteTypesFunc(const Parsing::FuncDecl& funDecl,
                                               const Parsing::FuncType& funcType)
{
    if (varTable.isInCompleteStructuredType(*funcType.returnType))
        addError("Incomplete return types", funDecl.location);
    for (const auto& paramTypeFunc : funcType.params)
        if (varTable.isInCompleteStructuredType(*paramTypeFunc))
            addError("Incomplete parameter types in function declarations", funDecl.location);
}

bool TypeResolution::incompatibleFunctionDeclarations(
    const FuncEntry& funcEntry,
    const Parsing::FuncDecl& funDecl)
{
    if ((funcEntry.storage == Storage::Extern || funcEntry.storage == Storage::None) &&
        funDecl.storage == Storage::Static) {
        addError("Incompatible storage types", funDecl.location);
        return false;
    }
    if (funDecl.params.size() != funcEntry.paramTypes.size()) {
        addError("Unequal parameter numbers in function declarations", funDecl.location);
        return false;
    }
    const auto funcType = dynCast<Parsing::FuncType>(funDecl.type.get());
    for (size_t i = 0; i < funDecl.params.size(); ++i) {
        const auto paramTypeEntry = funcEntry.paramTypes[i].get();
        const auto paramTypeFunc = funcType->params[i].get();
        if (!Parsing::areEquivalentArrayConversion(*paramTypeEntry, *paramTypeFunc)) {
            addError("Incompatible parameter types in function declarations", funDecl.location);
            return false;
        }
    }
    if (!Parsing::areEquivalentTypes(*funcType->returnType, *funcEntry.returnType)) {
        addError("Incompatible return types in function declarations", funDecl.location);
        return false;
    }
    return true;
}

void TypeResolution::visit(Parsing::VarDecl& varDecl)
{
    if (isIllegalVarDecl(varDecl))
        return;
    if (m_global && varDecl.storage == Storage::Static)
        m_globalStaticVars.insert(varDecl.name);
    if (!m_global && !m_globalStaticVars.contains(varDecl.name))
        m_definedFunctions.insert(varDecl.name);
    m_resolveExpr.m_isConst = true;

    if (varDecl.storage != Parsing::Declaration::StorageClass::Extern
        && varTable.isIncompleteTypeBase(*varDecl.type)) {
        addError("Cannot define variable with incomplete type", varDecl.location);
        return;
    }

    if (varDecl.init)
        initDecl(varDecl);

    if (hasError())
        return;
    if (illegalNonConstInitialization(varDecl, m_resolveExpr.m_isConst, m_global))
        addError("Is illegal non const variable initilization", varDecl.location);
}

bool TypeResolution::isIllegalVarDecl(const Parsing::VarDecl& varDecl)
{
    if (varDecl.storage == Storage::Extern && varDecl.init != nullptr) {
        addError("Initiated extern variable", varDecl.location);
        return true;
    }
    if (varDecl.storage == Storage::Static && m_definedFunctions.contains(varDecl.name)) {
        addError("Static variable with same name as defined function", varDecl.location);
        return true;
    }
    return false;
}

void TypeResolution::visit(Parsing::DeclForInit& declForInit)
{
    if (hasStorageClassSpecifier(declForInit))
        addError("Declaration in for cannot have storage specifier", declForInit.location);
    ASTTraverser::visit(declForInit);
}

void TypeResolution::visit(Parsing::ExprForInit& exprForInit)
{
    exprForInit.expression = m_resolveExpr.convertArrayType(*exprForInit.expression);
}

void TypeResolution::visit(Parsing::ReturnStmt& stmt)
{
    if (stmt.expr)
        stmt.expr = m_resolveExpr.convertArrayType(*stmt.expr);
}

void TypeResolution::visit(Parsing::ExprStmt& stmt)
{
    stmt.expr = m_resolveExpr.convertArrayType(*stmt.expr);
}

void TypeResolution::visit(Parsing::IfStmt& ifStmt)
{
    ifStmt.condition = m_resolveExpr.convertArrayType(*ifStmt.condition);
    if (ifStmt.condition) {
        if (ifStmt.condition->type && !isScalarType(*ifStmt.condition->type))
            addError("If condition must have scalar type", ifStmt.location);
    }
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt)
        ifStmt.elseStmt->accept(*this);
}

void TypeResolution::visit(Parsing::CaseStmt& caseStmt)
{
    caseStmt.condition = m_resolveExpr.convertArrayType(*caseStmt.condition);
    if (caseStmt.condition) {
        if (caseStmt.condition->type && !isScalarType(*caseStmt.condition->type))
            addError("Case condition must have scalar type", caseStmt.location);
    }
    caseStmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::WhileStmt& whileStmt)
{
    whileStmt.condition = m_resolveExpr.convertArrayType(*whileStmt.condition);
    if (whileStmt.condition) {
        if (whileStmt.condition->type && !isScalarType(*whileStmt.condition->type))
            addError("While condition must have scalar type", whileStmt.location);
    }
    whileStmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::DoWhileStmt& doWhileStmt)
{
    doWhileStmt.condition = m_resolveExpr.convertArrayType(*doWhileStmt.condition);
    if (doWhileStmt.condition) {
        if (doWhileStmt.condition->type && !isScalarType(*doWhileStmt.condition->type))
            addError("Do While condition must have scalar type", doWhileStmt.location);
    }
    doWhileStmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::ForStmt& forStmt)
{
    if (forStmt.init)
        forStmt.init->accept(*this);
    if (forStmt.condition) {
        forStmt.condition = m_resolveExpr.convertArrayType(*forStmt.condition);
        if (forStmt.condition->type && !isScalarType(*forStmt.condition->type))
            addError("For loop condition must have scalar type", forStmt.location);
    }
    if (forStmt.post)
        forStmt.post = m_resolveExpr.convertArrayType(*forStmt.post);
    forStmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::SwitchStmt& switchStmt)
{
    switchStmt.condition = m_resolveExpr.convertArrayType(*switchStmt.condition);
    if (switchStmt.condition) {
        if (switchStmt.condition->type && !isScalarType(*switchStmt.condition->type))
            addError("Switch condition must have scalar type", switchStmt.location);
    }
    switchStmt.body->accept(*this);
}

void TypeResolution::initDecl(Parsing::VarDecl& varDecl)
{
    location = varDecl.location;
    const auto type = varDecl.type.get();
    const auto init = varDecl.init.get();
    std::vector<std::unique_ptr<Parsing::Initializer>> newInits;
    walkInit(type, init, newInits);
    if (hasError())
        return;
    if (newInits.empty())
        return;
    if (newInits.size() == 1) {
        const auto first = newInits[0].get();
        const auto singleInit = dynCast<Parsing::SingleInitializer>(first);
        varDecl.init = std::make_unique<Parsing::SingleInitializer>(std::move(singleInit->expr));
        return;
    }
    varDecl.init = std::make_unique<Parsing::CompoundInitializer>(std::move(newInits));
}

void TypeResolution::walkInit(const Parsing::TypeBase* type,
                              Parsing::Initializer* init,
                              std::vector<std::unique_ptr<Parsing::Initializer>>& newInit)
{
    using TypeKind = Parsing::TypeBase::Kind;
    using InitKind = Parsing::Initializer::Kind;

    if (type->kind == TypeKind::Array && init->kind == InitKind::Compound) {
        const auto arrayType = dynCast<const Parsing::ArrayType>(type);
        const auto compoundInit = dynCast<Parsing::CompoundInitializer>(init);
        initArrayWithCompound(*arrayType, *compoundInit, newInit);
        return;
    }
    if (type->kind == TypeKind::Array && init->kind == InitKind::Single) {
        const auto arrayType = dynCast<const Parsing::ArrayType>(type);
        const auto singleInit = dynCast<Parsing::SingleInitializer>(init);
        initArrayWithSingle(*arrayType, *singleInit, newInit);
        return;
    }
    if ((type->kind == TypeKind::Pointer || type->kind == TypeKind::Var)
        && init->kind == InitKind::Single) {
        initVarWithSingle(type, init, newInit);
        return;
    }
    addError("Faulty initializer", location);
}

void TypeResolution::initArrayWithCompound(const Parsing::ArrayType& arrayType,
                                           const Parsing::CompoundInitializer& compoundInit,
                                           std::vector<std::unique_ptr<Parsing::Initializer>>& newInit)
{
    if (arrayType.size < compoundInit.size()) {
        addError("Cannot initialize array with longer compound", location);
        return;
    }
    const auto elemType = arrayType.elementType.get();
    for (auto& partInit : compoundInit.initializers)
        walkInit(elemType, partInit.get(), newInit);
    const i64 notInitElems = arrayType.size - compoundInit.size();
    const i64 lengthZero = notInitElems * getTypeOfSize(elemType);
    if (lengthZero)
        emplaceZeroInit(newInit, lengthZero);
}

void TypeResolution::initArrayWithSingle(const Parsing::ArrayType& arrayType,
                                         const Parsing::SingleInitializer& singleInit,
                                         std::vector<std::unique_ptr<Parsing::Initializer>>& newInit)
{
    if (!isCharacterType(arrayType.elementType->type)) {
        addError("Single initializer must be of type Char for array", location);
        return;
    }
    if (singleInit.expr->kind != Parsing::Expr::Kind::String) {
        addError("Char array single init must be string expression", location);
        return;
    }
    const auto stringExpr = dynCast<const Parsing::StringExpr>(singleInit.expr.get());
    const std::string& str = stringExpr->value;
    if (arrayType.size < str.size()) {
        addError("Cannot initialize array with longer string", location);
        return;
    }
    i64 i = 0;
    for (; i + 4 < str.size(); i += 4) {
        i32 value = 0;
        value += str[i];
        value += str[i + 1] << 8;
        value += str[i + 2] << 16;
        value += str[i + 3] << 24;
        auto constExprI32 = std::make_unique<Parsing::ConstExpr>(
            value, std::make_unique<Parsing::VarType>(Type::I32));
        newInit.emplace_back(std::make_unique<Parsing::SingleInitializer>(std::move(constExprI32)));
    }
    for (;i < str.size(); ++i) {
        auto constExpr = std::make_unique<Parsing::ConstExpr>(
            static_cast<i8>(str[i]), std::make_unique<Parsing::VarType>(Type::I8));
        newInit.emplace_back(std::make_unique<Parsing::SingleInitializer>(std::move(constExpr)));
    }
    const i64 diff = arrayType.size - str.size();
    if (diff)
        emplaceZeroInit(newInit, diff);
}

void TypeResolution::initVarWithSingle(
    const Parsing::TypeBase* type,
    Parsing::Initializer* init,
    std::vector<std::unique_ptr<Parsing::Initializer>>& newInit)
{
    const auto singleInit = dynCast<Parsing::SingleInitializer>(init);
    auto exprConverted = m_resolveExpr.convertArrayType(*singleInit->expr);
    if (hasError())
        return;
    auto expr = Parsing::converOrAssign(
        *type, *exprConverted->type, exprConverted, m_errors);
    newInit.emplace_back(std::make_unique<Parsing::SingleInitializer>(std::move(expr)));
}

void emplaceZeroInit(std::vector<std::unique_ptr<Parsing::Initializer>>& newInit, i64 lengthZero)
{
    if (!newInit.empty()) {
        const auto& back = newInit.back();
        if (back->kind == Parsing::Initializer::Kind::Zero) {
            const auto zeroInit = dynCast<Parsing::ZeroInitializer>(back.get());
            lengthZero += zeroInit->size;
            newInit.pop_back();
        }
    }
    newInit.emplace_back(std::make_unique<Parsing::ZeroInitializer>(lengthZero));
}

bool isZeroSingleInit(const Parsing::Initializer& init)
{
    if (init.kind != Parsing::Initializer::Kind::Single)
        return false;
    const auto singleInit = dynCast<const Parsing::SingleInitializer>(&init);
    if (singleInit->expr->kind != Parsing::Expr::Kind::Constant)
        return false;
    const auto constExpr = dynCast<const Parsing::ConstExpr>(singleInit->expr.get());
    return Parsing::isZeroArithmeticType(*constExpr);
}
} // namespace Semantics