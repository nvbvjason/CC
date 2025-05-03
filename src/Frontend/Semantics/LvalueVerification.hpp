#pragma once

#ifndef CC_SEMANTICS_LVALUE_VERIFICATION_HPP
#define CC_SEMANTICS_LVALUE_VERIFICATION_HPP

#include "ConstASTTraverser.hpp"
#include "ASTParser.hpp"

namespace Semantics {

class LvalueVerification : public Parsing::ConstASTTraverser {
    bool m_valid = true;
    Parsing::Program& m_program;
public:
    explicit LvalueVerification(Parsing::Program& program)
        : m_program(program) {}

    bool resolve();

    void visit(const Parsing::UnaryExpr& unaryExpr) override;
};

} // Semantics

#endif // CC_SEMANTICS_LVALUE_VERIFICATION_HPP
