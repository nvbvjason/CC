#pragma once

#ifndef CC_CODEGEN_AST_TO_ASSEMBLY_HPP
#define CC_CODEGEN_AST_TO_ASSEMBLY_HPP

#include "AbstractTree.hpp"
#include "ConcreteTree.hpp"

namespace CodeGen {

std::string asmProgram(const Program& program);
void asmFunction(std::string& result, const std::shared_ptr<Function>& functionNode);
void asmInstruction(std::string& result, const std::shared_ptr<Inst>& instruction);
std::string asmOperand(const std::shared_ptr<Operand>& operand);
std::string asmRegister(const RegisterOperand* reg);
std::string asmUnaryOperator(UnaryInst::Operator oper);
std::string asmFormatLabel(const std::string& name);
std::string asmFormatInstruction(const std::string& mnemonic,
                                 const std::string& operands = "",
                                 const std::string& comment = "");
} // CodeGen

#endif // CC_CODEGEN_AST_TO_ASSEMBLY_HPP