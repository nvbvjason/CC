#pragma once

#include "ASTParser.hpp"

#include <vector>

namespace Semantics {

void initArrays(std::vector<Parsing::VarDecl*>& arrays);
static void initArray(Parsing::VarDecl& array);
static std::vector<i64> getDimensions(const Parsing::VarDecl& array);
static std::vector<i64> getScales(const std::vector<i64>& dimensions, i64 size);
} // Semantics