#pragma once

#ifndef CC_SEMANTICS_REMOVE_REDUNDANT_DECLS_HPP
#define CC_SEMANTICS_REMOVE_REDUNDANT_DECLS_HPP

#include "ASTParser.hpp"

#include <string>
#include <unordered_map>

namespace Semantics {

class RemoveRedundantDecls {
    std::unordered_map<std::string, bool> functionsInternalLinkage;
    std::unordered_map<std::string, bool> varInternalLinkage;
public:
    void handleFunctions(std::unique_ptr<Parsing::Declaration>& decl);
    void handleVarDecl(const std::unique_ptr<Parsing::Declaration>& decl);
    void go(Parsing::Program& prog);
};

} // Semantics

#endif // CC_SEMANTICS_REMOVE_REDUNDANT_DECLS_HPP
