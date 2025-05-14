#include "TypeResolution.hpp"

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

void TypeResolution::visit(const Parsing::VarDecl& varDecl)
{
    if (varDecl.storage == StorageClass::ExternLocal &&
        varDecl.init != nullptr) {
        m_valid = false;
        return;
    }
    m_isConst = true;
    ConstASTTraverser::visit(varDecl);
    if (mustBeConstantInitialised(varDecl, m_isConst))
        m_valid = false;
}

void TypeResolution::visit(const Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    ConstASTTraverser::visit(varExpr);
}
} // Semantics