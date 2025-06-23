#include "DeSugar.hpp"
#include "ASTExpr.hpp"
#include "ASTTypes.hpp"
#include "Operators.hpp"

namespace Semantics {
void DeSugar::deSugar(Parsing::Program& program)
{
    ASTTraverser::visit(program);
}

void DeSugar::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    if (assignmentExpr.op != Parsing::AssignmentExpr::Operator::Assign &&
        assignmentExpr.lhs->kind == Parsing::Expr::Kind::Var) {
        const auto lhs = static_cast<Parsing::VarExpr*>(assignmentExpr.lhs.get());
        const Parsing::BinaryExpr::Operator op = Parsing::Operators::getBinaryOperator(assignmentExpr.op);
        auto leftCopy = std::make_unique<Parsing::VarExpr>(lhs->name);
        if (lhs->type != nullptr)
            leftCopy->type = Parsing::deepCopy(*assignmentExpr.lhs->type);
        assignmentExpr.op = Parsing::AssignmentExpr::Operator::Assign;
        assignmentExpr.rhs = std::make_unique<Parsing::BinaryExpr>(
            op, std::move(leftCopy), std::move(assignmentExpr.rhs));
    }
    else if (assignmentExpr.op != Parsing::AssignmentExpr::Operator::Assign &&
        assignmentExpr.lhs->kind == Parsing::Expr::Kind::Dereference) {
        auto leftCopy = deepCopyDeref(*assignmentExpr.lhs);
        const Parsing::BinaryExpr::Operator op = Parsing::Operators::getBinaryOperator(assignmentExpr.op);
        assignmentExpr.op = Parsing::AssignmentExpr::Operator::Assign;
        assignmentExpr.rhs = std::make_unique<Parsing::BinaryExpr>(
            op, std::move(leftCopy), std::move(assignmentExpr.rhs));
    }
    ASTTraverser::visit(assignmentExpr);
}

std::unique_ptr<Parsing::Expr> deepCopyDeref(const Parsing::Expr& expr)
{
    using Kind = Parsing::Expr::Kind;
    if (expr.kind == Kind::Var) {
        const auto varExpr = static_cast<const Parsing::VarExpr*>(&expr);
        auto copy = std::make_unique<Parsing::VarExpr>(varExpr->name);
        if (expr.type != nullptr)
            copy->type = Parsing::deepCopy(*expr.type);
        return copy;
    }
    if (expr.kind == Kind::Dereference) {
        const auto deref = static_cast<const Parsing::DereferenceExpr*>(&expr);
        std::unique_ptr<Parsing::Expr> inner = deepCopyDeref(*deref->reference);
        if (expr.type != nullptr)
            inner->type = Parsing::deepCopy(*expr.type);
        return std::make_unique<Parsing::DereferenceExpr>(std::move(inner));
    }
    return nullptr;
}

} // Semantics