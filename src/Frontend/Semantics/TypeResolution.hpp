#pragma once

#ifndef CC_SEMANTICS_TYPE_RESOLUTION_HPP
#define CC_SEMANTICS_TYPE_RESOLUTION_HPP

#include "ASTParser.hpp"
#include "ConstASTTraverser.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Semantics {

class TypeResolution : public Parsing::ConstASTTraverser {
public:
    bool validate(const Parsing::Program& program);
};

} // Semantics

#endif // CC_SEMANTICS_TYPE_RESOLUTION_HPP
