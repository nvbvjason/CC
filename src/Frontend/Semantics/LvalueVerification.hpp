#pragma once

#ifndef CC_SEMANTICS_LVALUE_VERIFICATION_HPP
#define CC_SEMANTICS_LVALUE_VERIFICATION_HPP

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
};

} // Semantics

#endif // CC_SEMANTICS_LVALUE_VERIFICATION_HPP
