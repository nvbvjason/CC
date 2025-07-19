#pragma once

#include "ASTTraverser.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"

namespace Semantics {
class ValidateReturn : public Parsing::ASTTraverser {
    bool m_hasValidReturns = true;
public:
    bool programValidate(Parsing::Program& program);
private:
    static void addReturnZero(Parsing::FunDecl& funDecl);
    void visit(Parsing::FunDecl& funDecl) override;
};

inline void ValidateReturn::addReturnZero(Parsing::FunDecl& funDecl)
{
    auto zeroConstExpr = std::make_unique<Parsing::ConstExpr>(0, std::make_unique<Parsing::VarType>(Type::I32));
    auto returnStmt = std::make_unique<Parsing::ReturnStmt>(std::move(zeroConstExpr));
    auto returnBlockStmt = std::make_unique<Parsing::StmtBlockItem>(std::move(returnStmt));
    funDecl.body->body.push_back(std::move(returnBlockStmt));
}
} // Semantics