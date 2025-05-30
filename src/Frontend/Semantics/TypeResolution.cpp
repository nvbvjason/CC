#include "Frontend/Semantics/TypeResolution.hpp"

namespace Semantics {
bool TypeResolution::validate(const Parsing::Program& program)
{
    ConstASTTraverser::visit(program);
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

void TypeResolution::visit(const Parsing::FunDecl& funDecl)
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
    ConstASTTraverser::visit(funDecl);
    m_global = true;
}

void TypeResolution::visit(const Parsing::DeclForInit& declForInit)
{
    if (hasStorageClassSpecifier(declForInit))
        m_valid = false;
    ConstASTTraverser::visit(declForInit);
}

void TypeResolution::visit(const Parsing::FunCallExpr& funCallExpr)
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
    ConstASTTraverser::visit(funCallExpr);
}

void TypeResolution::visit(const Parsing::VarDecl& varDecl)
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
    ConstASTTraverser::visit(varDecl);
    if (illegalNonConstInitialization(varDecl, m_isConst, m_global))
        m_valid = false;
}

void TypeResolution::visit(const Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    ConstASTTraverser::visit(varExpr);
}

} // namespace Semantics