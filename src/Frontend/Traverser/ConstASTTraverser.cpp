#include "../Traverser/ConstASTTraverser.hpp"
#include "../AST/ASTParser.hpp"

namespace Parsing {

void ConstASTTraverser::visit(const Program& program)
{
    program.function->accept(*this);
}

void ConstASTTraverser::visit(const Function& function)
{
    function.body->accept(*this);
}

void ConstASTTraverser::visit(const Block& block)
{
    for (const std::unique_ptr<BlockItem>& blockItem : block.body)
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

void ConstASTTraverser::visit(const IfStmt& ifStmt)
{
    ifStmt.condition->accept(*this);
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt)
        ifStmt.elseStmt->accept(*this);
}

void ConstASTTraverser::visit(const ReturnStmt& returnStmt)
{
    returnStmt.expr->accept(*this);
}

void ConstASTTraverser::visit(const ExprStmt& exprStmt)
{
    exprStmt.expr->accept(*this);
}

void ConstASTTraverser::visit(const CompoundStmt& compoundStmt)
{
    compoundStmt.block->accept(*this);
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

void ConstASTTraverser::visit(const ConditionalExpr& conditionalExpr)
{
    conditionalExpr.condition->accept(*this);
    conditionalExpr.first->accept(*this);
    conditionalExpr.second->accept(*this);
}
} // Parsing