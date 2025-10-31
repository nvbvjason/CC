#include "ASTTraverser.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"

namespace Parsing {

void ASTTraverser::visit(Program& program)
{
    for (auto& decl : program.declarations)
        decl->accept(*this);
}

void ASTTraverser::visit(VarDecl& varDecl)
{
    if (varDecl.init)
        varDecl.init->accept(*this);
}

// Declaration
void ASTTraverser::visit(FunDeclaration& funDecl)
{
    if (funDecl.body)
        funDecl.body->accept(*this);
}

void ASTTraverser::visit(Block& block)
{
    for (auto& blockItem : block.body)
        blockItem->accept(*this);
}

// BlockItem
void ASTTraverser::visit(StmtBlockItem& stmtBlockItem)
{
    stmtBlockItem.stmt->accept(*this);
}

void ASTTraverser::visit(DeclBlockItem& declBlockItem)
{
    declBlockItem.decl->accept(*this);
}

void ASTTraverser::visit(FuncType& functionType)
{
    functionType.returnType->accept(*this);
    for (const std::unique_ptr<TypeBase>& type : functionType.params)
        type->accept(*this);
}

void ASTTraverser::visit(PointerType& pointerType)
{
    pointerType.referenced->accept(*this);
}

void ASTTraverser::visit(ArrayType& arrayType)
{
    arrayType.elementType->accept(*this);
}

void ASTTraverser::visit(SingleInitializer& singleInitializer)
{
    singleInitializer.exp->accept(*this);
}

void ASTTraverser::visit(CompoundInitializer& compoundInitializer)
{
    for (const auto& initializer : compoundInitializer.initializers)
        initializer->accept(*this);
}

// ForInit
void ASTTraverser::visit(DeclForInit& declForInit)
{
    declForInit.decl->accept(*this);
}

void ASTTraverser::visit(ExprForInit& exprForInit)
{
    if (exprForInit.expression != nullptr)
        exprForInit.expression->accept(*this);
}

// Statements
void ASTTraverser::visit(ReturnStmt& returnStmt)
{
    returnStmt.expr->accept(*this);
}

void ASTTraverser::visit(ExprStmt& exprStmt)
{
    exprStmt.expr->accept(*this);
}

void ASTTraverser::visit(IfStmt& ifStmt)
{
    ifStmt.condition->accept(*this);
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt)
        ifStmt.elseStmt->accept(*this);
}

void ASTTraverser::visit(CompoundStmt& compoundStmt)
{
    compoundStmt.block->accept(*this);
}

void ASTTraverser::visit(LabelStmt& labelStmt)
{
    labelStmt.stmt->accept(*this);
}

void ASTTraverser::visit(CaseStmt& caseStmt)
{
    caseStmt.condition->accept(*this);
    caseStmt.body->accept(*this);
}

void ASTTraverser::visit(DefaultStmt& defaultStmt)
{
    defaultStmt.body->accept(*this);
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
    if (forStmt.init)
        forStmt.init->accept(*this);
    if (forStmt.condition)
        forStmt.condition->accept(*this);
    if (forStmt.post)
        forStmt.post->accept(*this);
    forStmt.body->accept(*this);
}

void ASTTraverser::visit(SwitchStmt& switchStmt)
{
    switchStmt.condition->accept(*this);
    switchStmt.body->accept(*this);
}

// Expression
void ASTTraverser::visit(CastExpr& castExpr)
{
    castExpr.expr->accept(*this);
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

void ASTTraverser::visit(TernaryExpr& conditionalExpr)
{
    conditionalExpr.condition->accept(*this);
    conditionalExpr.trueExpr->accept(*this);
    conditionalExpr.falseExpr->accept(*this);
}

void ASTTraverser::visit(FuncCallExpr& functionCallExpr)
{
    for (auto& expr : functionCallExpr.args)
        expr->accept(*this);
}

void ASTTraverser::visit(DereferenceExpr& dereferenceExpr)
{
    dereferenceExpr.reference->accept(*this);
}

void ASTTraverser::visit(AddrOffExpr& addrOffExpr)
{
    addrOffExpr.reference->accept(*this);
}

void ASTTraverser::visit(SubscriptExpr& subscriptExpr)
{
    subscriptExpr.referencing->accept(*this);
    subscriptExpr.index->accept(*this);
}
} // Parsing