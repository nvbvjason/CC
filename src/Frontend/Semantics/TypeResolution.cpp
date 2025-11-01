#include "TypeResolution.hpp"
#include "ASTIr.hpp"
#include "DynCast.hpp"
#include "TypeConversion.hpp"
#include "Utils.hpp"
#include "ASTDeepCopy.hpp"

namespace {
using BinaryOp = Parsing::BinaryExpr::Operator;
}

namespace Semantics {
bool TypeResolution::validate(Parsing::Program& program)
{
    ASTTraverser::visit(program);
    return m_valid;
}

void TypeResolution::visit(Parsing::FunDeclaration& funDecl)
{
    const auto it = m_functions.find(funDecl.name);
    if (it != m_functions.end() && !validFuncDecl(it->second, funDecl)) {
        m_valid = false;
        return;
    }
    auto funcType = dynCast<Parsing::FuncType>(funDecl.type.get());
    std::vector<std::unique_ptr<Parsing::TypeBase>> paramsTypes;
    for (auto & param : funcType->params) {
        if (param->type == Type::Array) {
            auto arrayType = dynCast<Parsing::ArrayType>(param.get());
            auto pointerType = std::make_unique<Parsing::PointerType>(Parsing::deepCopy(*arrayType->elementType));
            param = std::move(pointerType);
        }
    }
    if (funcType->returnType->type == Type::Array) {
        m_valid = false;
        return;
    }

    if (funDecl.body != nullptr)
        m_definedFunctions.insert(funDecl.name);
    const auto type = dynCast<Parsing::FuncType>(funDecl.type.get());
    FuncEntry funcEntry(type->params, type->returnType->type, funDecl.storage,
        funDecl.body != nullptr);
    m_functions.emplace_hint(it, funDecl.name, std::move(funcEntry));
    m_global = false;


    if (funDecl.body)
        funDecl.body->accept(*this);

    m_global = true;
}

bool TypeResolution::validFuncDecl(const FuncEntry& funcEntry, const Parsing::FunDeclaration& funDecl)
{
    if ((funcEntry.storage == Storage::Extern || funcEntry.storage == Storage::None) &&
        funDecl.storage == Storage::Static)
        return false;
    if (funDecl.params.size() != funcEntry.paramTypes.size())
        return false;
    const auto funcType = dynCast<Parsing::FuncType>(funDecl.type.get());
    for (size_t i = 0; i < funDecl.params.size(); ++i)
        if (funcEntry.paramTypes[i]->type != funcType->params[i]->type)
            return false;
    if (funcType->returnType->type != funcEntry.returnType)
        return false;
    return true;
}

void TypeResolution::visit(Parsing::VarDecl& varDecl)
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
        varDecl.init->accept(*this);

    if (!m_valid)
        return;
    if (illegalNonConstInitialization(varDecl, m_isConst, m_global)) {
        m_valid = false;
        return;
    }
    if (varDecl.init == nullptr)
        return;
    if (varDecl.init->kind == Parsing::Initializer::Kind::Single) {
        auto singleInit = dynCast<Parsing::SingleInitializer>(varDecl.init.get());

        singleInit->exp = convertArrayType(*singleInit->exp);

        if (!m_valid)
            return;

        if (!Parsing::areEquivalent(*varDecl.type, *singleInit->exp->type)) {
            if (varDecl.type->type == Type::Pointer) {
                if (!canConvertToNullPtr(*singleInit->exp)) {
                    m_valid = false;
                    return;
                }
                auto typeExpr = std::make_unique<Parsing::VarType>(Type::U64);
                varDecl.init = std::make_unique<Parsing::SingleInitializer>(
                    std::make_unique<Parsing::ConstExpr>(0ul, std::move(typeExpr))
                );
            }
        }
    }
    else {
        m_valid = false;
        return;
    }
    assignTypeToArithmeticUnaryExpr(varDecl);
}

