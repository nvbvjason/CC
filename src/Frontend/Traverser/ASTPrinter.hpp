#pragma once

#ifndef CC_PARSING_ABSTRACT_TREE_PRINTER_HPP
#define CC_PARSING_ABSTRACT_TREE_PRINTER_HPP

#include "ASTParser.hpp"
#include "ConstASTTraverser.hpp"

#include <sstream>
#include <string>


namespace Parsing {
class ASTPrinter : public ConstASTVisitor {
    std::ostringstream oss;

public:
    std::string getString() const;

    void visit(const Program& program) override;

    void visit(const Function& function) override;

    // BlockItem
    void visit(const StmtBlockItem& stmtBlockItem) override;
    void visit(const DeclBlockItem& declBlockItem) override;

    void visit(const Declaration& declaration) override;

    // Statement
    void visit(const IfStmt& ifStmt) override;
    void visit(const ReturnStmt& returnStmt) override;
    void visit(const ExprStmt& exprStmt) override;
    void visit(const NullStmt& nullStmt) override;

    // Expression
    void visit(const UnaryExpr& unaryExpr) override;
    void visit(const BinaryExpr& binaryExpr) override;
    void visit(const AssignmentExpr& assignmentExpr) override;
    void visit(const ConstExpr& constExpr) override;
    void visit(const VarExpr& varExpr) override;
    void visit(const ConditionalExpr& conditionalExpr) override;
};
} // namespace Parsing

#endif // CC_PARSING_ABSTRACT_TREE_PRINTER_HPP