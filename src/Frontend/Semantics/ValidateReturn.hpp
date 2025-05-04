#pragma once

#ifndef CC_PARSING_VALIDATE_RETURN_HPP
#define CC_PARSING_VALIDATE_RETURN_HPP

#include "Frontend/Traverser/ASTTraverser.hpp"
#include "ASTParser.hpp"

namespace Semantics {
class ValidateReturn : public Parsing::ASTTraverser {
    bool m_hasValidReturns = true;
public:
    bool programValidate(Parsing::Program& program);
private:
    static void addReturnZero(Parsing::Function& function);
    void visit(Parsing::Function& function) override;
    void visit(Parsing::StmtBlockItem& stmtBlockItem) override;
};

inline void ValidateReturn::addReturnZero(Parsing::Function& function)
{
    auto zeroConstExpr = std::make_unique<Parsing::ConstExpr>(0);
    auto returnStmt = std::make_unique<Parsing::ReturnStmt>(std::move(zeroConstExpr));
    auto returnBlockStmt = std::make_unique<Parsing::StmtBlockItem>(std::move(returnStmt));
    function.body->body.push_back(std::move(returnBlockStmt));
}
} // Semantics

#endif // CC_PARSING_VALIDATE_RETURN_HPP
