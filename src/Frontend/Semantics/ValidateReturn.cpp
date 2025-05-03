#include "ValidateReturn.hpp"

namespace Semantics {
bool ValidateReturn::programValidate(Parsing::Program& program)
{
    return functionValidate(*program.function);
}

void ValidateReturn::addReturnZero(Parsing::Function& function)
{
    auto zeroConstExpr = std::make_unique<Parsing::ConstExpr>(0);
    auto returnStmt = std::make_unique<Parsing::ReturnStmt>(std::move(zeroConstExpr));
    auto returnBlockStmt = std::make_unique<Parsing::StmtBlockItem>(std::move(returnStmt));
    function.body.push_back(std::move(returnBlockStmt));
}

bool ValidateReturn::functionValidate(Parsing::Function& function)
{
    if (function.body.empty()) {
        addReturnZero(function);
        return true;
    }
    auto& lastBlockItemUnique = function.body.back();
    auto* lastBlockItem = lastBlockItemUnique.get();
    if (lastBlockItem->kind != Parsing::BlockItem::Kind::Statement) {
        addReturnZero(function);
        return true;
    }
    auto* stmtBlockItem = dynamic_cast<Parsing::StmtBlockItem*>(lastBlockItem);
    if (stmtBlockItem->stmt->kind != Parsing::Stmt::Kind::Return) {
        addReturnZero(function);
        return true;
    }
    return true;
}

} // Semantics