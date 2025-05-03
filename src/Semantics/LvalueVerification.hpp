#pragma once

#ifndef CC_SEMANTICS_LVALUE_VERIFICATION_HPP
#define CC_SEMANTICS_LVALUE_VERIFICATION_HPP

namespace Semantics {

class LvalueVerification {
public:
    LvalueVerification() = default;
    bool programValidate();
    bool functionValidate();
    bool blockItemValidate();
    bool declarationValidate();
    bool stmtValidate();
    bool exprValidate();
};

} // Semantics

#endif // CC_SEMANTICS_LVALUE_VERIFICATION_HPP
