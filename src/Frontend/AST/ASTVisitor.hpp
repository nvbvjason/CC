#pragma once

#ifndef CC_PARSER_AST_VISITOR_HPP
#define CC_PARSER_AST_VISITOR_HPP

#include "ASTFwd.hpp"

namespace Parsing {

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(Program&) = 0;
    virtual void visit(Function&) = 0;

    virtual void visit(StmtBlockItem&) = 0;
    virtual void visit(DeclBlockItem&) = 0;

    virtual void visit(Declaration&) = 0;

    // Statements
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(ExprStmt&) = 0;
    virtual void visit(NullStmt&) = 0;

    // Expressions
    virtual void visit(ConstExpr&) = 0;
    virtual void visit(VarExpr&) = 0;
    virtual void visit(UnaryExpr&) = 0;
    virtual void visit(BinaryExpr&) = 0;
    virtual void visit(AssignmentExpr&) = 0;
};

class ConstASTVisitor {
public:
    virtual ~ConstASTVisitor() = default;

    virtual void visit(const Program&) = 0;
    virtual void visit(const Function&) = 0;

    virtual void visit(const StmtBlockItem&) = 0;
    virtual void visit(const DeclBlockItem&) = 0;

    virtual void visit(const Declaration&) = 0;

    // Statements
    virtual void visit(const ReturnStmt&) = 0;
    virtual void visit(const ExprStmt&) = 0;
    virtual void visit(const NullStmt&) = 0;

    // Expressions
    virtual void visit(const ConstExpr&) = 0;
    virtual void visit(const VarExpr&) = 0;
    virtual void visit(const UnaryExpr&) = 0;
    virtual void visit(const BinaryExpr&) = 0;
    virtual void visit(const AssignmentExpr&) = 0;
};

} // Parsing

#endif // CC_PARSER_AST_VISITOR_HPP
