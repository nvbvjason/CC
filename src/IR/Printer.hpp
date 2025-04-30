#pragma once

#include "AbstractTree.hpp"
#include <ostream>
#include <string>

namespace Ir {

class Printer {
    std::ostream& out;
    size_t indentLevel = 0;

    void printIndent()
    {
        for (size_t i = 0; i < indentLevel; ++i)
            out << "  ";
    }

public:
    explicit Printer(std::ostream& os)
        : out(os) {}

    void print(const Program& program);
    void print(const Function& function);
    void print(const Instruction& instruction);
    void print(const Value& value) const;
    void print(const Identifier& identifier) const;

private:
    void visit(const ReturnInst& inst) const;
    void visit(const UnaryInst& inst) const;
    void visit(const BinaryInst& inst) const;
    void visit(const CopyInst& inst) const;
    void visit(const JumpInst& inst) const;
    void visit(const JumpIfZeroInst& inst) const;
    void visit(const JumpIfNotZeroInst& inst) const;
    void visit(const LabelInst& inst) const;
    void visit(const ValueVar& val) const;
    void visit(const ValueConst& val) const;
};

std::string to_string(UnaryInst::Operation op);
std::string to_string(BinaryInst::Operation op);

} // namespace Ir