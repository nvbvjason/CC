#pragma once

#ifndef CC_CODEGEN_ASM_PRINTER_HPP
#define CC_CODEGEN_ASM_PRINTER_HPP

#include "AsmAST.hpp"

#include <sstream>
#include <string>

namespace CodeGen {

class AsmPrinter {
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
    std::string printProgram(const Program& program);
    void add(const TopLevel& topLevel);
    void add(const Function& function);
    void add(const StaticVariable& staticVariable);
    void add(const Inst& inst);
    void add(const MoveInst& moveInst);
    void add(const UnaryInst& unaryInst);
    void add(const BinaryInst& binaryInst);
    void add(const CmpInst& cmpInst);
    void add(const IdivInst& idivInst);
    void add(const CdqInst& cpqInst);
    void add(const JmpInst& jmpInst);
    void add(const JmpCCInst& jmpCCInst);
    void add(const SetCCInst& setCCInst);
    void add(const LabelInst& labelInst);
    void add(const PushInst& pushInst);
    void add(const CallInst& callInst);
    void add(const ReturnInst& returnInst);
private:
    void addLine(const std::string& name, const std::string& operands = "");
    std::string getIndent() const;
};
std::string to_string(const Identifier& identifier);
std::string to_string(const Operand& operand);
std::string to_string(const ImmOperand& immOperand);
std::string to_string(const RegisterOperand& registerOperand);
std::string to_string(const PseudoOperand& pseudoOperand);
std::string to_string(const StackOperand& stackOperand);

std::string to_string(const UnaryInst::Kind& kind);
std::string to_string(const UnaryInst::Operator& oper);
std::string to_string(const RegisterOperand::Kind& oper);
std::string to_string(const BinaryInst::Operator& oper);
std::string to_string(const Inst::CondCode& condCode);
std::string to_string(AssemblyType type);
} // CodeGen

#endif // CC_CODEGEN_ASM_PRINTER_HPP
