#include "TypeResolution.hpp"
#include "ASTInitializer.hpp"
#include "ASTIr.hpp"
#include "DynCast.hpp"
#include "TypeConversion.hpp"
#include "Utils.hpp"

#include <cassert>

namespace Semantics {
bool TypeResolution::validate(Parsing::Program& program)
{
    for (std::unique_ptr<Parsing::Declaration>& decl : program.declarations)
        checkDecl(*decl);
    return m_valid;
}

void TypeResolution::checkDecl(Parsing::Declaration& decl)
{
    using Kind = Parsing::Declaration::Kind;
    switch (decl.kind) {
        case Kind::VarDecl: {
            const auto varDecl = dyn_cast<Parsing::VarDecl>(&decl);
            checkVarDecl(*varDecl);
            break;
        }
        case Kind::FuncDecl: {
            const auto funcDecl = dyn_cast<Parsing::FunDecl>(&decl);
            checkFuncDecl(*funcDecl);
            break;
        }
        default:
            std::abort();
    }
}

void TypeResolution::checkVarDecl(Parsing::VarDecl& varDecl)
{
    if (isIllegalVarDecl(varDecl)) {
        m_valid = false;
        return;
    }
    if (m_global && varDecl.storage == Storage::Static)
        m_globalStaticVars.insert(varDecl.name);
    if (!m_global && !m_globalStaticVars.contains(varDecl.name))
        m_definedFunctions.insert(varDecl.name);
    m_isConst = true;
    if (varDecl.init)
        checkInitializer(*varDecl.init);
    if (!m_valid)
        return;
    if (illegalNonConstInitialization(varDecl, m_isConst, m_global)) {
        m_valid = false;
        return;
    }
    if (varDecl.init == nullptr)
        return;
    if (varDecl.init->kind != Parsing::Initializer::Kind::Single) {
        m_valid = false;
        return;
    }
    auto init = dyn_cast<Parsing::SingleInit>(varDecl.init.get());
    if (!Parsing::areEquivalent(*varDecl.type, *init->exp->type)) {
        if (varDecl.type->type == Type::Pointer) {
            if (!canConvertToNullPtr(*init->exp)) {
                m_valid = false;
                return;
            }
            auto typeExpr = std::make_unique<Parsing::VarType>(Type::U64);
            varDecl.init = std::make_unique<Parsing::SingleInit>(
                std::make_unique<Parsing::ConstExpr>(0ul, std::move(typeExpr)));
        }
    }
    assignTypeToArithmeticUnaryExpr(varDecl);
}

void TypeResolution::checkFuncDecl(const Parsing::FunDecl& funDecl)
{
    const auto it = m_functions.find(funDecl.name);
    if (it != m_functions.end() && !validFuncDecl(it->second, funDecl)) {
        m_valid = false;
        return;
    }
    if (!m_global && funDecl.body != nullptr) {
        m_valid = false;
        return;
    }
    if (funDecl.body != nullptr)
        m_definedFunctions.insert(funDecl.name);
    m_global = false;
    const auto type = dyn_cast<Parsing::FuncType>(funDecl.type.get());
    FuncEntry funcEntry(
        type->params, type->returnType->type, funDecl.storage, funDecl.body != nullptr);
    m_functions.emplace_hint(it, funDecl.name, std::move(funcEntry));
    if (funDecl.body)
        checkBlock(*funDecl.body);
    m_global = true;
}

void TypeResolution::checkBlock(const Parsing::Block& block)
{
    for (const std::unique_ptr<Parsing::BlockItem>& blockItem : block.body)
        checkBlockItem(*blockItem);
}

void TypeResolution::checkBlockItem(Parsing::BlockItem& blockItem)
{
    using Kind = Parsing::BlockItem::Kind;
    switch (blockItem.kind) {
        case Kind::Declaration: {
            const auto decl = dyn_cast<Parsing::DeclBlockItem>(&blockItem);
            checkDeclBlockItem(*decl);
            break;
        }
        case Kind::Statement: {
            auto stmt = dyn_cast<Parsing::StmtBlockItem>(&blockItem);
            checkStmtBlockItem(*stmt);
            break;
        }
        default:
            std::abort();
    }
}

void TypeResolution::checkStmtBlockItem(const Parsing::StmtBlockItem& stmtBlockItem)
{
    checkStmt(*stmtBlockItem.stmt);
}

void TypeResolution::checkDeclBlockItem(const Parsing::DeclBlockItem& declBlockItem)
{
    checkDecl(*declBlockItem.decl);
}

void TypeResolution::checkInitializer(Parsing::Initializer& initializer)
{
    using Kind = Parsing::Initializer::Kind;
    switch (initializer.kind) {
        case Kind::Compound: {
            const auto compoundInit = dyn_cast<Parsing::CompoundInit>(&initializer);
            checkCompoundInit(*compoundInit);
            break;
        }
        case Kind::Single: {
            const auto singleInit = dyn_cast<Parsing::SingleInit>(&initializer);
            checkSingleInit(*singleInit);
            break;
        }
        default:
            std::abort();
    }
}

void TypeResolution::checkSingleInit(Parsing::SingleInit& singleInit)
{
    checkExpr(*singleInit.exp);
}

void TypeResolution::checkCompoundInit(const Parsing::CompoundInit& compoundInit)
{
    for (const std::unique_ptr<Parsing::Initializer>& initializer : compoundInit.exp)
        checkInitializer(*initializer);
}

void TypeResolution::checkForInit(Parsing::ForInit& forInit)
{
    using Kind = Parsing::ForInit::Kind;
    switch (forInit.kind) {
        case Kind::Declaration: {
            const auto decl = dyn_cast<Parsing::DeclForInit>(&forInit);
            checkDeclForInit(*decl);
            break;
        }
        case Kind::Expression: {
            const auto expr = dyn_cast<Parsing::ExprForInit>(&forInit);
            checkExprForInit(*expr);
            break;
        }
        default:
            std::abort();
    }
}

void TypeResolution::checkDeclForInit(const Parsing::DeclForInit& declForInit)
{
    if (hasStorageClassSpecifier(declForInit))
        m_valid = false;
    checkDecl(*declForInit.decl);
}

void TypeResolution::checkExprForInit(const Parsing::ExprForInit& exprForInit)
{
    if (exprForInit.expression)
        checkExpr(*exprForInit.expression);
}

void TypeResolution::checkStmt(Parsing::Stmt& stmt)
{
    using Kind = Parsing::Stmt::Kind;
    switch (stmt.kind) {
        case Kind::Return: {
            const auto ret = dyn_cast<Parsing::ReturnStmt>(&stmt);
            checkReturnStmt(*ret);
            return;
        }
        case Kind::Expression: {
            const auto expr = dyn_cast<Parsing::ExprStmt>(&stmt);
            checkExprStmt(*expr);
            return;
        }
        case Kind::If: {
            const auto cond = dyn_cast<Parsing::IfStmt>(&stmt);
            checkIfStmt(*cond);
            return;
        }
        case Kind::Goto: {
            const auto gotoStmt = dyn_cast<Parsing::GotoStmt>(&stmt);
            checkGotoStmt(*gotoStmt);
            return;
        }
        case Kind::Compound: {
            const auto compound = dyn_cast<Parsing::CompoundStmt>(&stmt);
            checkCompoundStmt(*compound);
            return;
        }
        case Kind::Break: {
            const auto breakStmt = dyn_cast<Parsing::BreakStmt>(&stmt);
            checkBreakStmt(*breakStmt);
            return;
        }
        case Kind::Continue: {
            const auto continueStmt = dyn_cast<Parsing::ContinueStmt>(&stmt);
            checkContinueStmt(*continueStmt);
            return;
        }
        case Kind::Label: {
            const auto label = dyn_cast<Parsing::LabelStmt>(&stmt);
            checkLabelStmt(*label);
            return;
        }
        case Kind::Case: {
            const auto caseStmt = dyn_cast<Parsing::CaseStmt>(&stmt);
            checkCaseStmt(*caseStmt);
            return;
        }
        case Kind::Default: {
            const auto defaultStmt = dyn_cast<Parsing::DefaultStmt>(&stmt);
            checkDefaultStmt(*defaultStmt);
            return;
        }
        case Kind::While: {
            const auto whileStmt = dyn_cast<Parsing::WhileStmt>(&stmt);
            checkWhileStmt(*whileStmt);
            return;
        }
        case Kind::DoWhile: {
            const auto whileStmt = dyn_cast<Parsing::DoWhileStmt>(&stmt);
            checkDoWhileStmt(*whileStmt);
            return;
        }
        case Kind::For: {
            const auto forStmt = dyn_cast<Parsing::ForStmt>(&stmt);
            checkForStmt(*forStmt);
            return;
        }
        case Kind::Switch: {
            const auto switchStmt = dyn_cast<Parsing::SwitchStmt>(&stmt);
            checkSwitchStmt(*switchStmt);
            return;
        }
        case Kind::Null: {
            const auto nullStmt = dyn_cast<Parsing::NullStmt>(&stmt);
            checkNullStmt(*nullStmt);
            return;
        }
    }
    std::abort();
}

void TypeResolution::checkReturnStmt(const Parsing::ReturnStmt& returnStmt)
{
    checkExpr(*returnStmt.expr);
}

void TypeResolution::checkExprStmt(const Parsing::ExprStmt& exprStmt)
{
    checkExpr(*exprStmt.expr);
}

void TypeResolution::checkIfStmt(const Parsing::IfStmt& ifStmt)
{
    checkExpr(*ifStmt.condition);
    checkStmt(*ifStmt.thenStmt);
    if (ifStmt.elseStmt)
        checkStmt(*ifStmt.elseStmt);
}

void TypeResolution::checkCompoundStmt(const Parsing::CompoundStmt& compoundStmt)
{
    checkBlock(*compoundStmt.block);
}

void TypeResolution::checkLabelStmt(const Parsing::LabelStmt& labelStmt)
{
    checkStmt(*labelStmt.stmt);
}

void TypeResolution::checkCaseStmt(const Parsing::CaseStmt& caseStmt)
{
    checkExpr(*caseStmt.condition);
    checkStmt(*caseStmt.body);
}

void TypeResolution::checkDefaultStmt(const Parsing::DefaultStmt& defaultStmt)
{
    checkStmt(*defaultStmt.body);
}

void TypeResolution::checkWhileStmt(const Parsing::WhileStmt& whileStmt)
{
    checkExpr(*whileStmt.condition);
    checkStmt(*whileStmt.body);
}

void TypeResolution::checkDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt)
{
    checkStmt(*doWhileStmt.body);
    checkExpr(*doWhileStmt.condition);
}

