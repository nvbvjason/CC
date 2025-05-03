#pragma once

#ifndef CC_PARSING_CONST_AST_TRAVERSER_HPP
#define CC_PARSING_CONST_AST_TRAVERSER_HPP

#include "ASTVisitor.hpp"

namespace Parsing {

class ConstASTTraverser : public ConstASTVisitor {
public:
    void visit(const Program& program) override;

    void visit(const Function& function) override;

    void visit(const StmtBlockItem& stmtBlockItem) override;
    void visit(const DeclBlockItem& declBlockItem) override;

    void visit(const Declaration& declaration) override;

    void visit(const ReturnStmt& returnStmt) override;
    void visit(const ExprStmt& exprStmt) override;
    void visit(const NullStmt& nullStmt) override {}

    void visit(const ConstExpr& constExpr) override {}
    void visit(const VarExpr& varExpr) override {}
    void visit(const UnaryExpr& unaryExpr) override;
    void visit(const BinaryExpr& binaryExpr) override;
    void visit(const AssignmentExpr& assignmentExpr) override;
};

} // Parsing

#endif // CC_PARSING_CONST_AST_TRAVERSER_HPP
