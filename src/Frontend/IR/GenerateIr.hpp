#pragma once

#ifndef CC_IR_GENERATE_IR_HPP
#define CC_IR_GENERATE_IR_HPP

#include "ASTIr.hpp"
#include "ASTParser.hpp"
#include "SymbolTable.hpp"
#include "ExprResult.hpp"

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

    std::unique_ptr<ExprResult> genInst(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> genInstAndConvert(const Parsing::Expr& parsingExpr);

    std::unique_ptr<ExprResult> genCastInst(const Parsing::CastExpr& castExpr);
    std::unique_ptr<ExprResult> genUnaryInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genUnaryBasicInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genBinaryInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genBinaryAndInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genBinaryOrInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genAssignInst(const Parsing::AssignmentExpr& assignmentExpr);
    std::unique_ptr<ExprResult> genTernaryInst(const Parsing::TernaryExpr& ternaryExpr);
    std::unique_ptr<ExprResult> genFuncCallInst(const Parsing::FunCallExpr& funcCallExpr);
    std::unique_ptr<ExprResult> genAddrOfInst(const Parsing::AddrOffExpr& addrOffExpr);
    std::unique_ptr<ExprResult> genDereferenceInst(const Parsing::DereferenceExpr& dereferenceExpr);
    std::unique_ptr<ExprResult> genConstInst(const Parsing::ConstExpr& constExpr);
    std::unique_ptr<ExprResult> genVarInst(const Parsing::VarExpr& varExpr);
};

} // IR

#endif // CC_IR_GENERATE_IR_HPP