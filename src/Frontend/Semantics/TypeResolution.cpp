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
        m_storageClassMap[funDecl.name] = funDecl.storageClass;
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
    m_isConst = true;
    ConstASTTraverser::visit(varDecl);
    if (!m_isConst && (varDecl.storageClass == StorageClass::StaticGlobal ||
                       varDecl.storageClass == StorageClass::StaticLocal ||
                       varDecl.storageClass == StorageClass::ExternGlobalInitialized ||
                       varDecl.storageClass == StorageClass::AutoGlobalScope ||
                       varDecl.storageClass == StorageClass::ExternGlobalInitialized))
        m_valid = false;
}

void TypeResolution::visit(const Parsing::VarExpr& varExpr)
{
    m_isConst = false;
    ConstASTTraverser::visit(varExpr);
}
} // Semantics