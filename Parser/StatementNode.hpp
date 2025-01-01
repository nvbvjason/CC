#pragma once

#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include "ExpressionNode.hpp"

namespace Parsing {

struct StatementNode {
    ExpressionNode expression;
};

}

#endif //STATEMENT_HPP