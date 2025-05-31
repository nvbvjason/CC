#include "ValidateReturn.hpp"

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

    auto& lastBlockItem = funDecl.body->body.back();
    if (lastBlockItem->kind != Parsing::BlockItem::Kind::Statement) {
        addReturnZero(funDecl);
        return;
    }

    auto* stmtBlockItem = dynamic_cast<Parsing::StmtBlockItem*>(lastBlockItem.get());
    if (stmtBlockItem->stmt->kind != Parsing::Stmt::Kind::Return)
        addReturnZero(funDecl);
}

void ValidateReturn::visit(Parsing::StmtBlockItem& stmtBlockItem)
{
    if (stmtBlockItem.stmt->kind == Parsing::Stmt::Kind::Return)
        m_hasValidReturns = true;
    stmtBlockItem.stmt->accept(*this);
}
} // Semantics