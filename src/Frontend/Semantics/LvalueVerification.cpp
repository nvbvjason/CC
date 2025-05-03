#include "LvalueVerification.hpp"

namespace Semantics {

bool LvalueVerification::resolve()
{
    m_valid = true;
    m_program.accept(*this);
    return m_valid;
}

void LvalueVerification::visit(const Parsing::UnaryExpr& unaryExpr)
{
    using Operator = Parsing::UnaryExpr::Operator;
    if (unaryExpr.op != Operator::PostFixDecrement && unaryExpr.op != Operator::PostFixIncrement &&
        unaryExpr.op != Operator::PrefixDecrement && unaryExpr.op != Operator::PrefixIncrement)
        return;
    if (unaryExpr.operand->kind == Parsing::Expr::Kind::Assignment ||
        unaryExpr.operand->kind == Parsing::Expr::Kind::Binary ||
        unaryExpr.operand->kind == Parsing::Expr::Kind::Constant) {
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

} // Semantics