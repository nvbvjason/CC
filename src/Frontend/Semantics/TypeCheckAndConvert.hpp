#pragma once

#include "ASTTraverser.hpp"

#include <memory>

namespace Semantics {

class TypeCheckAndConvert final : Parsing::ASTTraverser {
public:
    void convert(Parsing::Program& program);

    void visit(Parsing::FunDeclaration& funDeclaration) override;

    void visit(Parsing::ReturnStmt& stmt) override;
    void visit(Parsing::ExprStmt& stmt) override;
    void visit(Parsing::IfStmt& ifStmt) override;
    void visit(Parsing::CaseStmt& stmt) override;
    void visit(Parsing::WhileStmt& stmt) override;
    void visit(Parsing::DoWhileStmt& stmt) override;
    void visit(Parsing::ForStmt& stmt) override;
    void visit(Parsing::SwitchStmt& stmt) override;
};

std::unique_ptr<Parsing::Expr> convertArrayType(const Parsing::Expr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::Expr& expr);

std::unique_ptr<Parsing::Expr> convert(const Parsing::ConstExpr& expr);
std::unique_ptr<Parsing::Expr> convert(const Parsing::VarExpr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::CastExpr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::UnaryExpr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::BinaryExpr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::AssignmentExpr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::TernaryExpr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::FuncCallExpr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::DereferenceExpr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::AddrOffExpr& expr);
std::unique_ptr<Parsing::Expr> convert(Parsing::SubscriptExpr& expr);

} // Semantics