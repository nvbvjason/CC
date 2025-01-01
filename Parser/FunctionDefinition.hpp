#pragma once

#ifndef FUNCTION_DEFINITION_HPP
#define FUNCTION_DEFINITION_HPP

#include "Statement.hpp"

#include <string>

namespace Parsing {

struct FunctionDefinition {
    std::string name;
    Statement body;
};

}



#endif // FUNCTION_DEFINITION_HPP
