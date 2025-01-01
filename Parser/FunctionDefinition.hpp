#pragma once

#ifndef FUNCTION_DEFINITION_HPP
#define FUNCTION_DEFINITION_HPP

#include "Statement.hpp"

#include <string_view>

namespace Parsing {

struct FunctionDefinition {
    std::string_view name;
    Statement body;
};

}



#endif // FUNCTION_DEFINITION_HPP
