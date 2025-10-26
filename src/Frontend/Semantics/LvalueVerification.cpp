#include "LvalueVerification.hpp"
#include "DynCast.hpp"

namespace Semantics {

std::vector<Error> LvalueVerification::resolve(Parsing::Program& program)
{
    ConstASTTraverser::visit(program);
    return std::move(m_errors);
}

void LvalueVerification::visit(const Parsing::UnaryExpr& unaryExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;
    if (unaryExpr.op != Operator::PostFixDecrement && unaryExpr.op != Operator::PostFixIncrement &&
        unaryExpr.op != Operator::PrefixDecrement && unaryExpr.op != Operator::PrefixIncrement)
        return;
    if (isNotAnLvalue(unaryExpr.operand->kind)) {
        m_errors.emplace_back("Unary operation on non lvalue ", unaryExpr.location);
        return;
    }
    if (unaryExpr.operand->kind != Parsing::Expr::Kind::Unary)
        return;
    const auto innerUnaryExpr = dynCast<Parsing::UnaryExpr>(unaryExpr.operand.get());
    if (innerUnaryExpr->op == Operator::PostFixDecrement || innerUnaryExpr->op == Operator::PostFixIncrement)
        m_errors.emplace_back("Postfix as inner unary expression ", unaryExpr.location);
}

void LvalueVerification::visit(const Parsing::AssignmentExpr& assignmentExpr)
{
    if (assignmentExpr.lhs->kind != Parsing::Expr::Kind::Var &&
        assignmentExpr.lhs->kind != Parsing::Expr::Kind::Dereference)
        m_errors.emplace_back("Assignment on non allowed expression type ", assignmentExpr.lhs->location);
    ConstASTTraverser::visit(assignmentExpr);
}

void LvalueVerification::visit(const Parsing::AddrOffExpr& addrOffExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;
    if (isNotAnLvalue(addrOffExpr.reference->kind)) {
        m_errors.emplace_back("Address of operation on non lvalue ", addrOffExpr.reference->location);
        return;
    }
    if (addrOffExpr.reference->kind == Parsing::Expr::Kind::Unary) {
        const auto unaryExpr = dynCast<Parsing::UnaryExpr>(addrOffExpr.reference.get());
        if (unaryExpr->op == Operator::PrefixDecrement || unaryExpr->op == Operator::PrefixIncrement) {
            m_errors.emplace_back("Address of operation on prefix ", addrOffExpr.reference->location);
            return;
        }
        if (unaryExpr->op == Operator::PostFixDecrement || unaryExpr->op == Operator::PostFixDecrement) {
            m_errors.emplace_back("Address of operation on postfix ", addrOffExpr.reference->location);
            return;
        }
    }
    ConstASTTraverser::visit(addrOffExpr);
}
} // Semantics