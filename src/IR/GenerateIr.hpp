#pragma once

#ifndef CC_IR_CONCRETE_TREE_HPP
#define CC_IR_CONCRETE_TREE_HPP

#include "IrAST.hpp"
#include "Parsing/ParserAST.hpp"

namespace Ir {

void program(const Parsing::Program* parsingProgram, Program& tackyProgram);
std::unique_ptr<Function> function(const Parsing::Function& parsingFunction);
void blockItem(const Parsing::BlockItem& blockItem,
               std::vector<std::unique_ptr<Instruction>>& instructions);
void declaration(const Parsing::Declaration& decl,
                 std::vector<std::unique_ptr<Instruction>>& insts);
void statement(const Parsing::Stmt& stmt,
               std::vector<std::unique_ptr<Instruction>>& insts);
std::shared_ptr<Value> inst(const Parsing::Expr& parsingExpr,
                            std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> unaryInst(const Parsing::Expr& parsingExpr,
                                 std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> binaryInst(const Parsing::Expr& parsingExpr,
                                  std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> binaryAndInst(const Parsing::BinaryExpr& parsingExpr,
                                     std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> binaryOrInst(const Parsing::BinaryExpr& binaryExpr,
                                    std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> assignInst(const Parsing::Expr& binaryExpr,
                                  std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> constInst(const Parsing::Expr& parsingExpr);
std::shared_ptr<Value> varInst(const Parsing::Expr& parsingExpr);

} // IR

#endif // CC_IR_CONCRETE_TREE_HPP