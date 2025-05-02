#include "ValidateReturn.hpp"

namespace Parsing {
bool ValidateReturn::programValidate(Program& program)
{
    return functionValidate(*program.function);
}

void ValidateReturn::addReturnZero(Function& function)
{
    auto zeroConstExpr = std::make_unique<ConstExpr>(0);
    auto returnStmt = std::make_unique<ReturnStmt>(std::move(zeroConstExpr));
    auto returnBlockStmt = std::make_unique<StmtBlockItem>(std::move(returnStmt));
    function.body.push_back(std::move(returnBlockStmt));
}

bool ValidateReturn::functionValidate(Function& function)
{
    if (function.body.empty()) {
        addReturnZero(function);
        return true;
    }
    auto lastBlockItem = *function.body.back();
    if (lastBlockItem.kind != BlockItem::Kind::Statement) {
        addReturnZero(function);
        return true;
    }
    const auto stmtBlockItem = static_cast<StmtBlockItem*>(&lastBlockItem);
    if (stmtBlockItem->stmt->kind != Stmt::Kind::Return) {
        addReturnZero(function);
        return true;
    }
    return true;
}

} // Parsing