void TypeResolution::checkForStmt(const Parsing::ForStmt& forStmt)
{
    if (forStmt.init)
        checkForInit(*forStmt.init);
    if (forStmt.condition)
        checkExpr(*forStmt.condition);
    if (forStmt.post)
        checkExpr(*forStmt.post);
    checkStmt(*forStmt.body);
}

void TypeResolution::checkSwitchStmt(const Parsing::SwitchStmt& switchStmt)
{
    checkExpr(*switchStmt.condition);
    checkStmt(*switchStmt.body);
}

void TypeResolution::checkExpr(Parsing::Expr& expr)
{
    using Kind = Parsing::Expr::Kind;
    switch (expr.kind) {
        case Kind::Constant: {
            const auto constant = dyn_cast<Parsing::ConstExpr>(&expr);
            checkConstExpr(*constant);
            return;
        }
        case Kind::Var: {
            const auto var = dyn_cast<Parsing::VarExpr>(&expr);
            checkVarExpr(*var);
            return;
        }
        case Kind::Cast: {
            const auto cast = dyn_cast<Parsing::CastExpr>(&expr);
            checkCastExpr(*cast);
            return;
        }
        case Kind::Binary: {
            const auto binary = dyn_cast<Parsing::BinaryExpr>(&expr);
            checkBinaryExpr(*binary);
            return;
        }
        case Kind::Unary: {
            const auto unary = dyn_cast<Parsing::UnaryExpr>(&expr);
            checkUnaryExpr(*unary);
            return;
        }
        case Kind::Assignment: {
            const auto assignment = dyn_cast<Parsing::AssignmentExpr>(&expr);
            checkAssignExpr(*assignment);
            return;
        }
        case Kind::Ternary: {
            const auto ternary = dyn_cast<Parsing::TernaryExpr>(&expr);
            checkTernaryExpr(*ternary);
            return;
        }
        case Kind::FunctionCall: {
            const auto funcCall = dyn_cast<Parsing::FuncCallExpr>(&expr);
            checkFuncCallExpr(*funcCall);
            return;
        }
        case Kind::Dereference: {
            const auto dereference = dyn_cast<Parsing::DereferenceExpr>(&expr);
            checkDereferenceExpr(*dereference);
            return;
        }
        case Kind::AddrOf: {
            const auto addrOf = dyn_cast<Parsing::AddrOffExpr>(&expr);
            checkAddrOffExpr(*addrOf);
            return;
        }
        case Kind::Subscript: {
            const auto subscript = dyn_cast<Parsing::SubscriptExpr>(&expr);
            checkSubscriptExpr(*subscript);
            return;
        }
    }
    std::abort();
}