void TypeResolution::assignTypeToArithmeticUnaryExpr(Parsing::VarDecl& varDecl)
{
    if (varDecl.init->kind == Parsing::Initializer::Kind::Single) {
        auto singleInit = dynCast<Parsing::SingleInitializer>(varDecl.init.get());
        if (singleInit->exp->kind == Parsing::Expr::Kind::Constant &&
            varDecl.type->type != singleInit->exp->type->type) {
            const auto constExpr = dynCast<const Parsing::ConstExpr>(singleInit->exp.get());
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
        if (varDecl.type->type != singleInit->exp->type->type) {
            varDecl.init = std::make_unique<Parsing::SingleInitializer>(
                std::make_unique<Parsing::CastExpr>(
                std::make_unique<Parsing::VarType>(varDecl.type->type),
                std::move(singleInit->exp)));
        }
    }
}

void TypeResolution::visit(Parsing::SingleInitializer& singleInitializer)
{
    singleInitializer.exp = convertArrayType(*singleInitializer.exp);
}

void TypeResolution::visit(Parsing::CompoundInitializer& compoundInitializer)
{
    for (const auto& initializer : compoundInitializer.initializers)
        initializer->accept(*this);
}

void TypeResolution::visit(Parsing::DeclForInit& declForInit)
{
    if (hasStorageClassSpecifier(declForInit))
        m_valid = false;
    ASTTraverser::visit(declForInit);
}

void TypeResolution::visit(Parsing::ExprForInit& exprForInit)
{
    exprForInit.expression = convertArrayType(*exprForInit.expression);
}

void TypeResolution::visit(Parsing::ReturnStmt& stmt)
{
    stmt.expr = convertArrayType(*stmt.expr);
}

void TypeResolution::visit(Parsing::ExprStmt& stmt)
{
    stmt.expr = convertArrayType(*stmt.expr);
}

void TypeResolution::visit(Parsing::IfStmt& ifStmt)
{
    ifStmt.condition = convertArrayType(*ifStmt.condition);
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt)
        ifStmt.elseStmt->accept(*this);
}

void TypeResolution::visit(Parsing::CaseStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::WhileStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::DoWhileStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::ForStmt& stmt)
{
    if (stmt.init)
        stmt.init->accept(*this);
    if (stmt.condition)
        stmt.condition = convertArrayType(*stmt.condition);
    if (stmt.post)
        stmt.post = convertArrayType(*stmt.post);
    stmt.body->accept(*this);
}

