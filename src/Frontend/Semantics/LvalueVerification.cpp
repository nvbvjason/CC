#include "LvalueVerification.hpp"

namespace Semantics {

bool LvalueVerification::resolve(Parsing::Program& program)
{
    m_valid = true;
    ConstASTTraverser::visit(program);
    return m_valid;
}

void LvalueVerification::visit(const Parsing::UnaryExpr& unaryExpr)
{
    using ExprKind = Parsing::Expr::Kind;
    using Operator = Parsing::UnaryExpr::Operator;
    if (unaryExpr.op != Operator::PostFixDecrement && unaryExpr.op != Operator::PostFixIncrement &&
        unaryExpr.op != Operator::PrefixDecrement && unaryExpr.op != Operator::PrefixIncrement)
        return;
    if (unaryExpr.operand->kind == ExprKind::Assignment ||
        unaryExpr.operand->kind == ExprKind::Binary ||
        unaryExpr.operand->kind == ExprKind::FunctionCall ||
        unaryExpr.operand->kind == ExprKind::Constant) {
        m_valid = false;
        return;
    }
    if (unaryExpr.operand->kind != Parsing::Expr::Kind::Unary)
        return;
    const auto innerUnaryExpr = static_cast<Parsing::UnaryExpr*>(unaryExpr.operand.get());
    if (innerUnaryExpr->op == Operator::PostFixDecrement
        || innerUnaryExpr->op == Operator::PostFixIncrement)
        m_valid = false;
}

void LvalueVerification::visit(const Parsing::AssignmentExpr& assignmentExpr)
{
    if (assignmentExpr.lhs->kind != Parsing::Expr::Kind::Var)
        m_valid = false;
    ConstASTTraverser::visit(assignmentExpr);
}
} // Semantics