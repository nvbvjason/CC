#pragma once

#ifndef FUNCTION_DEFINITION_HPP
#define FUNCTION_DEFINITION_HPP

#include "StatementNode.hpp"

#include <string>

namespace Parsing {

struct FunctionNode {
    std::string name;
    StatementNode body;
};

}



#endif // FUNCTION_DEFINITION_HPP
