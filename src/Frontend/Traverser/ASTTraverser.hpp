#pragma once

#ifndef CC_PARSING_AST_TRAVERSER_HPP
#define CC_PARSING_AST_TRAVERSER_HPP

#include "ASTVisitor.hpp"

namespace Parsing {

class ASTTraverser : public ASTVisitor {
public:
    void visit(Program& program) override;

    // Declaration
    void visit(VarDecl& varDecl) override;
    void visit(FunDecl& funDecl) override;

    void visit(Block& block) override;

    void visit(StmtBlockItem& stmtBlockItem) override;
    void visit(DeclBlockItem& declBlockItem) override;

    // Type
    void visit(VarType& varType) override {}
    void visit(FuncType& functionType) override;
    void visit(PointerType& pointerType) override;

    // ForInit
    void visit(DeclForInit& declForInit) override;
    void visit(ExprForInit& exprForInit) override;

    // Statement
    void visit(ReturnStmt& returnStmt) override;
    void visit(ExprStmt& exprStmt) override;
    void visit(IfStmt& ifStmt) override;
    void visit(GotoStmt& gotoStmt) override {}
    void visit(CompoundStmt& compoundStmt) override;
    void visit(BreakStmt& breakStmt) override {}
    void visit(ContinueStmt& continueStmt) override {}
    void visit(LabelStmt& labelStmt) override;
    void visit(CaseStmt& caseStmt) override;
    void visit(DefaultStmt& defaultStmt) override;
    void visit(WhileStmt& whileStmt) override;
    void visit(DoWhileStmt& doWhileStmt) override;
    void visit(ForStmt& forStmt) override;
    void visit(SwitchStmt& switchStmt) override;
    void visit(NullStmt& nullStmt) override {}

    // Expression
    void visit(ConstExpr& constExpr) override {}
    void visit(VarExpr& varExpr) override {}
    void visit(CastExpr& castExpr) override;
    void visit(UnaryExpr& unaryExpr) override;
    void visit(BinaryExpr& binaryExpr) override;
    void visit(AssignmentExpr& assignmentExpr) override;
    void visit(TernaryExpr& conditionalExpr) override;
    void visit(FunCallExpr& functionCallExpr) override;
    void visit(DereferenceExpr& functionCallExpr) override;
    void visit(AddrOffExpr& addrOffExpr) override;
};

} // Parsing

#endif // CC_PARSING_AST_TRAVERSER_HPP