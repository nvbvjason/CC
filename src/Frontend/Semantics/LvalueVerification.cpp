#include "LvalueVerification.hpp"

namespace Semantics {

bool LvalueVerification::resolve()
{
    m_program.accept(*this);
    return m_valid;
}

} // Semantics