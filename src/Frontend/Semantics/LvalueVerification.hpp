#pragma once

#include "ConstASTTraverser.hpp"
#include "ASTParser.hpp"

namespace Semantics {

class LvalueVerification : public Parsing::ConstASTTraverser {
    bool m_valid = true;
public:
    LvalueVerification() = default;

    bool resolve(Parsing::Program& program);

    void visit(const Parsing::UnaryExpr& unaryExpr) override;
    void visit(const Parsing::AssignmentExpr& assignmentExpr) override;
    void visit(const Parsing::AddrOffExpr& addrOffExpr) override;
};

inline bool isNotAnLvalue(const Parsing::Expr::Kind kind)
{
    using Kind = Parsing::Expr::Kind;
    return kind == Kind::Assignment ||
           kind == Kind::Binary ||
           kind == Kind::FunctionCall ||
           kind == Kind::Constant ||
           kind == Kind::Ternary;
}

} // Semantics