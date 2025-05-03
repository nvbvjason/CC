#pragma once

#include "ASTIr.hpp"

#include <sstream>
#include <string>

namespace Ir {

class Printer {
    std::ostringstream m_oss;
    size_t indentLevel = 0;
public:
    Printer() = default;

    std::string print(const Program& program);
    void print(const Function& function);
    void print(const Instruction& instruction);
    void print(const Value& value);
    void print(const Identifier& identifier);

private:
    void visit(const ReturnInst& inst);
    void visit(const UnaryInst& inst);
    void visit(const BinaryInst& inst);
    void visit(const CopyInst& inst);
    void visit(const JumpInst& inst);
    void visit(const JumpIfZeroInst& inst);
    void visit(const JumpIfNotZeroInst& inst);
    void visit(const LabelInst& inst);
    void visit(const ValueVar& val);
    void visit(const ValueConst& val);

    void printIndent();
};

std::string to_string(UnaryInst::Operation op);
std::string to_string(BinaryInst::Operation op);

} // namespace Ir