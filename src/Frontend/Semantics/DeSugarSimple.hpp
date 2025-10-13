#pragma once

#include "ASTTraverser.hpp"

namespace Semantics {

class DeSugarSimple : public Parsing::ASTTraverser {
public:
    void deSugar(Parsing::Program& program);

    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
};

} // Semantics