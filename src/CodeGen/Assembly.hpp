#pragma once

#include "AsmAST.hpp"
#include "ASTIr.hpp"

namespace CodeGen {

std::string asmProgram(const Program& program);
void asmFunction(std::string& result, const Function& functionNode);
void asmStaticVariable(std::string& result, const StaticVariable& variable);
void asmStaticVariableByte(std::string& result, const StaticVariable& variable);
void asmStaticVariableLong(std::string& result, const StaticVariable& variable);
void asmStaticVariableQuad(std::string& result, const StaticVariable& variable);
void asmStaticVariableDouble(std::string& result, const StaticVariable& variable);
void asmStaticConstant(std::string& result, const ConstVariable& variable);
void asmStaticArray(std::string& result, const CompoundVariable& array);
void asmStaticString(std::string& result, const StringVariable& variable);
void asmInstruction(std::string& result, const std::unique_ptr<Inst>& instruction);

std::string asmStaticOperand(const Operand* operand);

std::string asmOperand(const Operand* operand);
std::string asmDataOperand(const DataOperand& dataOperand);
std::string asmMemoryOperand(const MemoryOperand& memoryOperand);
std::string asmImmOperand(const ImmOperand& immOperand);
std::string asmRegisterOperand(const RegisterOperand& operand);
std::string asmRegister(const AsmType& type, Operand::RegKind reg);
std::string asmIndexedOperand(const IndexedOperand& indexedOperand);

std::string asmUnaryOperator(UnaryInst::Operator oper, AsmType type);
std::string asmBinaryOperator(BinaryInst::Operator oper, AsmType type);
std::string asmFormatLabel(const std::string& name);
std::string asmFormatInstruction(const std::string& mnemonic,
                                 const std::string& operands = "",
                                 const std::string& comment = "");
std::string createLabel(const std::string& name);
std::string addType(const std::string& instruction, AsmType type);
std::string condCode(BinaryInst::CondCode condCode);
std::string getTypeName(AsmType type);
std::string genAsmCompatibleString(const StringVariable& variable);

} // CodeGen