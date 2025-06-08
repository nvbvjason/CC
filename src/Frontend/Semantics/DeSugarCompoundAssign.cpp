#include "DeSugarCompoundAssign.hpp"
#include "ASTExpr.hpp"
#include "Operators.hpp"

namespace Semantics {
void DeSugarCompoundAssign::deSugar(Parsing::Program& program)
{
    ASTTraverser::visit(program);
}

void DeSugarCompoundAssign::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    if (assignmentExpr.op != Parsing::AssignmentExpr::Operator::Assign &&
        assignmentExpr.lhs->kind == Parsing::Expr::Kind::Var) {
        const auto lhs = static_cast<Parsing::VarExpr*>(assignmentExpr.lhs.get());
        const Parsing::BinaryExpr::Operator op = Parsing::Operators::getBinaryOperator(assignmentExpr.op);
        auto leftCopy = std::make_unique<Parsing::VarExpr>(lhs->name);
        if (lhs->type != nullptr)
            leftCopy->type = std::make_unique<Parsing::VarType>(lhs->type->kind);
        assignmentExpr.op = Parsing::AssignmentExpr::Operator::Assign;
        assignmentExpr.rhs = std::make_unique<Parsing::BinaryExpr>(
            op, std::move(leftCopy), std::move(assignmentExpr.rhs));
    }
    ASTTraverser::visit(assignmentExpr);
}
} // Semantics