#pragma once

#ifndef CC_CODEGEN_AST_TO_ASSEMBLY_HPP
#define CC_CODEGEN_AST_TO_ASSEMBLY_HPP

#include "AsmAST.hpp"

namespace CodeGen {

std::string asmProgram(const Program& program);
void asmFunction(std::string& result, const Function& functionNode);
void asmGlobalVar(std::string& result, const StaticVariable& variable);
void asmStaticVariable(std::string& result, const StaticVariable& staticVariableNode);
void asmInstruction(std::string& result, const std::unique_ptr<Inst>& instruction);
std::string asmOperand(const std::shared_ptr<Operand>& operand);
std::string asmRegister(const RegisterOperand* reg);
std::string asmUnaryOperator(UnaryInst::Operator oper);
std::string asmBinaryOperator(BinaryInst::Operator oper);
std::string asmFormatLabel(const std::string& name);
std::string asmFormatInstruction(const std::string& mnemonic,
                                 const std::string& operands = "",
                                 const std::string& comment = "");
std::string createLabel(const std::string& name);
std::string condCode(BinaryInst::CondCode condCode);
std::string alignDirective();

} // CodeGen

#endif // CC_CODEGEN_AST_TO_ASSEMBLY_HPP