void TypeResolution::checkConstExpr(Parsing::ConstExpr& expr)
{

}

void TypeResolution::checkVarExpr(Parsing::VarExpr& expr)
{
    m_isConst = false;
}

void TypeResolution::checkCastExpr(const Parsing::CastExpr& castExpr)
{
    checkExpr(*castExpr.expr);
    if (!m_valid)
        return;
    const Type outerType = castExpr.type->type;
    const Type innerType = castExpr.expr->type->type;
    if (isCastFromPointerToAndFromDouble(outerType, innerType)) {
        m_valid = false;
        return;
    }
}

void TypeResolution::checkUnaryExpr(Parsing::UnaryExpr& unaryExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;

    checkExpr(*unaryExpr.operand);

    if (!m_valid)
        return;
    if (unaryExpr.operand->type->type == Type::Double) {
        if (unaryExpr.op == Operator::Complement) {
            m_valid = false;
            return;
        }
    }
    if (unaryExpr.operand->type->type == Type::Pointer) {
        if (isIllegalUnaryPointerOperator(unaryExpr.op)) {
            m_valid = false;
            return;
        }
    }
    if (unaryExpr.op == Operator::Not)
        unaryExpr.type = std::make_unique<Parsing::VarType>(s_boolType);
    else
        unaryExpr.type = std::make_unique<Parsing::VarType>(unaryExpr.operand->type->type);
}

