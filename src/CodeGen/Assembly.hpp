#pragma once

#ifndef CC_CODEGEN_AST_TO_ASSEMBLY_HPP
#define CC_CODEGEN_AST_TO_ASSEMBLY_HPP

#include "AbstractTree.hpp"
#include "ConcreteTree.hpp"

namespace CodeGen {

std::string assembleProgram(const Program& program);
void assembleFunction(std::string& result, const std::shared_ptr<Function>& functionNode);
void assembleInstruction(std::string& result, const std::shared_ptr<Inst>& instruction);
std::string assembleOperand(const std::shared_ptr<Operand>& operand);
std::string assembleRegister(const RegisterOperand* reg);
std::string assembleUnaryOperator(UnaryInst::Operator oper);

} // CodeGen

#endif // CC_CODEGEN_AST_TO_ASSEMBLY_HPP