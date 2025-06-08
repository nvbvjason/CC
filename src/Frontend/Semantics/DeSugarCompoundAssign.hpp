#pragma once

#ifndef CC_SEMANTICS_DESUGAR_COMPOUND_ASSIGN_HPP
#define CC_SEMANTICS_DESUGAR_COMPOUND_ASSIGN_HPP

#include "ASTTraverser.hpp"

namespace Semantics {

class DeSugarCompoundAssign : public Parsing::ASTTraverser{
public:
    void deSugar(Parsing::Program& program);

    void visit(Parsing::AssignmentExpr& assignmentExpr);
};

} // Semantics

#endif // CC_SEMANTICS_DESUGAR_COMPOUND_ASSIGN_HPP
