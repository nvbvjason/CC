#include "TypeResolution.hpp"
#include "ASTIr.hpp"
#include "TypeConversion.hpp"

namespace Semantics {
bool TypeResolution::validate(Parsing::Program& program)
{
    ASTTraverser::visit(program);
    return m_valid;
}

void TypeResolution::visit(Parsing::FunDecl& funDecl)
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
    const auto type = static_cast<Parsing::FuncType*>(funDecl.type.get());
    FuncEntry funcEntry(type->params, type->returnType->kind, funDecl.storage,
        funDecl.body != nullptr);
    m_functions.emplace_hint(it, funDecl.name, std::move(funcEntry));
    ASTTraverser::visit(funDecl);
    m_global = true;
}

bool TypeResolution::validFuncDecl(const FuncEntry& funcEntry, const Parsing::FunDecl& funDecl)
{
    if ((funcEntry.storage == Storage::Extern || funcEntry.storage == Storage::None)
        && funDecl.storage == Storage::Static)
        return false;
    if (funDecl.params.size() != funcEntry.paramTypes.size())
        return false;
    const auto funcType = static_cast<Parsing::FuncType*>(funDecl.type.get());
    for (size_t i = 0; i < funDecl.params.size(); ++i)
        if (funcEntry.paramTypes[i]->kind != funcType->params[i]->kind)
            return false;
    if (funcType->returnType->kind != funcEntry.returnType)
        return false;
    return true;
}

void TypeResolution::visit(Parsing::DeclForInit& declForInit)
{
    if (hasStorageClassSpecifier(declForInit))
        m_valid = false;
    ASTTraverser::visit(declForInit);
}

void TypeResolution::visit(Parsing::FunCallExpr& funCallExpr)
{
    const auto it = m_functions.find(funCallExpr.name);
    if (it == m_functions.end()) {
        m_valid = false;
        return;
    }
    if (it->second.paramTypes.size() != funCallExpr.args.size()) {
        m_valid = false;
        return;
    }
    ASTTraverser::visit(funCallExpr);
    if (!m_valid)
        return;
    for (int i = 0; i < funCallExpr.args.size(); ++i) {
        const Type typeInner = funCallExpr.args[i]->type->kind;
        const Type castTo = it->second.paramTypes[i]->kind;
        if (typeInner == Type::Pointer || castTo == Type::Pointer) {
            if (!Parsing::areEquivalent(*funCallExpr.args[i]->type, *it->second.paramTypes[i])) {
                m_valid = false;
                return;
            }
        }
        if (typeInner != castTo)
            funCallExpr.args[i] = std::make_unique<Parsing::CastExpr>(
                std::make_unique<Parsing::VarType>(castTo), std::move(funCallExpr.args[i]));
    }
}

void TypeResolution::visit(Parsing::VarDecl& varDecl)
{
    if (varDecl.storage == Storage::Extern &&
        varDecl.init != nullptr) {
        m_valid = false;
        return;
    }
    if (varDecl.storage == Storage::Static && m_definedFunctions.contains(varDecl.name)) {
        m_valid = false;
        return;
    }
    if (m_global && varDecl.storage == Storage::Static)
        m_globalStaticVars.insert(varDecl.name);
    if (!m_global && !m_globalStaticVars.contains(varDecl.name))
        m_definedFunctions.insert(varDecl.name);
    m_isConst = true;
    ASTTraverser::visit(varDecl);
    if (!m_valid)
        return;
    if (illegalNonConstInitialization(varDecl, m_isConst, m_global)) {
        m_valid = false;
        return;
    }
    if (varDecl.init == nullptr)
        return;
    if (varDecl.type->kind == Type::Pointer && !canConvertToPtr(*varDecl.init)) {
        m_valid = false;
        return;
    }
    if (varDecl.init->kind == Parsing::Expr::Kind::Constant &&
        varDecl.type->kind != varDecl.init->type->kind) {
        const auto constExpr = static_cast<const Parsing::ConstExpr*>(varDecl.init.get());
        if (varDecl.type->kind == Type::U32)
            convertConstantExpr<u32, Type::U32>(varDecl, *constExpr);
        else if (varDecl.type->kind == Type::U64)
            convertConstantExpr<u64, Type::U64>(varDecl, *constExpr);
        else if (varDecl.type->kind == Type::I32)
            convertConstantExpr<i32, Type::I32>(varDecl, *constExpr);
        else if (varDecl.type->kind == Type::I64)
            convertConstantExpr<i64, Type::I64>(varDecl, *constExpr);
        else if (varDecl.type->kind == Type::Double)
            convertConstantExpr<double, Type::Double>(varDecl, *constExpr);
    }
    else if (varDecl.type->kind == Type::Pointer)
        return;
    else {
        if (varDecl.type->kind != varDecl.init->type->kind) {
            varDecl.init = std::make_unique<Parsing::CastExpr>(
                std::make_unique<Parsing::VarType>(varDecl.type->kind),
                std::move(varDecl.init));
        }
    }
}

void TypeResolution::visit(Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    ASTTraverser::visit(varExpr);
}

void TypeResolution::visit(Parsing::UnaryExpr& unaryExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;
    ASTTraverser::visit(unaryExpr);
    if (!m_valid)
        return;
    if (unaryExpr.operand->type->kind == Type::Double) {
        if (unaryExpr.op == Operator::Complement) {
            m_valid = false;
            return;
        }
    }
    if (unaryExpr.operand->type->kind == Type::Pointer) {
        if (unaryExpr.op == Operator::Complement ||
            unaryExpr.op == Operator::PrefixDecrement ||
            unaryExpr.op == Operator::PrefixIncrement ||
            unaryExpr.op == Operator::PostFixDecrement ||
            unaryExpr.op == Operator::PostFixIncrement ||
            unaryExpr.op == Operator::Negate) {
            m_valid = false;
            return;
        }
    }
    if (unaryExpr.op == Operator::Not)
        unaryExpr.type = std::make_unique<Parsing::VarType>(s_boolType);
    else
        unaryExpr.type = std::make_unique<Parsing::VarType>(unaryExpr.operand->type->kind);
}

