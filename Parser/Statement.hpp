#pragma once

#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include "Expression.hpp"

namespace Parsing {

struct Statement {
    Expression expression;
};

}

#endif //STATEMENT_HPP