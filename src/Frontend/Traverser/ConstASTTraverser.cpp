#include "../Traverser/ConstASTTraverser.hpp"
#include "../AST/ASTParser.hpp"

namespace Parsing {

void ConstASTTraverser::visit(const Program& program)
{
    for (const auto& decl : program.declarations)
        decl->accept(*this);
}

void ConstASTTraverser::visit(const Block& block)
{
    for (const std::unique_ptr<BlockItem>& blockItem : block.body)
        blockItem->accept(*this);
}

// Declaration
void ConstASTTraverser::visit(const VarDecl& varDecl)
{
    if (varDecl.init)
        varDecl.init->accept(*this);
}

void ConstASTTraverser::visit(const FunDecl& funDecl)
{
    if (funDecl.body)
        funDecl.body->accept(*this);
}

// BlockItem
void ConstASTTraverser::visit(const StmtBlockItem& stmtBlockItem)
{
    stmtBlockItem.stmt->accept(*this);
}

void ConstASTTraverser::visit(const DeclBlockItem& declBlockItem)
{
    declBlockItem.decl->accept(*this);
}

// ForInit
void ConstASTTraverser::visit(const DeclForInit& declForInit)
{
    declForInit.decl->accept(*this);
}

void ConstASTTraverser::visit(const ExprForInit& exprForInit)
{
    if (exprForInit.expression)
        exprForInit.expression->accept(*this);
}

// Statements
void ConstASTTraverser::visit(const ReturnStmt& returnStmt)
{
    returnStmt.expr->accept(*this);
}

void ConstASTTraverser::visit(const ExprStmt& exprStmt)
{
    exprStmt.expr->accept(*this);
}

void ConstASTTraverser::visit(const IfStmt& ifStmt)
{
    ifStmt.condition->accept(*this);
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt)
        ifStmt.elseStmt->accept(*this);
}

void ConstASTTraverser::visit(const CompoundStmt& compoundStmt)
{
    compoundStmt.block->accept(*this);
}

void ConstASTTraverser::visit(const LabelStmt& labelStmt)
{
    labelStmt.stmt->accept(*this);
}

void ConstASTTraverser::visit(const CaseStmt& caseStmt)
{
    caseStmt.condition->accept(*this);
    caseStmt.body->accept(*this);
}

void ConstASTTraverser::visit(const DefaultStmt& defaultStmt)
{
    defaultStmt.body->accept(*this);
}

void ConstASTTraverser::visit(const WhileStmt& whileStmt)
{
    whileStmt.condition->accept(*this);
    whileStmt.body->accept(*this);
}

void ConstASTTraverser::visit(const DoWhileStmt& doWhileStmt)
{
    doWhileStmt.body->accept(*this);
    doWhileStmt.condition->accept(*this);
}

void ConstASTTraverser::visit(const ForStmt& forStmt)
{
    if (forStmt.init)
        forStmt.init->accept(*this);
    if (forStmt.condition)
        forStmt.condition->accept(*this);
    if (forStmt.post)
        forStmt.post->accept(*this);
    forStmt.body->accept(*this);
}

void ConstASTTraverser::visit(const SwitchStmt& switchStmt)
{
    switchStmt.condition->accept(*this);
    switchStmt.body->accept(*this);
}

// Expression
void ConstASTTraverser::visit(const UnaryExpr& unaryExpr)
{
    unaryExpr.operand->accept(*this);
}

void ConstASTTraverser::visit(const BinaryExpr& binaryExpr)
{
    binaryExpr.lhs->accept(*this);
    binaryExpr.rhs->accept(*this);
}

void ConstASTTraverser::visit(const AssignmentExpr& assignmentExpr)
{
    assignmentExpr.lhs->accept(*this);
    assignmentExpr.rhs->accept(*this);
}

void ConstASTTraverser::visit(const ConditionalExpr& conditionalExpr)
{
    conditionalExpr.condition->accept(*this);
    conditionalExpr.first->accept(*this);
    conditionalExpr.second->accept(*this);
}

void ConstASTTraverser::visit(const FunCallExpr& functionCallExpr)
{
    for (const std::unique_ptr<Expr>& expr : functionCallExpr.args)
        expr->accept(*this);
}
} // Parsing