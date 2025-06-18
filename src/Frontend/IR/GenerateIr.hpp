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
    std::vector<std::unique_ptr<Instruction>> m_insts;
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
    void genBlock(const Parsing::Block& block);
    void genBlockItem(const Parsing::BlockItem& blockItem);
    void genDeclaration(const Parsing::Declaration& decl);
    void genStaticLocal(const Parsing::VarDecl& varDecl);
    void genReturnStmt(const Parsing::ReturnStmt& returnStmt);
    void genIfStmt(const Parsing::IfStmt& ifStmt);
    void genForInit(const Parsing::ForInit& forInit);
    void genStmt(const Parsing::Stmt& stmt);
    void genIfBasicStmt(const Parsing::IfStmt& ifStmt);
    void genIfElseStmt(const Parsing::IfStmt& ifStmt);
    void genGotoStmt(const Parsing::GotoStmt& gotoStmt);
    void genCompoundStmt(const Parsing::CompoundStmt& compoundStmt);
    void genBreakStmt(const Parsing::BreakStmt& breakStmt);
    void genContinueStmt(const Parsing::ContinueStmt& continueStmt);
    void genLabelStmt(const Parsing::LabelStmt& labelStmt);
    void genCaseStmt(const Parsing::CaseStmt& caseStmt);
    void genDefaultStmt(const Parsing::DefaultStmt& defaultStmt);
    void genDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt);
    void genWhileStmt(const Parsing::WhileStmt& whileStmt);
    void genForStmt(const Parsing::ForStmt& forStmt);
    void genSwitchStmt(const Parsing::SwitchStmt& stmt);

    std::shared_ptr<Value> genInst(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> genCastInst(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> genUnaryInst(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> genUnaryBasicInst(const Parsing::UnaryExpr& unaryExpr);
    std::shared_ptr<Value> genUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr);
    std::shared_ptr<Value> genUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr);
    std::shared_ptr<Value> genBinaryInst(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> genBinaryAndInst(const Parsing::BinaryExpr& binaryExpr);
    std::shared_ptr<Value> genBinaryOrInst(const Parsing::BinaryExpr& binaryExpr);
    std::shared_ptr<Value> genAssignInst(const Parsing::Expr& binaryExpr);
    std::shared_ptr<Value> genTernaryInst(const Parsing::Expr& ternary);
    std::shared_ptr<Value> genFuncCallInst(const Parsing::Expr& parsingExpr);
    static std::shared_ptr<Value> genConstInst(const Parsing::Expr& parsingExpr);
    static std::shared_ptr<Value> genVarInst(const Parsing::Expr& parsingExpr);
};

} // IR

#endif // CC_IR_CONCRETE_TREE_HPP