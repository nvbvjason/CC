#pragma once

#ifndef CC_PARSING_VALIDATE_RETURN_HPP
#define CC_PARSING_VALIDATE_RETURN_HPP

#include "../Parsing/ParserAST.hpp"

namespace Parsing {
class ValidateReturn {
public:
    bool programValidate(Program& program);
    bool functionValidate(Function& function);
    void addReturnZero(Function& function);
};

} // Parsing

#endif // CC_PARSING_VALIDATE_RETURN_HPP
