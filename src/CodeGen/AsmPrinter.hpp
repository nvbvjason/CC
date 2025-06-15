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
    void add(const ConstVariable& constVariable);
    void add(const Inst& inst);
    void add(const MoveInst& move);
    void add(const MoveSXInst& moveSX);
    void add(const UnaryInst& unary);
    void add(const BinaryInst& binary);
    void add(const CmpInst& cmp);
    void add(const IdivInst& idiv);
    void add(const CdqInst& cpq);
    void add(const JmpInst& jmp);
    void add(const JmpCCInst& jmpCC);
    void add(const SetCCInst& setCC);
    void add(const LabelInst& label);
    void add(const PushInst& push);
    void add(const CallInst& call);
    void add(const ReturnInst& returnInst);
    void add(const Cvtsi2sdInst& cvtsi2sd);
    void add(const Cvttsd2siInst& cvttsd2si);
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
std::string to_string(const DataOperand& stackOperand);

std::string to_string(const UnaryInst::Kind& kind);
std::string to_string(const UnaryInst::Operator& oper);
std::string to_string(const RegisterOperand::Kind& oper);
std::string to_string(const BinaryInst::Operator& oper);
std::string to_string(const Inst::CondCode& condCode);
std::string to_string(AsmType type);
} // CodeGen

#endif // CC_CODEGEN_ASM_PRINTER_HPP