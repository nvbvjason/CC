#pragma once

#ifndef CC_SEMANTICS_DESUGAR_COMPOUND_ASSIGN_HPP
#define CC_SEMANTICS_DESUGAR_COMPOUND_ASSIGN_HPP

#include <memory>

#include "ASTTraverser.hpp"

namespace Semantics {

class DeSugar : public Parsing::ASTTraverser{
public:
    void deSugar(Parsing::Program& program);

    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
};

std::unique_ptr<Parsing::Expr> deepCopyDeref(const Parsing::Expr& derefExpr);
} // Semantics
#endif // CC_SEMANTICS_DESUGAR_COMPOUND_ASSIGN_HPP