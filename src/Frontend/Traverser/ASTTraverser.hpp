#pragma once

#ifndef CC_PARSING_AST_TRAVERSER_HPP
#define CC_PARSING_AST_TRAVERSER_HPP

#include "ASTVisitor.hpp"

namespace Parsing {

class ASTTraverser : public ASTVisitor {
public:
    void visit(Program& program) override;

    void visit(Function& function) override;

    void visit(Block& block) override;

    void visit(StmtBlockItem& stmtBlockItem) override;
    void visit(DeclBlockItem& declBlockItem) override;

    void visit(Declaration& declaration) override;

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
    void visit(UnaryExpr& unaryExpr) override;
    void visit(BinaryExpr& binaryExpr) override;
    void visit(AssignmentExpr& assignmentExpr) override;
    void visit(ConditionalExpr& conditionalExpr) override;
};

} // Parsing

#endif // CC_PARSING_AST_TRAVERSER_HPP
