#include "ValidateReturn.hpp"

#include "DynCast.hpp"
#include "Utils.hpp"

namespace Semantics {
bool ValidateReturn::programValidate(Parsing::Program& program)
{
    m_hasValidReturns = true;
    program.accept(*this);
    return m_hasValidReturns;
}

void ValidateReturn::visit(Parsing::FunDecl& funDecl)
{
    if (funDecl.body == nullptr)
        return;
    if (funDecl.body->body.empty()) {
        addReturnZero(funDecl);
        return;
    }

    for (std::unique_ptr<Parsing::BlockItem>& blockItem : funDecl.body->body)
        blockItem->accept(*this);

    const auto& lastBlockItem = funDecl.body->body.back();
    if (lastBlockItem->kind != Parsing::BlockItem::Kind::Statement) {
        addReturnZero(funDecl);
        return;
    }

    const auto stmtBlockItem = dynCast<Parsing::StmtBlockItem>(lastBlockItem.get());
    if (stmtBlockItem->stmt->kind != Parsing::Stmt::Kind::Return) {
        addReturnZero(funDecl);
        return;
    }
    const auto returnStmt = dynCast<Parsing::ReturnStmt>(stmtBlockItem->stmt.get());
    const auto funcType = dynCast<const Parsing::FuncType>(funDecl.type.get());
    if (funcType->returnType->type == Type::Pointer && returnStmt->expr->type->type != Type::Pointer) {
        if (!canConvertToNullPtr(*returnStmt->expr)) {
            m_hasValidReturns = false;
            return;
        }
        returnStmt->expr = std::make_unique<Parsing::CastExpr>(
            Parsing::deepCopy(*funcType->returnType), std::move(returnStmt->expr));
    }
    if (funcType->returnType->type == Type::Pointer || returnStmt->expr->type->type == Type::Pointer) {
        if (!Parsing::areEquivalent(*funcType->returnType, *returnStmt->expr->type)) {
            m_hasValidReturns = false;
            return;
        }
    }
    if (funcType->returnType->type != returnStmt->expr->type->type) {
        returnStmt->expr = std::make_unique<Parsing::CastExpr>(
            std::make_unique<Parsing::VarType>(funcType->returnType->type),
            std::move(returnStmt->expr));
    }
}
} // Semantics