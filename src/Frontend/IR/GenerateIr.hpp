#pragma once

#ifndef CC_IR_CONCRETE_TREE_HPP
#define CC_IR_CONCRETE_TREE_HPP

#include "ASTIr.hpp"
#include "ASTParser.hpp"

namespace Ir {

void program(const Parsing::Program* parsingProgram, Program& tackyProgram);
std::unique_ptr<Function> function(const Parsing::Function& parsingFunction);
void blockIr(const Parsing::Block& block, std::vector<std::unique_ptr<Instruction>>& instructions);
void blockItem(const Parsing::BlockItem& blockItem,
               std::vector<std::unique_ptr<Instruction>>& instructions);
void declaration(const Parsing::Declaration& decl,
                 std::vector<std::unique_ptr<Instruction>>& insts);
void forInitialization(const Parsing::ForInit& forInit,
                       std::vector<std::unique_ptr<Instruction>>& insts);
void statement(const Parsing::Stmt& stmt,
               std::vector<std::unique_ptr<Instruction>>& insts);
void ifStatement(const Parsing::IfStmt& stmt,
                 std::vector<std::unique_ptr<Instruction>>& insts);
void ifElseStatement(const Parsing::IfStmt& stmt,
                     std::vector<std::unique_ptr<Instruction>>& insts);
void gotoStatement(const Parsing::GotoStmt& stmt,
                   std::vector<std::unique_ptr<Instruction>>& insts);
void compoundStatement(const Parsing::CompoundStmt& stmt,
                       std::vector<std::unique_ptr<Instruction>>& insts);
void breakStatement(const Parsing::BreakStmt& stmt,
                    std::vector<std::unique_ptr<Instruction>>& insts);
void continueStatement(const Parsing::ContinueStmt& stmt,
                       std::vector<std::unique_ptr<Instruction>>& insts);
void labelStatement(const Parsing::LabelStmt& stmt,
                    std::vector<std::unique_ptr<Instruction>>& insts);
void doWhileStatement(const Parsing::DoWhileStmt& stmt,
                      std::vector<std::unique_ptr<Instruction>>& insts);
void whileStatement(const Parsing::WhileStmt& stmt,
                    std::vector<std::unique_ptr<Instruction>>& insts);
void forStatement(const Parsing::ForStmt& stmt,
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
std::shared_ptr<Value> simpleAssignInst(const Parsing::AssignmentExpr& assignExpr,
                                        std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> compoundAssignInst(const Parsing::AssignmentExpr& assignExpr,
                                          std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> conditionalExpr(const Parsing::Expr& stmt,
                                       std::vector<std::unique_ptr<Instruction>>& insts);
std::shared_ptr<Value> constInst(const Parsing::Expr& parsingExpr);
std::shared_ptr<Value> varInst(const Parsing::Expr& parsingExpr);

} // IR

#endif // CC_IR_CONCRETE_TREE_HPP