void TypeResolution::checkBinaryExpr(Parsing::BinaryExpr& binaryExpr)
{
    using Operator = Parsing::BinaryExpr::Operator;

    checkExpr(*binaryExpr.lhs);
    checkExpr(*binaryExpr.rhs);

    if (!m_valid)
        return;
    if (binaryExpr.op == Operator::And || binaryExpr.op == Operator::Or) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return;
    }
    const Type leftType = binaryExpr.lhs->type->type;
    const Type rightType = binaryExpr.rhs->type->type;
    const Type commonType = getCommonType(leftType, rightType);
    if (commonType == Type::Double && (isBinaryBitwise(binaryExpr.op) || binaryExpr.op == Operator::Modulo)) {
        m_valid = false;
        return;
    }
    if (leftType == Type::Pointer || rightType == Type::Pointer) {
        if (binaryExpr.op == Operator::Modulo || binaryExpr.op == Operator::Multiply ||
            binaryExpr.op == Operator::Divide ||
            binaryExpr.op == Operator::BitwiseOr || binaryExpr.op == Operator::BitwiseXor) {
            m_valid = false;
            return;
            }
        if (!areValidNonArithmeticTypesInBinaryExpr(binaryExpr)) {
            m_valid = false;
            return;
        }
        if (leftType != Type::Pointer || rightType != Type::Pointer) {
            if (leftType != Type::Pointer) {
                binaryExpr.lhs = std::make_unique<Parsing::CastExpr>(
                    std::move(Parsing::deepCopy(*binaryExpr.rhs->type)), std::move(binaryExpr.lhs));
            }
            if (rightType != Type::Pointer) {
                binaryExpr.rhs = std::make_unique<Parsing::CastExpr>(
                    std::move(Parsing::deepCopy(*binaryExpr.lhs->type)), std::move(binaryExpr.rhs));
            }
        }
        if (binaryExpr.op == Operator::Equal || binaryExpr.op == Operator::NotEqual) {
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
            return;
        }
        m_valid = false;
        return;
    }
    assignTypeToArithmeticBinaryExpr(binaryExpr, leftType, rightType, commonType);
}

