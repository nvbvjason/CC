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

    // Statement
    void visit(IfStmt& ifStmt) override;
    void visit(ReturnStmt& returnStmt) override;
    void visit(ExprStmt& exprStmt) override;
    void visit(CompoundStmt& compoundStmt) override;
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
