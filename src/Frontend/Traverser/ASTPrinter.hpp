#pragma once

#include "ASTParser.hpp"
#include "ConstASTTraverser.hpp"

#include <sstream>
#include <string>

namespace Parsing {
class ASTPrinter final : public ConstASTTraverser {
    class IndentGuard {
    public:
        explicit IndentGuard(int& level) : m_level(level) { ++m_level; }
        ~IndentGuard() { --m_level; }
    private:
        int& m_level;
    };
    std::ostringstream oss;
    i32 m_indentLevel = 0;
    i32 m_indentMultiplier = 3;
public:
    std::string getString() const;

    void visit(const Program& program) override;

    // Declartion
    void visit(const VarDecl& varDecl) override;
    void visit(const FuncDeclaration& funDecl) override;

    void visit(const Block& block) override;

    void visit(const ZeroInitializer& zeroInitializer) override;
    void visit(const StringInitializer&) override;

    // Type
    void visit(const VarType& varType) override;
    void visit(const FuncType& functionType) override;
    void visit(const PointerType& pointerType) override;
    void visit(const ArrayType& arrayType) override;

    // BlockItem
    void visit(const StmtBlockItem& stmtBlockItem) override;
    void visit(const DeclBlockItem& declBlockItem) override;

    // ForInit
    void visit(const DeclForInit& declForInit) override;
    void visit(const ExprForInit& exprForInit) override;

    // Statement
    void visit(const ReturnStmt& returnStmt) override;
    void visit(const ExprStmt& exprStmt) override;
    void visit(const IfStmt& ifStmt) override;
    void visit(const GotoStmt& gotoStmt) override;
    void visit(const CompoundStmt& function) override;
    void visit(const BreakStmt& breakStmt) override;
    void visit(const ContinueStmt& continueStmt) override;
    void visit(const LabelStmt& labelStmt) override;
    void visit(const CaseStmt& caseStmt) override;
    void visit(const DefaultStmt& defaultStmt) override;
    void visit(const WhileStmt& whileStmt) override;
    void visit(const DoWhileStmt& doWhileStmt) override;
    void visit(const ForStmt& forStmt) override;
    void visit(const SwitchStmt& switchStmt) override;
    void visit(const NullStmt& nullStmt) override;

    // Expression
    void visit(const UnaryExpr& unaryExpr) override;
    void visit(const StringExpr& stringExpr) override;
    void visit(const CastExpr& castExpr) override;
    void visit(const BinaryExpr& binaryExpr) override;
    void visit(const AssignmentExpr& assignmentExpr) override;
    void visit(const ConstExpr& constExpr) override;
    void visit(const VarExpr& varExpr) override;
    void visit(const TernaryExpr& conditionalExpr) override;
    void visit(const AddrOffExpr& addrOffExpr) override;
    void visit(const DereferenceExpr& dereferenceExpr) override;
    void visit(const FuncCallExpr& functionCallExpr) override;
    void visit(const SubscriptExpr& subscriptExpr) override;

private:
    void addLine(const std::string &line);
    std::string getIndent() const;
};
} // namespace Parsing