void TypeResolution::checkAssignExpr(Parsing::AssignmentExpr& assignExpr)
{
    checkExpr(*assignExpr.lhs);
    checkExpr(*assignExpr.rhs);

    if (!m_valid)
        return;
    const Type leftType = assignExpr.lhs->type->type;
    const Type rightType = assignExpr.rhs->type->type;
    if (!isLegalAssignExpr(assignExpr)) {
        m_valid = false;
        return;
    }
    if (leftType != rightType)
        assignExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(leftType), std::move(assignExpr.rhs));
    assignExpr.type = Parsing::deepCopy(*assignExpr.lhs->type);
}

void TypeResolution::checkTernaryExpr(Parsing::TernaryExpr& ternaryExpr)
{
    checkExpr(*ternaryExpr.condition);
    checkExpr(*ternaryExpr.trueExpr);
    checkExpr(*ternaryExpr.falseExpr);

    const Type trueType = ternaryExpr.trueExpr->type->type;
    const Type falseType = ternaryExpr.falseExpr->type->type;
    const Type commonType = getCommonType(trueType, falseType);
    if (trueType == Type::Pointer || falseType == Type::Pointer) {
        if (trueType == Type::Pointer && falseType == Type::Pointer) {
            if (!Parsing::areEquivalent(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
                m_valid = false;
            ternaryExpr.type = std::move(Parsing::deepCopy(*ternaryExpr.trueExpr->type));
            return;
        }
        if (!areValidNonArithmeticTypesInTernaryExpr(ternaryExpr)) {
            m_valid = false;
            return;
        }
        if (trueType != Type::Pointer) {
            ternaryExpr.trueExpr = std::make_unique<Parsing::CastExpr>(
                std::move(Parsing::deepCopy(*ternaryExpr.falseExpr->type)), std::move(ternaryExpr.trueExpr));
        }
        if (falseType != Type::Pointer) {
            ternaryExpr.falseExpr = std::make_unique<Parsing::CastExpr>(
                std::move(Parsing::deepCopy(*ternaryExpr.trueExpr->type)), std::move(ternaryExpr.falseExpr));
        }
        ternaryExpr.type = std::move(Parsing::deepCopy(*ternaryExpr.trueExpr->type));
        return;
    }
    if (commonType != trueType)
        ternaryExpr.trueExpr = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(ternaryExpr.trueExpr));
    if (commonType != falseType)
        ternaryExpr.falseExpr = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(ternaryExpr.falseExpr));
    ternaryExpr.type = std::make_unique<Parsing::VarType>(commonType);
}

