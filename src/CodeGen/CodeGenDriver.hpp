#pragma once

#include "AsmAST.hpp"
#include "ASTIr.hpp"
#include "ShortTypes.hpp"

namespace CodeGen {

void run(const Ir::Program& irProgram, const std::string& argument, const std::string& inputFile);
[[nodiscard]] i32 replacingPseudoRegisters(const Function& function);
void fixUpInstructions(Function& function, i32 stackAlloc);
void fixAsm(const Program& codegenProgram);
static Program codegen(const Ir::Program& irProgram);
static void assemble(const std::string& asmFile, const std::string& outputFile);
static void linkLib(const std::string& asmFile, const std::string& outputFile, const std::string& argument);
static void makeLib(const std::string& asmFile, const std::string& outputFile);
std::string writeAsmFile(const std::string& inputFile, const std::string& output);

} // CodeGen