#pragma once

#ifndef CC_CODEGEN_AST_TO_ASSEMBLY_HPP
#define CC_CODEGEN_AST_TO_ASSEMBLY_HPP

#include "AsmAST.hpp"

namespace CodeGen {

std::string asmProgram(const Program& program);
void asmFunction(std::string& result, const Function& functionNode);
void asmStaticVariable(std::string& result, const StaticVariable& variable);
void asmStaticVariableLong(std::string& result, const StaticVariable& variable);
void asmStaticVariableQuad(std::string& result, const StaticVariable& variable);
void asmStaticVariableDouble(std::string& result, const StaticVariable& variable);
void asmStaticConstant(std::string& result, const ConstVariable& variable);
void asmInstruction(std::string& result, const std::unique_ptr<Inst>& instruction);
std::string asmOperand(const std::shared_ptr<Operand>& operand);
std::string asmRegister(const RegisterOperand* reg);
std::string asmUnaryOperator(UnaryInst::Operator oper, AsmType type);
std::string asmBinaryOperator(BinaryInst::Operator oper, AsmType type);
std::string asmFormatLabel(const std::string& name);
std::string asmFormatInstruction(const std::string& mnemonic,
                                 const std::string& operands = "",
                                 const std::string& comment = "");
std::string createLabel(const std::string& name);
std::string addType(const std::string& instruction, AsmType type);
std::string condCode(BinaryInst::CondCode condCode);

} // CodeGen
#endif // CC_CODEGEN_AST_TO_ASSEMBLY_HPP