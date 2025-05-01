#pragma once

#ifndef CC_PARSING_ABSTRACT_TREE_PRINTER_HPP
#define CC_PARSING_ABSTRACT_TREE_PRINTER_HPP

#include "AbstractTree.hpp"

#include <sstream>
#include <string>

namespace Parsing {
class ASTPrinter {
    std::ostringstream oss;
public:
    std::string print(const Program* program);
private:
    void print(const Function* function, int indent);
    void print(const BlockItem* blockItem, int indent);
    void print(const Declaration* declaration, int indent);
    void print(const Statement* statement, int indent);
    void print(const Expr* expr, int indent);
};
} // namespace Parsing

#endif // CC_PARSING_ABSTRACT_TREE_PRINTER_HPP