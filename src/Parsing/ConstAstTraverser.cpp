#include "ConstAstTraverser.hpp"
#include "ASTParser.hpp"

namespace Parsing {

void ConstASTTraverser::visit(const Program& program)
{
    program.function->accept(*this);
}

void ConstASTTraverser::visit(const Function& function)
{
    for (const std::unique_ptr<BlockItem>& blockItem : function.body)
        blockItem->accept(*this);
}

void ConstASTTraverser::visit(const StmtBlockItem& stmtBlockItem)
{
    stmtBlockItem.stmt->accept(*this);
}

void ConstASTTraverser::visit(const DeclBlockItem& declBlockItem)
{
    declBlockItem.decl->accept(*this);
}

void ConstASTTraverser::visit(const Declaration& declaration)
{
    if (declaration.init)
        declaration.init->accept(*this);
}

void ConstASTTraverser::visit(const ReturnStmt& returnStmt)
{
    returnStmt.expr->accept(*this);
}

void ConstASTTraverser::visit(const ExprStmt& exprStmt)
{
    exprStmt.expr->accept(*this);
}

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
} // Parsing