#include "VariableResolution.hpp"

namespace Parsing {
bool VariableResolution::resolve()
{
    resolveFunction(*program.function);
    return m_valid;
}

void VariableResolution::resolveFunction(Function& func)
{
    for (std::unique_ptr<BlockItem>& blockItem: func.body) {
        if (!m_valid)
            return;
        resolveBlockItem(*blockItem);
    }
}

void VariableResolution::resolveBlockItem(BlockItem& blockItem)
{
    if (!m_valid)
        return;
    switch (blockItem.kind) {
        case BlockItem::Kind::Declaration: {
            auto declaration = static_cast<DeclarationBlockItem*>(&blockItem);
            resolveDeclaration(*declaration->decl);
            break;
        }
        case BlockItem::Kind::Statement: {
            auto declaration = static_cast<StmtBlockItem*>(&blockItem);
            resolveStmt(*declaration->stmt);
            break;
        }
    }
}

void VariableResolution::resolveDeclaration(Declaration& declaration)
{
    if (!m_valid)
        return;
    if (variableMap.contains(declaration.name)) {
        m_valid = false;
        return;
    }
    std::string uniqueName = makeTemporary(declaration.name);
    variableMap[declaration.name] = uniqueName;
    declaration.name = uniqueName;
    if (declaration.init != nullptr)
        resolveExpr(*declaration.init);
}

void VariableResolution::resolveStmt(Stmt& stmt)
{
    if (!m_valid)
        return;
    switch (stmt.kind) {
        case Stmt::Kind::Expression: {
            auto exprStmt = static_cast<ExprStmt*>(&stmt);
            resolveExpr(*exprStmt->expression);
            break;
        }
        case Stmt::Kind::Return: {
            auto returnStmt = static_cast<ReturnStmt*>(&stmt);
            resolveExpr(*returnStmt->expression);
            break;
        }
        case Stmt::Kind::Null:
            break;
    }
}

void VariableResolution::resolveExpr(Expr& declaration)
{
    if (!m_valid)
        return;
    switch (declaration.kind) {
        case Expr::Kind::Constant:
            break;
        case Expr::Kind::Var: {
            auto varExpr = static_cast<VarExpr*>(&declaration);
            if (!variableMap.contains(varExpr->name)) {
                m_valid = false;
                return;
            }
            varExpr->name = variableMap.at(varExpr->name);
            break;
        }
        case Expr::Kind::Unary: {
            auto unaryExpr = static_cast<UnaryExpr*>(&declaration);
            resolveExpr(*unaryExpr->operand);
            break;
        }
        case Expr::Kind::Binary: {
            auto binaryExpr = static_cast<BinaryExpr*>(&declaration);
            resolveExpr(*binaryExpr->lhs);
            resolveExpr(*binaryExpr->rhs);
            break;
        }
        case Expr::Kind::Assignment: {
            auto assignmentExpr = static_cast<AssignmentExpr*>(&declaration);
            if (assignmentExpr->lhs->kind != Expr::Kind::Var) {
                m_valid = false;
                return;
            }
            auto varExpr = static_cast<VarExpr*>(assignmentExpr->lhs.get());
            if (!variableMap.contains(varExpr->name)) {
                m_valid = false;
                return;
            }
            varExpr->name = variableMap.at(varExpr->name);
            resolveExpr(*assignmentExpr->rhs);
            break;
        }
    }
}

std::string VariableResolution::makeTemporary(const std::string& name)
{
    return name + '.' + std::to_string(m_counter++);
}
} // Parsing