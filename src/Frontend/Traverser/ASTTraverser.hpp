#pragma once

#include "ASTVisitor.hpp"

namespace Parsing {

class ASTTraverser : public ASTVisitor {
public:
    void visit(Program& program) override;

    // Declaration
    void visit(VarDecl& varDecl) override;
    void visit(FuncDecl& funDecl) override;

    void visit(Block& block) override;

    void visit(StmtBlockItem& stmtBlockItem) override;
    void visit(DeclBlockItem& declBlockItem) override;
    void visit(StructDecl& structDecl) override;
    void visit(UnionDecl& unionDecl) override;
    void visit(MemberDecl& memberDecl) override {}

    // Type
    void visit(VarType& varType) override {}
    void visit(FuncType& functionType) override;
    void visit(PointerType& pointerType) override;
    void visit(ArrayType& arrayType) override;
    void visit(StructType& structType) override {}
    void visit(UnionType& unionType) override {}

    // Initializers
    void visit(SingleInitializer& singleInitializer) override;
    void visit(CompoundInitializer& compoundInitializer) override;
    void visit(ZeroInitializer& zeroInitializer) override {}
    void visit(StringInitializer& stringInitializer) override {}

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
    void visit(StringExpr& stringExpr) override {}
    void visit(VarExpr& varExpr) override {}
    void visit(CastExpr& castExpr) override;
    void visit(UnaryExpr& unaryExpr) override;
    void visit(BinaryExpr& binaryExpr) override;
    void visit(AssignmentExpr& assignmentExpr) override;
    void visit(TernaryExpr& conditionalExpr) override;
    void visit(FuncCallExpr& functionCallExpr) override;
    void visit(DereferenceExpr& dereferenceExpr) override;
    void visit(AddrOffExpr& addrOffExpr) override;
    void visit(SubscriptExpr& subscriptExpr) override;
    void visit(SizeOfTypeExpr& sizeOfTypeExpr) override;
    void visit(SizeOfExprExpr& sizeOfExprExpr) override;
    void visit(DotExpr& dotExpr) override;
    void visit(ArrowExpr& arrowExpr) override;
};

} // Parsing