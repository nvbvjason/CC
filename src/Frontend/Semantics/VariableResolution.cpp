#include "VariableResolution.hpp"

namespace Semantics {
bool VariableResolution::resolve()
{
    resolveFunction(*program.function);
    return m_valid;
}

void VariableResolution::resolveFunction(Parsing::Function& func)
{
    for (std::unique_ptr<Parsing::BlockItem>& blockItem: func.body) {
        if (!m_valid)
            return;
        resolveBlockItem(*blockItem);
    }
}

void VariableResolution::resolveBlockItem(Parsing::BlockItem& blockItem)
{
    if (!m_valid)
        return;
    switch (blockItem.kind) {
        case Parsing::BlockItem::Kind::Declaration: {
            auto declaration = static_cast<Parsing::DeclBlockItem*>(&blockItem);
            resolveDeclaration(*declaration->decl);
            break;
        }
        case Parsing::BlockItem::Kind::Statement: {
            auto declaration = static_cast<Parsing::StmtBlockItem*>(&blockItem);
            resolveStmt(*declaration->stmt);
            break;
        }
    }
}

void VariableResolution::resolveDeclaration(Parsing::Declaration& declaration)
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

void VariableResolution::resolveStmt(Parsing::Stmt& stmt)
{
    using Kind = Parsing::Stmt::Kind;
    if (!m_valid)
        return;
    switch (stmt.kind) {
        case Kind::Expression: {
            auto exprStmt = static_cast<Parsing::ExprStmt*>(&stmt);
            resolveExpr(*exprStmt->expr);
            break;
        }
        case Kind::Return: {
            auto returnStmt = static_cast<Parsing::ReturnStmt*>(&stmt);
            resolveExpr(*returnStmt->expr);
            break;
        }
        case Kind::Null:
            break;
    }
}

void VariableResolution::resolveExpr(Parsing::Expr& declaration)
{
    using Kind = Parsing::Expr::Kind;
    if (!m_valid)
        return;
    switch (declaration.kind) {
        case Kind::Constant:
            break;
        case Kind::Var: {
            auto varExpr = static_cast<Parsing::VarExpr*>(&declaration);
            if (!variableMap.contains(varExpr->name)) {
                m_valid = false;
                return;
            }
            varExpr->name = variableMap.at(varExpr->name);
            break;
        }
        case Kind::Unary: {
            auto unaryExpr = static_cast<Parsing::UnaryExpr*>(&declaration);
            resolveExpr(*unaryExpr->operand);
            break;
        }
        case Kind::Binary: {
            auto binaryExpr = static_cast<Parsing::BinaryExpr*>(&declaration);
            resolveExpr(*binaryExpr->lhs);
            resolveExpr(*binaryExpr->rhs);
            break;
        }
        case Kind::Assignment: {
            auto assignmentExpr = static_cast<Parsing::AssignmentExpr*>(&declaration);
            if (assignmentExpr->lhs->kind != Kind::Var) {
                m_valid = false;
                return;
            }
            auto varExpr = static_cast<Parsing::VarExpr*>(assignmentExpr->lhs.get());
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
} // Semantics