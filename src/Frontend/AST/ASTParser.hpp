#pragma once

#ifndef CC_PARSING_ABSTRACT_TREE_HPP
#define CC_PARSING_ABSTRACT_TREE_HPP

#include "ASTVisitor.hpp"
#include "ASTExpr.hpp"
#include "ASTBase.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Parsing {

struct VarDecl final : Declaration {
    std::string name;
    std::unique_ptr<Expr> init = nullptr;
    std::unique_ptr<TypeBase> type;

    explicit VarDecl(const StorageClass storageClass, std::string name, std::unique_ptr<TypeBase>&& type)
        : Declaration(Kind::VarDecl, storageClass), name(std::move(name)), type(std::move(type)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    VarDecl() = delete;
};

struct DeclForInit final : ForInit {
    std::unique_ptr<VarDecl> decl;

    explicit DeclForInit(std::unique_ptr<VarDecl> decl)
        : ForInit(Kind::Declaration), decl(std::move(decl)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    DeclForInit() = delete;
};

struct ExprForInit final : ForInit {
    std::unique_ptr<Expr> expression = nullptr;

    explicit ExprForInit(std::unique_ptr<Expr> expr)
        : ForInit(Kind::Expression), expression(std::move(expr)) {}

    ExprForInit()
        : ForInit(Kind::Expression) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct StmtBlockItem final : BlockItem {
    std::unique_ptr<Stmt> stmt;

    explicit StmtBlockItem(std::unique_ptr<Stmt> stmt)
        : BlockItem(Kind::Statement), stmt(std::move(stmt)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    StmtBlockItem() = delete;
};

struct DeclBlockItem final : BlockItem {
    std::unique_ptr<Declaration> decl;

    explicit DeclBlockItem(std::unique_ptr<Declaration> decl)
        : BlockItem(Kind::Declaration), decl(std::move(decl)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    DeclBlockItem() = delete;
};

struct Block {
    std::vector<std::unique_ptr<BlockItem>> body;

    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

struct FunDecl final : Declaration {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<Block> body = nullptr;
    std::unique_ptr<TypeBase> type = nullptr;

    explicit FunDecl(const StorageClass storageClass,
                     std::string name,
                     std::vector<std::string>&& ps,
                     std::unique_ptr<TypeBase>&& t)
        : Declaration(Kind::FuncDecl, storageClass),
            name(std::move(name)),
            params(std::move(ps)),
            type(std::move(t)){}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    FunDecl() = delete;
};

struct ReturnStmt final : Stmt {
    std::unique_ptr<Expr> expr;

    explicit ReturnStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Return), expr(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    ReturnStmt() = delete;
};

struct ExprStmt final : Stmt {
    std::unique_ptr<Expr> expr;

    explicit ExprStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Expression), expr(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    ExprStmt() = delete;
};

struct IfStmt final : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenStmt;
    std::unique_ptr<Stmt> elseStmt = nullptr;

    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenStmt)
        : Stmt(Kind::If), condition(std::move(condition)), thenStmt(std::move(thenStmt)) {}

    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenStmt, std::unique_ptr<Stmt> elseStmt)
        : Stmt(Kind::If), condition(std::move(condition)), thenStmt(std::move(thenStmt)),
          elseStmt(std::move(elseStmt)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    IfStmt() = delete;
};

struct GotoStmt final : Stmt {
    std::string identifier;

    explicit GotoStmt(std::string iden)
        : Stmt(Kind::Goto), identifier(std::move(iden)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    GotoStmt() = delete;
};

struct CompoundStmt final : Stmt {
    std::unique_ptr<Block> block;

    explicit CompoundStmt(std::unique_ptr<Block> block)
        : Stmt(Kind::Compound), block(std::move(block)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    CompoundStmt() = delete;
};

struct BreakStmt final : Stmt {
    std::string identifier;

    explicit BreakStmt()
        : Stmt(Kind::Break) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct ContinueStmt final : Stmt {
    std::string identifier;

    explicit ContinueStmt()
        : Stmt(Kind::Continue) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct LabelStmt final : Stmt {
    std::string identifier;
    std::unique_ptr<Stmt> stmt;

    explicit LabelStmt(std::string name, std::unique_ptr<Stmt> stmt)
        : Stmt(Kind::Label), identifier(std::move(name)), stmt(std::move(stmt)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    LabelStmt() = delete;
};

struct CaseStmt final : Stmt {
    std::string identifier;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    CaseStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(Kind::Case), condition(std::move(condition)), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    CaseStmt() = delete;
};

struct DefaultStmt final : Stmt {
    std::string identifier;
    std::unique_ptr<Stmt> body;
    explicit DefaultStmt(std::unique_ptr<Stmt> body)
        : Stmt(Kind::Default), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    DefaultStmt() = delete;
};

struct WhileStmt final : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    std::string identifier;

    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(Kind::While), condition(std::move(condition)), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    WhileStmt() = delete;
};

struct DoWhileStmt final : Stmt {
    std::unique_ptr<Stmt> body;
    std::unique_ptr<Expr> condition;
    std::string identifier;

    DoWhileStmt(std::unique_ptr<Stmt> body, std::unique_ptr<Expr> condition)
        : Stmt(Kind::DoWhile), body(std::move(body)), condition(std::move(condition)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    DoWhileStmt() = delete;
};

struct ForStmt final : Stmt {
    std::unique_ptr<ForInit> init = nullptr;
    std::unique_ptr<Expr> condition = nullptr;
    std::unique_ptr<Expr> post = nullptr;
    std::unique_ptr<Stmt> body;
    std::string identifier;

    explicit ForStmt(std::unique_ptr<Stmt> body)
        : Stmt(Kind::For), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    ForStmt() = delete;
};

struct SwitchStmt final : Stmt {
    std::string identifier;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    std::vector<std::variant<i32, i64, u32, u64>> cases;

    bool hasDefault = false;

    SwitchStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(Kind::Switch), condition(std::move(condition)), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    SwitchStmt() = delete;
};

struct NullStmt final : Stmt {
    NullStmt()
        : Stmt(Kind::Null) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct Program {
    std::vector<std::unique_ptr<Declaration>> declarations;
    ~Program() = default;
    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

} // namespace Parsing

#endif // CC_PARSING_ABSTRACT_TREE_HPP