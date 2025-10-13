#include "DeSugarDeref.hpp"
#include "ASTExpr.hpp"
#include "ASTTypes.hpp"
#include "Operators.hpp"
#include "Utils.hpp"

namespace Semantics {
void DeSugarDeref::deSugar(Parsing::Program& program)
{
    ASTTraverser::visit(program);
}

void DeSugarDeref::visit(Parsing::Block& block)
{
    std::vector<std::unique_ptr<Parsing::BlockItem>>& body = block.body;
    for (auto it = body.begin(); it != body.end(); ++it) {
        auto blockItem = it->get();
        blockItem->accept(*this);
    }
}

void DeSugarDeref::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    if (assignmentExpr.op != Parsing::AssignmentExpr::Operator::Assign &&
         assignmentExpr.lhs->kind == Parsing::Expr::Kind::Dereference) {
        std::unique_ptr<Parsing::Expr> leftCopy = deepCopy(*assignmentExpr.lhs);
        leftCopy->type = Parsing::deepCopy(*assignmentExpr.lhs->type);
        const Parsing::BinaryExpr::Operator op = Parsing::Operators::getBinaryOperator(assignmentExpr.op);
        assignmentExpr.op = Parsing::AssignmentExpr::Operator::Assign;
        auto rightHandSide = std::make_unique<Parsing::BinaryExpr>(
            op, std::move(leftCopy), std::move(assignmentExpr.rhs));
        assignTypeToArithmeticBinaryExpr(*rightHandSide);
        assignmentExpr.rhs = std::move(rightHandSide);
        if (assignmentExpr.lhs->type->type != assignmentExpr.rhs->type->type) {
            assignmentExpr.rhs = std::make_unique<Parsing::CastExpr>(
                Parsing::deepCopy(*assignmentExpr.lhs->type), std::move(assignmentExpr.rhs));
        }
    }
}

} // Semantics