void TypeResolution::assignTypeToSimpleBinaryExpr(
    Parsing::BinaryExpr& binaryExpr,
    const Type leftType, const Type rightType, const Type commonType)
{
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::And ||
        binaryExpr.op == Parsing::BinaryExpr::Operator::Or) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return;
    }
    if (commonType != leftType)
        binaryExpr.lhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.lhs));
    if (commonType != rightType)
        binaryExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.rhs));
    if (binaryExpr.op == Parsing::BinaryExpr::Operator::LeftShift ||
        binaryExpr.op == Parsing::BinaryExpr::Operator::RightShift) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(leftType);
        return;
    }
    if (isBinaryComparison(binaryExpr)) {
        if (commonType == Type::Double)
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        else if (isSigned(commonType))
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        else
            binaryExpr.type = std::make_unique<Parsing::VarType>(Type::U32);
        return;
    }
    binaryExpr.type = std::make_unique<Parsing::VarType>(commonType);
}

void TypeResolution::visit(Parsing::BinaryExpr& binaryExpr)
{
    using Oper = Parsing::BinaryExpr::Operator;
    ASTTraverser::visit(binaryExpr);
    if (!m_valid)
        return;
    const Type leftType = binaryExpr.lhs->type->kind;
    const Type rightType = binaryExpr.rhs->type->kind;
    const Type commonType = getCommonType(leftType, rightType);
    if (commonType == Type::Double && (isBinaryBitwise(binaryExpr) || binaryExpr.op == Oper::Modulo)) {
        m_valid = false;
        return;
    }
    if (leftType == Type::Pointer || rightType == Type::Pointer) {
        if (binaryExpr.op == Oper::Modulo || binaryExpr.op == Oper::Multiply ||
            binaryExpr.op == Oper::Divide ||
            binaryExpr.op == Oper::BitwiseOr || binaryExpr.op == Oper::BitwiseXor) {
            m_valid = false;
            return;
        }
        // if (Parsing::areEquivalent(*binaryExpr.lhs->type, *binaryExpr.rhs->type))
        //     return;
        m_valid = false;
        return;
    }
    assignTypeToSimpleBinaryExpr(binaryExpr, leftType, rightType, commonType);
}

void TypeResolution::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    ASTTraverser::visit(assignmentExpr);
    if (!m_valid)
        return;
    const Type leftType = assignmentExpr.lhs->type->kind;
    const Type rightType = assignmentExpr.rhs->type->kind;
    if (isIllegalAssignExpr(assignmentExpr)) {
        m_valid = false;
        return;
    }
    if (leftType != rightType)
        assignmentExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(leftType), std::move(assignmentExpr.rhs));
    assignmentExpr.type = std::make_unique<Parsing::VarType>(leftType);
}

void TypeResolution::visit(Parsing::CastExpr& castExpr)
{
    ASTTraverser::visit(castExpr);
    if (!m_valid)
        return;
    const Type outerType = castExpr.type->kind;
    const Type innerType = castExpr.expr->type->kind;
    if ((outerType == Type::Double && innerType == Type::Pointer) ||
        (outerType == Type::Pointer && innerType == Type::Double)) {
        m_valid = false;
        return;
    }
}

bool isIllegalAssignExpr(const Parsing::AssignmentExpr& assignmentExpr)
{
    const Type leftType = assignmentExpr.lhs->type->kind;
    const Type rightType = assignmentExpr.rhs->type->kind;
    if (leftType == Type::Pointer && rightType == Type::I32)
        return true;
    if (leftType == Type::Pointer || rightType == Type::Pointer) {
        if (!Parsing::areEquivalent(*assignmentExpr.lhs->type, *assignmentExpr.rhs->type))
            return true;
    }
    return false;
}

void TypeResolution::visit(Parsing::TernaryExpr& ternaryExpr)
{
    ASTTraverser::visit(ternaryExpr);
    const Type trueType = ternaryExpr.trueExpr->type->kind;
    const Type falseType = ternaryExpr.falseExpr->type->kind;
    const Type commonType = getCommonType(trueType, falseType);
    if (trueType == Type::Pointer || falseType == Type::Pointer) {
        m_valid = false;
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

void TypeResolution::visit(Parsing::AddrOffExpr& addrOffExpr)
{
    ASTTraverser::visit(addrOffExpr);
    if (!m_valid)
        return;
    if (addrOffExpr.reference->type->kind == Type::Pointer) {
        m_valid = false;
        return;
    }
    addrOffExpr.type = std::make_unique<Parsing::PointerType>(
        Parsing::deepCopy(*addrOffExpr.reference->type));
}

void TypeResolution::visit(Parsing::DereferenceExpr& dereferenceExpr)
{
    ASTTraverser::visit(dereferenceExpr);
    if (!m_valid)
        return;
    if (dereferenceExpr.reference->type->kind != Type::Pointer) {
        m_valid = false;
        return;
    }
    dereferenceExpr.type = Parsing::deepCopy(*dereferenceExpr.reference->type);
}

std::unique_ptr<Parsing::TypeBase> getCommonType(
    const std::unique_ptr<Parsing::TypeBase>& leftType,
    const std::unique_ptr<Parsing::TypeBase>& rightType)
{
    return {};
}
} // namespace Semantics