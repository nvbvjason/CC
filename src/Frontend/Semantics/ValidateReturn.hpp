#pragma once

#include "ASTTraverser.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"
#include "Error.hpp"

namespace Semantics {
class ValidateReturn : public Parsing::ASTTraverser {
    std::vector<Error> m_errors;
public:
    std::vector<Error> programValidate(Parsing::Program& program);
private:
    static void addReturnZero(Parsing::FuncDecl& funDecl);
    void visit(Parsing::FuncDecl& funDecl) override;
};

inline void ValidateReturn::addReturnZero(Parsing::FuncDecl& funDecl)
{
    auto zeroConstExpr = std::make_unique<Parsing::ConstExpr>(0, std::make_unique<Parsing::VarType>(Type::I32));
    auto returnStmt = std::make_unique<Parsing::ReturnStmt>(std::move(zeroConstExpr));
    auto returnBlockStmt = std::make_unique<Parsing::StmtBlockItem>(std::move(returnStmt));
    funDecl.body->body.push_back(std::move(returnBlockStmt));
}
} // Semantics