void TypeResolution::visit(Parsing::SwitchStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convertArrayType(Parsing::Expr& expr)
{
    if (expr.type && expr.type->type == Type::Array && expr.kind != Parsing::Expr::Kind::AddrOf) {
        if (expr.kind != Parsing::Expr::Kind::AddrOf) {
            auto arrayType = dynCast<Parsing::ArrayType>(expr.type.get());
            auto addressOf = std::make_unique<Parsing::AddrOffExpr>(Parsing::deepCopy(expr));
            addressOf->type = std::make_unique<Parsing::PointerType>(Parsing::deepCopy(*arrayType->elementType));
            return addressOf;
        }
    }
    return convert(expr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::Expr& expr)
{
    using ExprKind = Parsing::Expr::Kind;
    switch (expr.kind) {
        case ExprKind::Constant: {
            const auto constExpr = dynCast<Parsing::ConstExpr>(&expr);
            return convert(*constExpr);
        }
        case ExprKind::Var: {
            const auto varExpr = dynCast<Parsing::VarExpr>(&expr);
            return convert(*varExpr);
        }
        case ExprKind::Cast: {
            const auto cast = dynCast<Parsing::CastExpr>(&expr);
            return convert(*cast);
        }
        case ExprKind::Unary: {
            const auto unary = dynCast<Parsing::UnaryExpr>(&expr);
            return convert(*unary);
        }
        case ExprKind::Binary: {
            const auto binary = dynCast<Parsing::BinaryExpr>(&expr);
            return convert(*binary);
        }
        case ExprKind::Assignment: {
            const auto assignment = dynCast<Parsing::AssignmentExpr>(&expr);
            return convert(*assignment);
        }
        case ExprKind::Ternary: {
            const auto ternary = dynCast<Parsing::TernaryExpr>(&expr);
            return convert(*ternary);
        }
        case ExprKind::FunctionCall: {
            const auto functionCall = dynCast<Parsing::FuncCallExpr>(&expr);
            return convert(*functionCall);
        }
        case ExprKind::Dereference: {
            const auto deref = dynCast<Parsing::DereferenceExpr>(&expr);
            return convert(*deref);
        }
        case ExprKind::AddrOf: {
            const auto addrOf = dynCast<Parsing::AddrOffExpr>(&expr);
            return convert(*addrOf);
        }
        case ExprKind::Subscript: {
            const auto subscript = dynCast<Parsing::SubscriptExpr>(&expr);
            return convert(*subscript);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::ConstExpr& expr)
{
    return deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::FuncCallExpr& funCallExpr)
{
    const auto it = m_functions.find(funCallExpr.name);
    if (it == m_functions.end()) {
        m_valid = false;
        return Parsing::deepCopy(funCallExpr);
    }
    if (it->second.paramTypes.size() != funCallExpr.args.size()) {
        m_valid = false;
        return Parsing::deepCopy(funCallExpr);
    }

    std::vector<std::unique_ptr<Parsing::Expr>> args;
    for (const auto& arg : funCallExpr.args)
        args.push_back(convertArrayType(*arg));
    funCallExpr.args = std::move(args);

    if (!m_valid)
        return Parsing::deepCopy(funCallExpr);
    for (size_t i = 0; i < funCallExpr.args.size(); ++i) {
        const Type typeInner = funCallExpr.args[i]->type->type;
        const Type castTo = it->second.paramTypes[i]->type;
        if (typeInner == Type::Pointer && castTo == Type::Pointer) {
            if (!Parsing::areEquivalent(*funCallExpr.args[i]->type, *it->second.paramTypes[i])) {
                m_valid = false;
                return Parsing::deepCopy(funCallExpr);
            }
        }
        if (castTo != Type::Pointer && typeInner == Type::Pointer) {
            m_valid = false;
            return Parsing::deepCopy(funCallExpr);
        }
        if (typeInner != castTo)
            funCallExpr.args[i] = std::make_unique<Parsing::CastExpr>(
                Parsing::deepCopy(*it->second.paramTypes[i]), std::move(funCallExpr.args[i]));
    }
    return Parsing::deepCopy(funCallExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    ASTTraverser::visit(varExpr);
    return Parsing::deepCopy(varExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::UnaryExpr& unaryExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;

    unaryExpr.operand = convertArrayType(*unaryExpr.operand);

    if (!m_valid)
        return Parsing::deepCopy(unaryExpr);

    if (unaryExpr.operand->type->type == Type::Double) {
        if (unaryExpr.op == Operator::Complement) {
            m_valid = false;
            return Parsing::deepCopy(unaryExpr);
        }
    }
    if (unaryExpr.operand->type->type == Type::Pointer) {
        if (isIllegalUnaryPointerOperator(unaryExpr.op)) {
            m_valid = false;
            return Parsing::deepCopy(unaryExpr);
        }
    }
    if (unaryExpr.op == Operator::Not)
        unaryExpr.type = std::make_unique<Parsing::VarType>(s_boolType);
    else
        unaryExpr.type = Parsing::deepCopy(*unaryExpr.operand->type);
    return Parsing::deepCopy(unaryExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::BinaryExpr& binaryExpr)
{
    binaryExpr.lhs = convertArrayType(*binaryExpr.lhs);
    binaryExpr.rhs = convertArrayType(*binaryExpr.rhs);

    if (!m_valid)
        return Parsing::deepCopy(binaryExpr);
    if (binaryExpr.op == BinaryOp::And || binaryExpr.op == BinaryOp::Or) {
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return Parsing::deepCopy(binaryExpr);
    }
    const Type leftType = binaryExpr.lhs->type->type;
    const Type rightType = binaryExpr.rhs->type->type;
    const Type commonType = getCommonType(leftType, rightType);
    if (commonType == Type::Double && (isBinaryBitwise(binaryExpr.op) || binaryExpr.op == BinaryOp::Modulo)) {
        m_valid = false;
        return Parsing::deepCopy(binaryExpr);
    }
    if (leftType == Type::Pointer || rightType == Type::Pointer) {
        if (binaryExpr.op == BinaryOp::Modulo || binaryExpr.op == BinaryOp::Multiply ||
            binaryExpr.op == BinaryOp::Divide ||
            binaryExpr.op == BinaryOp::BitwiseOr || binaryExpr.op == BinaryOp::BitwiseXor) {
            m_valid = false;
            return Parsing::deepCopy(binaryExpr);
        }
        if (!areValidNonArithmeticTypesInBinaryExpr(binaryExpr)) {
            m_valid = false;
            return Parsing::deepCopy(binaryExpr);
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
        if (binaryExpr.op == BinaryOp::Equal || binaryExpr.op == BinaryOp::NotEqual) {
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
            return Parsing::deepCopy(binaryExpr);
        }
        m_valid = false;
        return Parsing::deepCopy(binaryExpr);
    }
    assignTypeToArithmeticBinaryExpr(binaryExpr);
    return Parsing::deepCopy(binaryExpr);
}

bool areValidNonArithmeticTypesInBinaryExpr(const Parsing::BinaryExpr& binaryExpr)
{
    if (Parsing::areEquivalent(*binaryExpr.lhs->type, *binaryExpr.rhs->type))
        return true;
    if (canConvertToNullPtr(*binaryExpr.lhs) || canConvertToNullPtr(*binaryExpr.rhs))
        return true;
    return false;
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::AssignmentExpr& assignmentExpr)
{
    assignmentExpr.lhs = convertArrayType(*assignmentExpr.lhs);
    assignmentExpr.rhs = convertArrayType(*assignmentExpr.rhs);

    if (!m_valid)
        return Parsing::deepCopy(assignmentExpr);
    const Type leftType = assignmentExpr.lhs->type->type;
    const Type rightType = assignmentExpr.rhs->type->type;
    if (!isLegalAssignExpr(assignmentExpr)) {
        m_valid = false;
        return Parsing::deepCopy(assignmentExpr);
    }
    if (leftType != rightType && assignmentExpr.op == Parsing::AssignmentExpr::Operator::Assign &&
        leftType != Type::Pointer) {
        assignmentExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(leftType), std::move(assignmentExpr.rhs));
    }
    assignmentExpr.type = Parsing::deepCopy(*assignmentExpr.lhs->type);
    return Parsing::deepCopy(assignmentExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::CastExpr& castExpr)
{
    castExpr.expr = convertArrayType(*castExpr.expr);

    if (!m_valid)
        return Parsing::deepCopy(castExpr);
    const Type outerType = castExpr.type->type;
    const Type innerType = castExpr.expr->type->type;
    if ((outerType == Type::Double && innerType == Type::Pointer) ||
        (outerType == Type::Pointer && innerType == Type::Double)) {
        m_valid = false;
        return Parsing::deepCopy(castExpr);
    }
    return Parsing::deepCopy(castExpr);
}

bool isLegalAssignExpr(Parsing::AssignmentExpr& assignmentExpr)
{
    const Type leftType = assignmentExpr.lhs->type->type;
    const Type rightType = assignmentExpr.rhs->type->type;
    const Type commonType = getCommonType(leftType, rightType);
    if ((leftType == Type::Pointer || rightType == Type::Pointer) &&
        isIllegalPointerCompoundAssignOperation(assignmentExpr.op)) {
        return false;
    }
    if (commonType == Type::Double && isIllegalDoubleCompoundAssignOperation(assignmentExpr.op))
        return false;
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

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::TernaryExpr& ternaryExpr)
{
    ternaryExpr.condition = convertArrayType(*ternaryExpr.condition);
    ternaryExpr.trueExpr = convertArrayType(*ternaryExpr.trueExpr);
    ternaryExpr.falseExpr = convertArrayType(*ternaryExpr.falseExpr);

    const Type trueType = ternaryExpr.trueExpr->type->type;
    const Type falseType = ternaryExpr.falseExpr->type->type;
    const Type commonType = getCommonType(trueType, falseType);
    if (trueType == Type::Pointer || falseType == Type::Pointer) {
        if (trueType == Type::Pointer && falseType == Type::Pointer) {
            if (!Parsing::areEquivalent(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
                m_valid = false;
            ternaryExpr.type = std::move(Parsing::deepCopy(*ternaryExpr.trueExpr->type));
            return Parsing::deepCopy(ternaryExpr);
        }
        if (!areValidNonArithmeticTypesInTernaryExpr(ternaryExpr)) {
            m_valid = false;
            return Parsing::deepCopy(ternaryExpr);
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
        return Parsing::deepCopy(ternaryExpr);
    }
    if (commonType != trueType)
        ternaryExpr.trueExpr = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(ternaryExpr.trueExpr));
    if (commonType != falseType)
        ternaryExpr.falseExpr = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(ternaryExpr.falseExpr));
    ternaryExpr.type = std::make_unique<Parsing::VarType>(commonType);
    return Parsing::deepCopy(ternaryExpr);
}

bool areValidNonArithmeticTypesInTernaryExpr(const Parsing::TernaryExpr& ternaryExpr)
{
    if (Parsing::areEquivalent(*ternaryExpr.trueExpr->type, *ternaryExpr.falseExpr->type))
        return true;
    if (canConvertToNullPtr(*ternaryExpr.trueExpr) || canConvertToNullPtr(*ternaryExpr.falseExpr))
        return true;
    return false;
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::AddrOffExpr& addrOffExpr)
{
    addrOffExpr.reference = convertArrayType(*addrOffExpr.reference);

    if (!m_valid)
        return Parsing::deepCopy(addrOffExpr);
    if (addrOffExpr.reference->kind == Parsing::Expr::Kind::AddrOf) {
        m_valid = false;
        return Parsing::deepCopy(addrOffExpr);
    }
    addrOffExpr.type = std::make_unique<Parsing::PointerType>(
        Parsing::deepCopy(*addrOffExpr.reference->type));
    return Parsing::deepCopy(addrOffExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::DereferenceExpr& dereferenceExpr)
{
    dereferenceExpr.reference = convertArrayType(*dereferenceExpr.reference);

    if (!m_valid)
        return Parsing::deepCopy(dereferenceExpr);
    if (dereferenceExpr.reference->type->type != Type::Pointer) {
        m_valid = false;
        return Parsing::deepCopy(dereferenceExpr);
    }
    const auto referencedPtrType = dynCast<const Parsing::PointerType>(dereferenceExpr.reference->type.get());
    dereferenceExpr.type = Parsing::deepCopy(*referencedPtrType->referenced);
    return Parsing::deepCopy(dereferenceExpr);
}

std::unique_ptr<Parsing::Expr> TypeResolution::convert(Parsing::SubscriptExpr& subscriptExpr)
{
    subscriptExpr.referencing = convertArrayType(*subscriptExpr.referencing);
    subscriptExpr.index = convertArrayType(*subscriptExpr.index);

    if (!m_valid)
        return Parsing::deepCopy(subscriptExpr);

    const Type referencingType = subscriptExpr.referencing->type->type;
    const Type indexType = subscriptExpr.index->type->type;

    if (referencingType == Type::Pointer && isInteger(indexType)) {
        if (indexType != Type::I64) {
            subscriptExpr.index = std::make_unique<Parsing::CastExpr>(
                std::make_unique<Parsing::VarType>(Type::I64), std::move(subscriptExpr.index));
        }
    }
    else if (indexType == Type::Pointer && isInteger(referencingType)) {
        if (referencingType != Type::I64) {
            subscriptExpr.referencing = std::make_unique<Parsing::CastExpr>(
                std::make_unique<Parsing::VarType>(Type::I64), std::move(subscriptExpr.referencing));
        }
        auto result = std::make_unique<Parsing::SubscriptExpr>(subscriptExpr.location,
            std::move(subscriptExpr.index), std::move(subscriptExpr.referencing));
        subscriptExpr.type = deepCopy(*subscriptExpr.referencing->type);
        return result;
    }
    else {
        m_valid = false;
        return Parsing::deepCopy(subscriptExpr);
    }

    subscriptExpr.type = deepCopy(*subscriptExpr.referencing->type);
    return Parsing::deepCopy(subscriptExpr);
}

bool TypeResolution::isIllegalVarDecl(const Parsing::VarDecl& varDecl) const
{
    if (varDecl.storage == Storage::Extern && varDecl.init != nullptr)
        return true;
    if (varDecl.storage == Storage::Static && m_definedFunctions.contains(varDecl.name))
        return true;
    return false;
}
} // namespace Semantics