#include "DeSugarSimple.hpp"
#include "ASTExpr.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "Operators.hpp"
#include "Utils.hpp"

namespace Semantics {
void DeSugarSimple::deSugar(Parsing::Program& program)
{
    ASTTraverser::visit(program);
}

void DeSugarSimple::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    if (assignmentExpr.op != Parsing::AssignmentExpr::Operator::Assign &&
        assignmentExpr.lhs->kind == Parsing::Expr::Kind::Var) {
        const auto lhs = dynCast<Parsing::VarExpr>(assignmentExpr.lhs.get());
        const Parsing::BinaryExpr::Operator op = Parsing::Operators::getBinaryOperator(assignmentExpr.op);
        std::unique_ptr<Parsing::Expr> leftCopy = deepCopy(*lhs);
        if (lhs->type != nullptr)
            leftCopy->type = Parsing::deepCopy(*assignmentExpr.lhs->type);
        assignmentExpr.op = Parsing::AssignmentExpr::Operator::Assign;
        assignmentExpr.rhs = std::make_unique<Parsing::BinaryExpr>(
            op, std::move(leftCopy), std::move(assignmentExpr.rhs));
    }
    ASTTraverser::visit(assignmentExpr);
}
} // Semantics