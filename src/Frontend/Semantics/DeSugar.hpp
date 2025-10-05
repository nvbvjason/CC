#pragma once

#include <memory>

#include "ASTTraverser.hpp"

namespace Semantics {

class DeSugar : public Parsing::ASTTraverser{
public:
    void deSugar(Parsing::Program& program);

    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
};

std::unique_ptr<Parsing::Expr> deepCopy(const Parsing::Expr& derefExpr);
} // Semantics