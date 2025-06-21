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
    using Operator = Parsing::UnaryExpr::Operator;
    if (unaryExpr.op != Operator::PostFixDecrement && unaryExpr.op != Operator::PostFixIncrement &&
        unaryExpr.op != Operator::PrefixDecrement && unaryExpr.op != Operator::PrefixIncrement)
        return;
    if (isNotAnLvalue(unaryExpr.operand->kind)) {
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

void LvalueVerification::visit(const Parsing::AddrOffExpr& addrOffExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;
    if (isNotAnLvalue(addrOffExpr.reference->kind)) {
        m_valid = false;
        return;
    }
    if (addrOffExpr.reference->kind == Parsing::Expr::Kind::Unary) {
        const auto unaryExpr = static_cast<Parsing::UnaryExpr*>(addrOffExpr.reference.get());
        if (unaryExpr->op == Operator::PostFixDecrement || unaryExpr->op == Operator::PrefixDecrement
            || unaryExpr->op == Operator::PrefixIncrement || unaryExpr->op == Operator::PostFixDecrement) {
            m_valid = false;
            return;
        }
    }
    ConstASTTraverser::visit(addrOffExpr);
}
} // Semantics