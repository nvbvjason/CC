#include "TypeResolution.hpp"

#include "ASTIr.hpp"
#include "ASTTypes.hpp"
#include "TypeConversion.hpp"

namespace Semantics {
bool TypeResolution::validate(Parsing::Program& program)
{
    ASTTraverser::visit(program);
    return m_valid;
}

bool TypeResolution::hasConflictingFuncLinkage(const Parsing::FunDecl& funDecl)
{
    const auto it = m_storageClassesFuncs.find(funDecl.name);
    if (it == m_storageClassesFuncs.end())
        return false;
    return (it->second == Storage::Extern ||
            it->second == Storage::None) && funDecl.storage == Storage::Static;
}

void TypeResolution::visit(Parsing::FunDecl& funDecl)
{
    if (hasConflictingFuncLinkage(funDecl)) {
        m_valid = false;
        return;
    }
    if (!m_storageClassesFuncs.contains(funDecl.name))
        m_storageClassesFuncs[funDecl.name] = funDecl.storage;
    if (!m_global && funDecl.body != nullptr) {
        m_valid = false;
        return;
    }
    const auto it = m_functionArgCounts.find(funDecl.name);
    if (it == m_functionArgCounts.end())
        m_functionArgCounts.emplace_hint(it, funDecl.name, funDecl.params.size());
    else if (it->second != funDecl.params.size()) {
        m_valid = false;
        return;
    }
    if (funDecl.body != nullptr)
        m_definedFunctions.insert(funDecl.name);
    m_global = false;
    const auto type = static_cast<Parsing::FuncType*>(funDecl.type.get());
    m_returnTypesFuncs.emplace(funDecl.name, type->returnType->kind);
    ASTTraverser::visit(funDecl);
    m_global = true;
}

void TypeResolution::visit(Parsing::DeclForInit& declForInit)
{
    if (hasStorageClassSpecifier(declForInit))
        m_valid = false;
    ASTTraverser::visit(declForInit);
}

void TypeResolution::visit(Parsing::FunCallExpr& funCallExpr)
{
    const auto it = m_functionArgCounts.find(funCallExpr.name);
    if (it == m_functionArgCounts.end()) {
        m_valid = false;
        return;
    }
    if (it->second != funCallExpr.args.size()) {
        m_valid = false;
        return;
    }
    ASTTraverser::visit(funCallExpr);
    funCallExpr.type = std::make_unique<Parsing::VarType>(m_returnTypesFuncs[funCallExpr.name]);
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
    if (illegalNonConstInitialization(varDecl, m_isConst, m_global))
        m_valid = false;
}

void TypeResolution::visit(Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    ASTTraverser::visit(varExpr);
}

void TypeResolution::visit(Parsing::UnaryExpr& unaryExpr)
{
    ASTTraverser::visit(unaryExpr);
    if (unaryExpr.op == Parsing::UnaryExpr::Operator::Not) {
        unaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return;
    }
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
    if (commonType != leftType)
        binaryExpr.lhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.lhs));
    if (commonType != rightType)
        binaryExpr.rhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(commonType), std::move(binaryExpr.rhs));
    if (binaryExpr.op == Oper::Equal || binaryExpr.op == Oper::NotEqual ||
        binaryExpr.op == Oper::LessThan || binaryExpr.op == Oper::LessOrEqual ||
        binaryExpr.op == Oper::GreaterThan || binaryExpr.op == Oper::GreaterOrEqual) {
        binaryExpr.type = std::make_unique<Parsing::VarType>(Type::I32);
        return;
    }
    binaryExpr.type = std::make_unique<Parsing::VarType>(commonType);
}

void TypeResolution::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    ASTTraverser::visit(assignmentExpr);
    const Type leftType = assignmentExpr.lhs->type->kind;
    const Type rightType = assignmentExpr.rhs->type->kind;
    if (leftType != rightType)
        assignmentExpr.lhs = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(rightType), std::move(assignmentExpr.lhs));
    assignmentExpr.type = std::make_unique<Parsing::VarType>(rightType);
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