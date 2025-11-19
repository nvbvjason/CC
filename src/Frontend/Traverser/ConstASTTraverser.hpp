#pragma once

#include "ASTVisitor.hpp"

namespace Parsing {

class ConstASTTraverser : public ConstASTVisitor {
public:
    void visit(const Program& program) override;
    void visit(const Block& block) override;

    // Declaration
    void visit(const VarDecl& varDecl) override;
    void visit(const FuncDecl& funDecl) override;
    void visit(const StructuredDecl& structuredDecl) override;
    void visit(const MemberDecl& memberDecl) override {}

    // BlockItem
    void visit(const StmtBlockItem& stmtBlockItem) override;
    void visit(const DeclBlockItem& declBlockItem) override;

    // Type
    void visit(const VarType& varType) override {}
    void visit(const FuncType& functionType) override;
    void visit(const PointerType& pointerType) override;
    void visit(const ArrayType& arrayType) override;
    void visit(const StructuredType& structuredType) override {}

    // Initializers
    void visit(const SingleInitializer& singleInitializer) override;
    void visit(const CompoundInitializer& compoundInitializer) override;
    void visit(const ZeroInitializer& zeroInitializer) override {}
    void visit(const StringInitializer& stringInitializer) override {}

    // ForInit
    void visit(const DeclForInit& declForInit) override;
    void visit(const ExprForInit& exprForInit) override;

    // Statements
    void visit(const ReturnStmt& returnStmt) override;
    void visit(const ExprStmt& exprStmt) override;
    void visit(const IfStmt& ifStmt) override;
    void visit(const GotoStmt& gotoStmt) override {}
    void visit(const CompoundStmt& compoundStmt) override;
    void visit(const BreakStmt& breakStmt) override {}
    void visit(const ContinueStmt& continueStmt) override {}
    void visit(const LabelStmt& labelStmt) override;
    void visit(const CaseStmt& caseStmt) override;
    void visit(const DefaultStmt& defaultStmt) override;
    void visit(const WhileStmt& whileStmt) override;
    void visit(const DoWhileStmt& doWhileStmt) override;
    void visit(const ForStmt& forStmt) override;
    void visit(const SwitchStmt& switchStmt) override;
    void visit(const NullStmt& nullStmt) override {}

    // Expression
    void visit(const ConstExpr& constExpr) override {}
    void visit(const StringExpr&) override {}
    void visit(const VarExpr& varExpr) override {}
    void visit(const CastExpr& castExpr) override;
    void visit(const UnaryExpr& unaryExpr) override;
    void visit(const BinaryExpr& binaryExpr) override;
    void visit(const AssignmentExpr& assignmentExpr) override;
    void visit(const TernaryExpr& conditionalExpr) override;
    void visit(const FuncCallExpr& functionCallExpr) override;
    void visit(const DereferenceExpr& dereferenceExpr) override;
    void visit(const AddrOffExpr& addrOffExpr) override;
    void visit(const SubscriptExpr& subscriptExpr) override;
    void visit(const SizeOfTypeExpr& sizeOfTypeExpr) override;
    void visit(const SizeOfExprExpr& sizeOfExprExpr) override;
    void visit(const DotExpr& dotExpr) override;
    void visit(const ArrowExpr& arrowExpr) override;
};

} // Parsing