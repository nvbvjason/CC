#pragma once

#include "ASTFwd.hpp"

namespace Parsing {

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(Program&) = 0;

    // Declaration
    virtual void visit(VarDecl&) = 0;
    virtual void visit(FuncDeclaration&) = 0;

    virtual void visit(Block&) = 0;

    // Type
    virtual void visit(VarType&) = 0;
    virtual void visit(FuncType&) = 0;
    virtual void visit(PointerType&) = 0;
    virtual void visit(ArrayType&) = 0;

    // Initializer
    virtual void visit(SingleInitializer&) = 0;
    virtual void visit(CompoundInitializer&) = 0;
    virtual void visit(ZeroInitializer&) = 0;

    virtual void visit(StmtBlockItem&) = 0;
    virtual void visit(DeclBlockItem&) = 0;

    // ForInit
    virtual void visit(DeclForInit&) = 0;
    virtual void visit(ExprForInit&) = 0;

    // Statements
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(GotoStmt&) = 0;
    virtual void visit(ExprStmt&) = 0;
    virtual void visit(CompoundStmt&) = 0;
    virtual void visit(BreakStmt&) = 0;
    virtual void visit(ContinueStmt&) = 0;
    virtual void visit(LabelStmt&) = 0;
    virtual void visit(CaseStmt&) = 0;
    virtual void visit(DefaultStmt&) = 0;
    virtual void visit(WhileStmt&) = 0;
    virtual void visit(DoWhileStmt&) = 0;
    virtual void visit(ForStmt&) = 0;
    virtual void visit(SwitchStmt&) = 0;
    virtual void visit(NullStmt&) = 0;

    // Expressions
    virtual void visit(ConstExpr&) = 0;
    virtual void visit(StringExpr&) = 0;
    virtual void visit(VarExpr&) = 0;
    virtual void visit(CastExpr&) = 0;
    virtual void visit(UnaryExpr&) = 0;
    virtual void visit(BinaryExpr&) = 0;
    virtual void visit(AssignmentExpr&) = 0;
    virtual void visit(TernaryExpr&) = 0;
    virtual void visit(FuncCallExpr&) = 0;
    virtual void visit(DereferenceExpr&) = 0;
    virtual void visit(AddrOffExpr&) = 0;
    virtual void visit(SubscriptExpr&) = 0;
};

class ConstASTVisitor {
public:
    virtual ~ConstASTVisitor() = default;

    virtual void visit(const Program&) = 0;

    // Declaration
    virtual void visit(const VarDecl&) = 0;
    virtual void visit(const FuncDeclaration&) = 0;

    virtual void visit(const Block&) = 0;

    // Type
    virtual void visit(const VarType&) = 0;
    virtual void visit(const FuncType&) = 0;
    virtual void visit(const PointerType&) = 0;
    virtual void visit(const ArrayType&) = 0;

    // Initializer
    virtual void visit(const SingleInitializer&) = 0;
    virtual void visit(const CompoundInitializer&) = 0;
    virtual void visit(const ZeroInitializer&) = 0;

    virtual void visit(const StmtBlockItem&) = 0;
    virtual void visit(const DeclBlockItem&) = 0;

    // ForInit
    virtual void visit(const DeclForInit&) = 0;
    virtual void visit(const ExprForInit&) = 0;

    // Statements
    virtual void visit(const ReturnStmt&) = 0;
    virtual void visit(const ExprStmt&) = 0;
    virtual void visit(const IfStmt&) = 0;
    virtual void visit(const GotoStmt&) = 0;
    virtual void visit(const CompoundStmt&) = 0;
    virtual void visit(const BreakStmt&) = 0;
    virtual void visit(const ContinueStmt&) = 0;
    virtual void visit(const LabelStmt&) = 0;
    virtual void visit(const CaseStmt&) = 0;
    virtual void visit(const DefaultStmt&) = 0;
    virtual void visit(const WhileStmt&) = 0;
    virtual void visit(const DoWhileStmt&) = 0;
    virtual void visit(const ForStmt&) = 0;
    virtual void visit(const SwitchStmt&) = 0;
    virtual void visit(const NullStmt&) = 0;

    // Expressions
    virtual void visit(const ConstExpr&) = 0;
    virtual void visit(const StringExpr&) = 0;
    virtual void visit(const VarExpr&) = 0;
    virtual void visit(const CastExpr&) = 0;
    virtual void visit(const UnaryExpr&) = 0;
    virtual void visit(const BinaryExpr&) = 0;
    virtual void visit(const AssignmentExpr&) = 0;
    virtual void visit(const TernaryExpr&) = 0;
    virtual void visit(const FuncCallExpr&) = 0;
    virtual void visit(const DereferenceExpr&) = 0;
    virtual void visit(const AddrOffExpr&) = 0;
    virtual void visit(const SubscriptExpr&) = 0;
};

} // Parsing