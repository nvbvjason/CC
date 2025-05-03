#pragma once

#ifndef CC_PARSING_VALIDATE_RETURN_HPP
#define CC_PARSING_VALIDATE_RETURN_HPP

#include "../Parsing/ASTParser.hpp"

namespace Semantics {
class ValidateReturn {
public:
    bool programValidate(Parsing::Program& program);
    bool functionValidate(Parsing::Function& function);
    void addReturnZero(Parsing::Function& function);
};

} // Semantics

#endif // CC_PARSING_VALIDATE_RETURN_HPP
