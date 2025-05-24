#pragma once

#ifndef CC_IR_CONCRETE_TREE_HPP
#define CC_IR_CONCRETE_TREE_HPP

#include "ASTIr.hpp"
#include "ASTParser.hpp"

namespace Ir {

void program(const Parsing::Program& parsingProgram, Program& tackyProgram);
std::unique_ptr<TopLevel> topLevelIr(const Parsing::Declaration& decl);
std::unique_ptr<TopLevel> functionIr(const Parsing::FunDecl& parsingFunction);
std::unique_ptr<TopLevel> staticVariableIr(const Parsing::VarDecl& varDecl);
void generateBlock(const Parsing::Block& block, std::vector<std::unique_ptr<Instruction>>& instructions);
void generateBlockItem(const Parsing::BlockItem& blockItem,
                       std::vector<std::unique_ptr<Instruction>>& instructions);
void generateDeclaration(const Parsing::Declaration& decl,
                         std::vector<std::unique_ptr<Instruction>>& insts);
void generateForInit(const Parsing::ForInit& forInit,
                     std::vector<std::unique_ptr<Instruction>>& insts);
void generateStmt(const Parsing::Stmt& stmt,
                  std::vector<std::unique_ptr<Instruction>>& insts);
void generateIfStmt(const Parsing::IfStmt& stmt,
                    std::vector<std::unique_ptr<Instruction>>& insts);
void generateIfElseStmt(const Parsing::IfStmt& stmt,
                        std::vector<std::unique_ptr<Instruction>>& insts);
void generateGotoStmt(const Parsing::GotoStmt& stmt,
                      std::vector<std::unique_ptr<Instruction>>& insts);
void generateCompoundStmt(const Parsing::CompoundStmt& stmt,
                          std::vector<std::unique_ptr<Instruction>>& insts);
void generateBreakStmt(const Parsing::BreakStmt& stmt,
                       std::vector<std::unique_ptr<Instruction>>& insts);
void generateContinueStmt(const Parsing::ContinueStmt& stmt,
                          std::vector<std::unique_ptr<Instruction>>& insts);
void generateLabelStmt(const Parsing::LabelStmt& stmt,
                       std::vector<std::unique_ptr<Instruction>>& insts);
void generateCaseStmt(const Parsing::CaseStmt& caseStmt,
                      std::vector<std::unique_ptr<Instruction>>& insts);
void generateDefaultStmt(const Parsing::DefaultStmt& defaultStmt,
                         std::vector<std::unique_ptr<Instruction>>& insts);
void generateDoWhileStmt(const Parsing::DoWhileStmt& stmt,
                         std::vector<std::unique_ptr<Instruction>>& insts);
void generateWhileStmt(const Parsing::WhileStmt& stmt,
                       std::vector<std::unique_ptr<Instruction>>& insts);
void generateForStmt(const Parsing::ForStmt& stmt,
                     std::vector<std::unique_ptr<Instruction>>& insts);
void generateSwitchStmt(const Parsing::SwitchStmt& stmt,
                        std::vector<std::unique_ptr<Instruction>>& insts);
std::shared_ptr<Value> generateInst(const Parsing::Expr& parsingExpr,
                                    std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> generateUnaryInst(const Parsing::Expr& parsingExpr,
                                         std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> generateBinaryInst(const Parsing::Expr& parsingExpr,
                                          std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> generateBinaryAndInst(const Parsing::BinaryExpr& parsingExpr,
                                             std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> generateBinaryOrInst(const Parsing::BinaryExpr& binaryExpr,
                                            std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> generateAssignInst(const Parsing::Expr& binaryExpr,
                                          std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> generateSimpleAssignInst(const Parsing::AssignmentExpr& assignExpr,
                                                std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> generateCompoundAssignInst(const Parsing::AssignmentExpr& assignExpr,
                                                  std::vector<std::unique_ptr<Instruction>>& instructions);
std::shared_ptr<Value> generateConditionalInst(const Parsing::Expr& stmt,
                                               std::vector<std::unique_ptr<Instruction>>& insts);
std::shared_ptr<Value> generateFuncCallInst(const Parsing::Expr& stmt,
                                            std::vector<std::unique_ptr<Instruction> >& insts);
std::shared_ptr<Value> generateConstInst(const Parsing::Expr& parsingExpr);
std::shared_ptr<Value> generateVarInst(const Parsing::Expr& parsingExpr);

} // IR

#endif // CC_IR_CONCRETE_TREE_HPP