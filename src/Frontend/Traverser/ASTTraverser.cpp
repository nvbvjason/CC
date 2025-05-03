#include "ASTTraverser.hpp"
#include "ASTParser.hpp"

namespace Parsing {

void ASTTraverser::visit(Program& program)
{
    program.function->accept(*this);
}

void ASTTraverser::visit(Function& function)
{
    for (std::unique_ptr<BlockItem>& blockItem : function.body)
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