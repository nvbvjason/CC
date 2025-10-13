#pragma once

#include "ASTTraverser.hpp"

namespace Semantics {

class DeSugarDeref : public Parsing::ASTTraverser {
    Parsing::VarExpr* left = nullptr;
    Parsing::VarExpr* right = nullptr;
public:
    void deSugar(Parsing::Program& program);

    void visit(Parsing::Block& block) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
};

} // Semantics