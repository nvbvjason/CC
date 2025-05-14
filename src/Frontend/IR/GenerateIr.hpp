#pragma once

#ifndef CC_IR_CONCRETE_TREE_HPP
#define CC_IR_CONCRETE_TREE_HPP

#include "ASTIr.hpp"
#include "ASTParser.hpp"

namespace Ir {

void program(const Parsing::Program* parsingProgram, Program& tackyProgram);
std::unique_ptr<TopLevel> topLevelIr(const Parsing::Declaration& decl);
std::unique_ptr<TopLevel> functionIr(const Parsing::FunDecl& parsingFunction);
std::unique_ptr<TopLevel> staticVariableIr(const Parsing::VarDecl& varDecl);
void blockIr(const Parsing::Block& block, std::vector<std::unique_ptr<Instruction>>& instructions);
void blockItemIr(const Parsing::BlockItem& blockItem,
               std::vector<std::unique_ptr<Instruction>>& instructions);
void declarationIr(const Parsing::Declaration& decl,
                 std::vector<std::unique_ptr<Instruction>>& insts);
void forInitIr(const Parsing::ForInit& forInit,
               std::vector<std::unique_ptr<Instruction>>& insts);
void stmtIr(const Parsing::Stmt& stmt,
            std::vector<std::unique_ptr<Instruction>>& insts);
void ifStmtIr(const Parsing::IfStmt& stmt,
              std::vector<std::unique_ptr<Instruction>>& insts);
void ifElseStmtIr(const Parsing::IfStmt& stmt,
                  std::vector<std::unique_ptr<Instruction>>& insts);
void gotoStmtIr(const Parsing::GotoStmt& stmt,
                   std::vector<std::unique_ptr<Instruction>>& insts);
void compoundStmtIr(const Parsing::CompoundStmt& stmt,
                    std::vector<std::unique_ptr<Instruction>>& insts);
void breakStmtIr(const Parsing::BreakStmt& stmt,
                 std::vector<std::unique_ptr<Instruction>>& insts);
void continueStmtIr(const Parsing::ContinueStmt& stmt,
                    std::vector<std::unique_ptr<Instruction>>& insts);
void labelStmtIr(const Parsing::LabelStmt& stmt,
                 std::vector<std::unique_ptr<Instruction>>& insts);
void caseStmtIr(const Parsing::CaseStmt& caseStmt,
                std::vector<std::unique_ptr<Instruction>>& insts);
void defaultStmtIr(const Parsing::DefaultStmt& defaultStmt,
                   std::vector<std::unique_ptr<Instruction>>& insts);
void doWhileStmtIr(const Parsing::DoWhileStmt& stmt,
                   std::vector<std::unique_ptr<Instruction>>& insts);
void whileStmtIr(const Parsing::WhileStmt& stmt,
                 std::vector<std::unique_ptr<Instruction>>& insts);
void forStmtIr(const Parsing::ForStmt& stmt,
               std::vector<std::unique_ptr<Instruction>>& insts);
void switchStmtIr(const Parsing::SwitchStmt& stmt,
                  std::vector<std::unique_ptr<Instruction>>& insts);
std::shared_ptr<Value> instIr(const Parsing::Expr& parsingExpr,
                            std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> unaryInstIr(const Parsing::Expr& parsingExpr,
                                 std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> binaryInstIr(const Parsing::Expr& parsingExpr,
                                  std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> binaryAndInstIr(const Parsing::BinaryExpr& parsingExpr,
                                     std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> binaryOrInstIr(const Parsing::BinaryExpr& binaryExpr,
                                    std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> assignInstIr(const Parsing::Expr& binaryExpr,
                                  std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> simpleAssignInstIr(const Parsing::AssignmentExpr& assignExpr,
                                        std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> compoundAssignInstIr(const Parsing::AssignmentExpr& assignExpr,
                                          std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> conditionalInstIr(const Parsing::Expr& stmt,
                                       std::vector<std::unique_ptr<Instruction>>& insts);
std::shared_ptr<Value> funcCallInstIr(const Parsing::Expr& stmt,
                                      std::vector<std::unique_ptr<Instruction> >& insts);
std::shared_ptr<Value> constInstIr(const Parsing::Expr& parsingExpr);
std::shared_ptr<Value> varInstIr(const Parsing::Expr& parsingExpr);

} // IR

#endif // CC_IR_CONCRETE_TREE_HPP