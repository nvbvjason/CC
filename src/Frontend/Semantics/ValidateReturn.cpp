#include "ValidateReturn.hpp"

namespace Semantics {
bool ValidateReturn::programValidate(Parsing::Program& program)
{
    m_hasValidReturns = true;
    program.accept(*this);
    return m_hasValidReturns;
}

bool ValidateReturn::functionValidate(Parsing::Function& function)
{
    function.accept(*this);
    return m_hasValidReturns;
}

void ValidateReturn::visit(Parsing::Function& function)
{
    if (function.body.empty()) {
            addReturnZero(function);
        return;
    }

    for (std::unique_ptr<Parsing::BlockItem>& blockItem : function.body)
        blockItem->accept(*this);

    auto& lastBlockItem = function.body.back();
    if (lastBlockItem->kind != Parsing::BlockItem::Kind::Statement) {
        addReturnZero(function);
        return;
    }

    auto* stmtBlockItem = dynamic_cast<Parsing::StmtBlockItem*>(lastBlockItem.get());
    if (stmtBlockItem->stmt->kind != Parsing::Stmt::Kind::Return)
        addReturnZero(function);
}

void ValidateReturn::visit(Parsing::StmtBlockItem& stmtBlockItem)
{
    if (stmtBlockItem.stmt->kind == Parsing::Stmt::Kind::Return)
        m_hasValidReturns = true;
    stmtBlockItem.stmt->accept(*this);
}
} // Semantics