void TypeResolution::checkFuncCallExpr(Parsing::FuncCallExpr& funcCallExpr)
{
    const auto it = m_functions.find(funcCallExpr.name);
    if (it == m_functions.end()) {
        m_valid = false;
        return;
    }
    if (it->second.paramTypes.size() != funcCallExpr.args.size()) {
        m_valid = false;
        return;
    }

    for (const std::unique_ptr<Parsing::Expr>& expr : funcCallExpr.args)
        checkExpr(*expr);

    if (!m_valid)
        return;
    for (size_t i = 0; i < funcCallExpr.args.size(); ++i) {
        const Type typeInner = funcCallExpr.args[i]->type->type;
        const Type castTo = it->second.paramTypes[i]->type;
        if (typeInner == Type::Pointer && castTo == Type::Pointer) {
            if (!Parsing::areEquivalent(*funcCallExpr.args[i]->type, *it->second.paramTypes[i])) {
                m_valid = false;
                return;
            }
        }
        if (castTo != Type::Pointer && typeInner == Type::Pointer) {
            m_valid = false;
            return;
        }
        if (typeInner != castTo)
            funcCallExpr.args[i] = std::make_unique<Parsing::CastExpr>(
                Parsing::deepCopy(*it->second.paramTypes[i]), std::move(funcCallExpr.args[i]));
    }
}

void TypeResolution::checkDereferenceExpr(Parsing::DereferenceExpr& dereferenceExpr)
{
    checkExpr(*dereferenceExpr.reference);
    if (!m_valid)
        return;
    if (dereferenceExpr.reference->type->type != Type::Pointer) {
        m_valid = false;
        return;
    }
    const auto referencedPtrType = dyn_cast<const Parsing::PointerType>(dereferenceExpr.reference->type.get());
    dereferenceExpr.type = Parsing::deepCopy(*referencedPtrType->referenced);
}

void TypeResolution::checkAddrOffExpr(Parsing::AddrOffExpr& addrOffExpr)
{
    assert(addrOffExpr.reference && "AddrOfExpr has no reference!");
    checkExpr(*addrOffExpr.reference);
    if (!m_valid)
        return;
    if (addrOffExpr.reference->kind == Parsing::Expr::Kind::AddrOf) {
        m_valid = false;
        return;
    }
    addrOffExpr.type = std::make_unique<Parsing::PointerType>(
        Parsing::deepCopy(*addrOffExpr.reference->type));
}

void TypeResolution::checkSubscriptExpr(const Parsing::SubscriptExpr& subscriptExpr)
{
    checkExpr(*subscriptExpr.reference);
    checkExpr(*subscriptExpr.index);
}

bool TypeResolution::validFuncDecl(const FuncEntry& funcEntry, const Parsing::FunDecl& funDecl)
{
    if ((funcEntry.storage == Storage::Extern || funcEntry.storage == Storage::None)
        && funDecl.storage == Storage::Static)
        return false;
    if (funDecl.params.size() != funcEntry.paramTypes.size())
        return false;
    const auto funcType = dyn_cast<Parsing::FuncType>(funDecl.type.get());
    for (size_t i = 0; i < funDecl.params.size(); ++i)
        if (funcEntry.paramTypes[i]->type != funcType->params[i]->type)
            return false;
    if (funcType->returnType->type != funcEntry.returnType)
        return false;
    return true;
}

