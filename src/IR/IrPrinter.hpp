#pragma once

#include "ASTIr.hpp"

#include <sstream>
#include <string>

namespace Ir {

class IrPrinter {
    class IndentGuard {
    public:
        explicit IndentGuard(size_t& level)
            : m_level(level) { ++m_level; }
        ~IndentGuard() { --m_level; }
    private:
        size_t& m_level;
    };
    std::ostringstream m_oss;
    size_t m_indentLevel = 0;
    static constexpr i32 c_indentMult = 4;
public:
    IrPrinter() = default;

    std::string print(const Program& program);
    void print(const StaticVariable& variable);
    void print(const Function& function);
    void print(const Instruction& instruction);
    static std::string print(const Value& value);
    static std::string print(const Identifier& identifier);
    static std::string print(const ValueVar& val);
    static std::string print(const ValueConst& val);

private:
    void visit(const ReturnInst& inst);
    void visit(const UnaryInst& inst);
    void visit(const BinaryInst& inst);
    void visit(const CopyInst& inst);
    void visit(const JumpInst& inst);
    void visit(const JumpIfZeroInst& inst);
    void visit(const JumpIfNotZeroInst& inst);
    void visit(const LabelInst& inst);
    void visit(const FunCallInst& inst);

    void addLine(const std::string &line);
    std::string getIndent() const;
};

std::string to_string(UnaryInst::Operation op);
std::string to_string(BinaryInst::Operation op);

} // namespace Ir