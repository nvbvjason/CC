#pragma once

#ifndef CC_IR_CONCRETE_TREE_HPP
#define CC_IR_CONCRETE_TREE_HPP

#include "ASTIr.hpp"
#include "ASTParser.hpp"
#include "SymbolTable.hpp"

#include <unordered_set>

namespace Ir {
class GenerateIr {
    using Storage = Parsing::Declaration::StorageClass;

    bool m_global = true;
    std::vector<std::unique_ptr<Instruction>> m_instructions;
    SymbolTable& m_symbolTable;
    std::unordered_set<std::string> m_writtenGlobals;
    std::vector<std::unique_ptr<TopLevel>> m_topLevels;
public:
    explicit GenerateIr(SymbolTable& symbolTable)
        : m_symbolTable(symbolTable) {}
    void program(const Parsing::Program& parsingProgram, Program& tackyProgram);
    std::unique_ptr<TopLevel> topLevelIr(const Parsing::Declaration& decl);
    std::unique_ptr<TopLevel> functionIr(const Parsing::FunDecl& parsingFunction);
    std::unique_ptr<TopLevel> staticVariableIr(const Parsing::VarDecl& varDecl);
    void generateBlock(const Parsing::Block& block);
    void generateBlockItem(const Parsing::BlockItem& blockItem);
    void generateDeclaration(const Parsing::Declaration& decl);
    void generateDeclarationStaticLocal(const Parsing::VarDecl& varDecl);
    void generateForInit(const Parsing::ForInit& forInit);
    void generateStmt(const Parsing::Stmt& stmts);
    void generateIfStmt(const Parsing::IfStmt& ifStmt);
    void generateIfElseStmt(const Parsing::IfStmt& ifStmt);
    void generateGotoStmt(const Parsing::GotoStmt& gotoStmt);
    void generateCompoundStmt(const Parsing::CompoundStmt& compoundStmt);
    void generateBreakStmt(const Parsing::BreakStmt& breakStmt);
    void generateContinueStmt(const Parsing::ContinueStmt& continueStmt);
    void generateLabelStmt(const Parsing::LabelStmt& labelStmt);
    void generateCaseStmt(const Parsing::CaseStmt& caseStmt);
    void generateDefaultStmt(const Parsing::DefaultStmt& defaultStmt);
    void generateDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt);
    void generateWhileStmt(const Parsing::WhileStmt& whileStmt);
    void generateForStmt(const Parsing::ForStmt& forStmt);
    void generateSwitchStmt(const Parsing::SwitchStmt& stmt);
    std::shared_ptr<Value> generateInst(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> generateCast(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> generateUnaryInst(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> generateUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr);
    std::shared_ptr<Value> generateUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr);
    std::shared_ptr<Value> generateBinaryInst(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> generateBinaryAndInst(const Parsing::BinaryExpr& binaryExpr);
    std::shared_ptr<Value> generateBinaryOrInst(const Parsing::BinaryExpr& binaryExpr);
    std::shared_ptr<Value> generateAssignInst(const Parsing::Expr& binaryExpr);
    std::shared_ptr<Value> generateTernaryInst(const Parsing::Expr& ternary);
    std::shared_ptr<Value> generateFuncCallInst(const Parsing::Expr& parsingExpr);
    static std::shared_ptr<Value> generateConstInst(const Parsing::Expr& parsingExpr);
    static std::shared_ptr<Value> generateVarInst(const Parsing::Expr& parsingExpr);
};

} // IR

#endif // CC_IR_CONCRETE_TREE_HPP