void TypeResolution::assignTypeToArithmeticBinaryExpr(
    Parsing::BinaryExpr& binaryExpr,
    const Type leftType, const Type rightType, const Type commonType)
{
    using Operator = Parsing::BinaryExpr::Operator;
    if (commonType != leftType)
        binaryExpr.lhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.lhs));
    if (commonType != rightType)
        binaryExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.rhs));
    if (binaryExpr.op == Operator::LeftShift ||
        binaryExpr.op == Operator::RightShift) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(leftType);
        return;
    }
    if (isBinaryComparison(binaryExpr)) {
        if (commonType == Type::Double || isSigned(commonType))
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        else
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::U32);
        return;
    }
    binaryExpr.type = std::make_unique<Parsing::VarType>(commonType);
}

void TypeResolution::assignTypeToArithmeticUnaryExpr(Parsing::VarDecl& varDecl)
{
    if (varDecl.init->kind != Parsing::Initializer::Kind::Single)
        return;
    auto init = dyn_cast<Parsing::SingleInit>(varDecl.init.get());
    if (init->exp->kind == Parsing::Expr::Kind::Constant &&
        varDecl.type->type != init->exp->type->type) {
        const auto constExpr = dyn_cast<const Parsing::ConstExpr>(init->exp.get());
        if (varDecl.type->type == Type::U32)
            convertConstantExpr<u32, Type::U32>(varDecl, *constExpr);
        else if (varDecl.type->type == Type::U64)
            convertConstantExpr<u64, Type::U64>(varDecl, *constExpr);
        else if (varDecl.type->type == Type::I32)
            convertConstantExpr<i32, Type::I32>(varDecl, *constExpr);
        else if (varDecl.type->type == Type::I64)
            convertConstantExpr<i64, Type::I64>(varDecl, *constExpr);
        else if (varDecl.type->type == Type::Double)
            convertConstantExpr<double, Type::Double>(varDecl, *constExpr);
        return;
        }
    if (varDecl.type->type != init->exp->type->type) {
        varDecl.init = std::make_unique<Parsing::SingleInit>(std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(varDecl.type->type),
            std::move(init->exp)));
    }
}

bool areValidNonArithmeticTypesInBinaryExpr(const Parsing::BinaryExpr& binaryExpr)
{
    if (Parsing::areEquivalent(*binaryExpr.lhs->type, *binaryExpr.rhs->type))
        return true;
    if (canConvertToNullPtr(*binaryExpr.lhs) || canConvertToNullPtr(*binaryExpr.rhs))
        return true;
    return false;
}

bool isLegalAssignExpr(Parsing::AssignmentExpr& assignmentExpr)
{
    const Type leftType = assignmentExpr.lhs->type->type;
    const Type rightType = assignmentExpr.rhs->type->type;
    if (leftType == Type::Pointer && rightType == Type::Pointer)
        return Parsing::areEquivalent(*assignmentExpr.lhs->type, *assignmentExpr.rhs->type);
    if (leftType == Type::Pointer) {
        if (canConvertToNullPtr(*assignmentExpr.rhs)) {
            assignmentExpr.rhs = std::make_unique<Parsing::CastExpr>(
                Parsing::deepCopy(*assignmentExpr.lhs->type), std::move(assignmentExpr.rhs));
        }
        else
            return false;
    }
    return true;
}

bool areValidNonArithmeticTypesInTernaryExpr(const Parsing::TernaryExpr& ternaryExpr)
{
    if (Parsing::areEquivalent(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
        return true;
    if (canConvertToNullPtr(*ternaryExpr.trueExpr) || canConvertToNullPtr(*ternaryExpr.falseExpr))
        return true;
    return false;
}

bool TypeResolution::isIllegalVarDecl(const Parsing::VarDecl& varDecl) const
{
    if (varDecl.storage == Storage::Extern && varDecl.init != nullptr)
        return true;
    if (varDecl.storage == Storage::Static && m_definedFunctions.contains(varDecl.name))
        return true;
    return false;
}

bool isCastFromPointerToAndFromDouble(const Type outerType, const Type innerType)
{
    return (outerType == Type::Double && innerType == Type::Pointer) ||
           (outerType == Type::Pointer && innerType == Type::Double);
}
} // namespace Semantics