#include "Frontend/Semantics/TypeResolution.hpp"

namespace Semantics {
bool TypeResolution::validate(const Parsing::Program& program)
{
    m_valid = true;
    m_storageClassMap.clear();
    ConstASTTraverser::visit(program);
    return m_valid;
}

void TypeResolution::visit(const Parsing::FunDecl& funDecl)
{
    if (!m_storageClassMap.contains(funDecl.name))
        m_storageClassMap[funDecl.name] = funDecl.storage;
    if (!m_atFileScope && funDecl.body != nullptr) {
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
    m_atFileScope = false;
    ConstASTTraverser::visit(funDecl);
    m_atFileScope = true;
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
    if (varDecl.storage == StorageClass::Extern &&
        varDecl.init != nullptr) {
        m_valid = false;
        return;
    }
    m_isConst = true;
    ConstASTTraverser::visit(varDecl);
    if (illegalNonConstInitialization(varDecl, m_isConst, m_atFileScope))
        m_valid = false;
}

void TypeResolution::visit(const Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    ConstASTTraverser::visit(varExpr);
}

} // namespace Semantics