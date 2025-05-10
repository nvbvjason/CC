#pragma once

#ifndef CC_SEMANTICS_TYPE_RESOLUTION_HPP
#define CC_SEMANTICS_TYPE_RESOLUTION_HPP
#include <string>
#include <unordered_map>

#include "ASTParser.hpp"
#include "ConstASTTraverser.hpp"

namespace Semantics {

class TypeResolution : public Parsing::ConstASTTraverser {
    bool m_valid;
    std::unordered_map<std::string, Parsing::Declaration::StorageClass> m_storageClassMap;
public:
    bool validate(const Parsing::Program& program);
    void visit(const Parsing::FunDecl& funDecl) override;
};

} // Semantics

#endif // CC_SEMANTICS_TYPE_RESOLUTION_HPP
