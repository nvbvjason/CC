#include "ValidateReturn.hpp"
#include "ASTDeepCopy.hpp"
#include "DynCast.hpp"
#include "Utils.hpp"

#include <cassert>

namespace Semantics {
std::vector<Error> ValidateReturn::programValidate(Parsing::Program& program)
{
    program.accept(*this);
    return std::move(m_errors);
}

void ValidateReturn::visit(Parsing::FuncDeclaration& funDecl)
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
            m_errors.emplace_back("Cannot convert return type to pointer ", returnStmt->expr->location);
            return;
        }
        returnStmt->expr = std::make_unique<Parsing::CastExpr>(
            Parsing::deepCopy(*funcType->returnType), std::move(returnStmt->expr));
    }
    if (funcType->returnType->type == Type::Void) {
        if (returnStmt->expr) {
            m_errors.emplace_back("Cannot return expression from function with return type void",
                returnStmt->expr->location);
        }
        return;
    }
    if (!returnStmt->expr) {
        m_errors.emplace_back("Must have return expression on non void function",returnStmt->location);
        return;
    }
    assert(funcType->returnType->type);
    assert(returnStmt->expr->type);
    assert(returnStmt->expr->type->type);
    if (funcType->returnType->type == Type::Pointer || returnStmt->expr->type->type == Type::Pointer) {
        if (!Parsing::areEquivalentTypes(*funcType->returnType, *returnStmt->expr->type)) {
            m_errors.emplace_back("Return type does not conform to function return type ",
                                returnStmt->expr->location);
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