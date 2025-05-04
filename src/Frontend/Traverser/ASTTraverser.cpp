#include "ASTTraverser.hpp"
#include "ASTParser.hpp"

namespace Parsing {

void ASTTraverser::visit(Program& program)
{
    program.function->accept(*this);
}

void ASTTraverser::visit(Function& function)
{
    function.body->accept(*this);
}

void ASTTraverser::visit(Block& block)
{
    for (auto& blockItem : block.body)
        blockItem->accept(*this);
}

void ASTTraverser::visit(StmtBlockItem& stmtBlockItem)
{
    stmtBlockItem.stmt->accept(*this);
}

void ASTTraverser::visit(DeclBlockItem& declBlockItem)
{
    declBlockItem.decl->accept(*this);
}

void ASTTraverser::visit(Declaration& declaration)
{
    if (declaration.init)
        declaration.init->accept(*this);
}

void ASTTraverser::visit(DeclForInit& declForInit)
{
    declForInit.decl->accept(*this);
}

void ASTTraverser::visit(ExprForInit& exprForInit)
{
    if (exprForInit.expression)
        exprForInit.expression->accept(*this);
}

void ASTTraverser::visit(IfStmt& ifStmt)
{
    ifStmt.condition->accept(*this);
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt)
        ifStmt.elseStmt->accept(*this);
}

void ASTTraverser::visit(ReturnStmt& returnStmt)
{
    returnStmt.expr->accept(*this);
}

void ASTTraverser::visit(ExprStmt& exprStmt)
{
    exprStmt.expr->accept(*this);
}

void ASTTraverser::visit(CompoundStmt& compoundStmt)
{
    compoundStmt.block->accept(*this);
}

void ASTTraverser::visit(WhileStmt& whileStmt)
{
    whileStmt.condition->accept(*this);
    whileStmt.body->accept(*this);
}

void ASTTraverser::visit(DoWhileStmt& doWhileStmt)
{
    doWhileStmt.body->accept(*this);
    doWhileStmt.condition->accept(*this);
}

void ASTTraverser::visit(ForStmt& forStmt)
{
    forStmt.init->accept(*this);
    if (forStmt.condition)
        forStmt.condition->accept(*this);
    if (forStmt.post)
        forStmt.post->accept(*this);
    forStmt.body->accept(*this);
}

void ASTTraverser::visit(UnaryExpr& unaryExpr)
{
    unaryExpr.operand->accept(*this);
}

void ASTTraverser::visit(BinaryExpr& binaryExpr)
{
    binaryExpr.lhs->accept(*this);
    binaryExpr.rhs->accept(*this);
}

void ASTTraverser::visit(AssignmentExpr& assignmentExpr)
{
    assignmentExpr.lhs->accept(*this);
    assignmentExpr.rhs->accept(*this);
}

void ASTTraverser::visit(ConditionalExpr& conditionalExpr)
{
    conditionalExpr.condition->accept(*this);
    conditionalExpr.first->accept(*this);
    conditionalExpr.second->accept(*this);
}
} // Parsing