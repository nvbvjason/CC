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
    FuncEntry funcEntry(type->params, type->returnType->kind, funDecl.storage, funDecl.body != nullptr);
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
        if (funcEntry.paramTypes[i] != funcType->params[i]->kind)
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
    funCallExpr.type = std::make_unique<Parsing::VarType>(it->second.returnType);
    ASTTraverser::visit(funCallExpr);
    for (int i = 0; i < funCallExpr.args.size(); ++i) {
        const Type typeInner = funCallExpr.args[i]->type->kind;
        const Type castTo = it->second.paramTypes[i];
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
    ASTTraverser::visit(unaryExpr);
    if (unaryExpr.operand->type->kind == Type::Double) {
        if (unaryExpr.op == Parsing::UnaryExpr::Operator::Complement) {
            m_valid = false;
            return;
        }
    }
    if (unaryExpr.op == Parsing::UnaryExpr::Operator::Not)
        unaryExpr.type = std::make_unique<Parsing::VarType>(s_boolType);
    else
        unaryExpr.type = std::make_unique<Parsing::VarType>(unaryExpr.operand->type->kind);
}

void TypeResolution::visit(Parsing::BinaryExpr& binaryExpr)
{
    using Oper = Parsing::BinaryExpr::Operator;
    ASTTraverser::visit(binaryExpr);
    if (binaryExpr.op == Oper::And ||
        binaryExpr.op == Oper::Or) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return;
    }
    const Type leftType = binaryExpr.lhs->type->kind;
    const Type rightType = binaryExpr.rhs->type->kind;
    const Type commonType = getCommonType(leftType, rightType);
    if (commonType == Type::Double && (isBinaryBitwise(binaryExpr) || binaryExpr.op == Oper::Modulo)) {
        m_valid = false;
        return;
    }
    if (commonType != leftType)
        binaryExpr.lhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.lhs));
    if (commonType != rightType)
        binaryExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.rhs));
    if (binaryExpr.op == Oper::LeftShift || binaryExpr.op == Oper::RightShift) {
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

void TypeResolution::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    ASTTraverser::visit(assignmentExpr);
    if (!m_valid)
        return;
    const Type leftType = assignmentExpr.lhs->type->kind;
    const Type rightType = assignmentExpr.rhs->type->kind;
    if (leftType != rightType)
        assignmentExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(leftType), std::move(assignmentExpr.rhs));
    assignmentExpr.type = std::make_unique<Parsing::VarType>(leftType);
}

void TypeResolution::visit(Parsing::TernaryExpr& ternaryExpr)
{
    ASTTraverser::visit(ternaryExpr);
    const Type trueType = ternaryExpr.trueExpr->type->kind;
    const Type falseType = ternaryExpr.falseExpr->type->kind;
    const Type commonType = getCommonType(trueType, falseType);
    if (commonType != trueType)
        ternaryExpr.trueExpr = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(ternaryExpr.trueExpr));
    if (commonType != falseType)
        ternaryExpr.falseExpr = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(ternaryExpr.falseExpr));
    ternaryExpr.type = std::make_unique<Parsing::VarType>(commonType);
}
} // namespace Semantics