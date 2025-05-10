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
    auto entry = m_storageClassMap.find(funDecl.name);
    if (entry == m_storageClassMap.end())
        m_storageClassMap.insert({funDecl.name, funDecl.storageClass});
    if (entry->second != funDecl.storageClass) {
        m_valid = false;
        return;
    }
    ConstASTTraverser::visit(funDecl);
}
} // Semantics