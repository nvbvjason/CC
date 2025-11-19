#pragma once

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
    std::unique_ptr<Initializer> init = nullptr;
    std::unique_ptr<TypeBase> type;

    VarDecl(const StorageClass storageClass, std::string name, std::unique_ptr<TypeBase> type)
        : Declaration(Kind::VarDecl, storageClass), name(std::move(name)), type(std::move(type)) {}

    VarDecl(const i64 loc, const StorageClass storageClass, std::string name, std::unique_ptr<TypeBase> type)
        : Declaration(loc, Kind::VarDecl, storageClass), name(std::move(name)), type(std::move(type)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Declaration* decl) { return decl->kind == Kind::VarDecl; }

    VarDecl() = delete;
};

struct DeclForInit final : ForInit {
    std::unique_ptr<VarDecl> decl;

    explicit DeclForInit(std::unique_ptr<VarDecl> decl)
        : ForInit(Kind::Declaration), decl(std::move(decl)) {}

    DeclForInit(const i64 loc, std::unique_ptr<VarDecl> decl)
        : ForInit(loc, Kind::Declaration), decl(std::move(decl)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const ForInit* forInit) { return forInit->kind == Kind::Declaration; }

    DeclForInit() = delete;
};

struct ExprForInit final : ForInit {
    std::unique_ptr<Expr> expression = nullptr;

    explicit ExprForInit(std::unique_ptr<Expr> expr)
        : ForInit(Kind::Expression), expression(std::move(expr)) {}

    ExprForInit(const i64 loc, std::unique_ptr<Expr> expr)
        : ForInit(loc, Kind::Expression), expression(std::move(expr)) {}

    ExprForInit()
        : ForInit(Kind::Expression) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const ForInit* forInit) { return forInit->kind == Kind::Expression; }
};

struct StmtBlockItem final : BlockItem {
    std::unique_ptr<Stmt> stmt;

    explicit StmtBlockItem(std::unique_ptr<Stmt> stmt)
        : BlockItem(Kind::Statement), stmt(std::move(stmt)) {}

    StmtBlockItem(const i64 loc, std::unique_ptr<Stmt> stmt)
        : BlockItem(loc, Kind::Statement), stmt(std::move(stmt)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const BlockItem* blockItem) { return blockItem->kind == Kind::Statement; }

    StmtBlockItem() = delete;
};

struct DeclBlockItem final : BlockItem {
    std::unique_ptr<Declaration> decl;

    explicit DeclBlockItem(std::unique_ptr<Declaration> decl)
        : BlockItem(Kind::Declaration), decl(std::move(decl)) {}

    DeclBlockItem(const i64 loc, std::unique_ptr<Declaration> decl)
        : BlockItem(loc, Kind::Declaration), decl(std::move(decl)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const BlockItem* blockItem) { return blockItem->kind == Kind::Declaration; }

    DeclBlockItem() = delete;
};

struct Block {
    std::vector<std::unique_ptr<BlockItem>> body;

    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

struct FuncDecl final : Declaration {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<Block> body = nullptr;
    std::unique_ptr<TypeBase> type = nullptr;

    FuncDecl(const StorageClass storageClass,
             std::string name,
             std::vector<std::string>&& ps,
             std::unique_ptr<TypeBase>&& t)
        : Declaration(Kind::FuncDecl, storageClass),
            name(std::move(name)),
            params(std::move(ps)),
            type(std::move(t)){}

    FuncDecl(const i64 loc,
             const StorageClass storageClass,
             std::string name,
             std::vector<std::string>&& ps,
             std::unique_ptr<TypeBase>&& t)
    : Declaration(loc, Kind::FuncDecl, storageClass),
            name(std::move(name)),
            params(std::move(ps)),
            type(std::move(t)){}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Declaration* declaration) { return declaration->kind == Kind::FuncDecl; }

    FuncDecl() = delete;
};

struct MemberDecl final : Declaration {
    const std::string identifier;
    std::unique_ptr<TypeBase> type;

    MemberDecl(const i64 loc,
               std::string identifier,
               std::unique_ptr<TypeBase>&& type)
        : Declaration(loc, Kind::MemberDecl, StorageClass::None),
            identifier(std::move(identifier)),
            type(std::move(type)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Declaration* declaration) { return declaration->kind == Kind::MemberDecl; }

    MemberDecl() = delete;
};

struct StructuredDecl final : Declaration {
    std::string identifier;
    std::vector<std::unique_ptr<MemberDecl>> members;
    const Type type;

    StructuredDecl(const i64 loc,
                   std::string identifier,
                   std::vector<std::unique_ptr<MemberDecl>>&& members,
                   const Type type)
        : Declaration(loc, Kind::StructuredDecl, StorageClass::None),
            identifier(std::move(identifier)),
            members(std::move(members)),
            type(type) {}

    [[nodiscard]] bool isStruct() const { return type == Type::Struct; }
    [[nodiscard]] bool isUnion() const { return type == Type::Union; }

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Declaration* declaration) { return declaration->kind == Kind::StructuredDecl; }

    StructuredDecl() = delete;
};

struct ReturnStmt final : Stmt {
    std::unique_ptr<Expr> expr;

    explicit ReturnStmt(const i64 loc)
        : Stmt(loc, Kind::Return), expr(nullptr) {}

    explicit ReturnStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Return), expr(std::move(expr)) {}

    ReturnStmt(const i64 loc, std::unique_ptr<Expr> expr)
        : Stmt(loc, Kind::Return), expr(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Return; }

    ReturnStmt() = delete;
};

struct ExprStmt final : Stmt {
    std::unique_ptr<Expr> expr;

    explicit ExprStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Expression), expr(std::move(expr)) {}

    ExprStmt(const i64 loc, std::unique_ptr<Expr> expr)
        : Stmt(loc, Kind::Expression), expr(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Expression; }

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

    IfStmt(const i64 loc, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenStmt)
        : Stmt(loc, Kind::If), condition(std::move(condition)), thenStmt(std::move(thenStmt)) {}

    IfStmt(const i64 loc,
           std::unique_ptr<Expr> condition,
           std::unique_ptr<Stmt> thenStmt,
           std::unique_ptr<Stmt> elseStmt)
        : Stmt(loc, Kind::If), condition(std::move(condition)),
          thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::If; }

    IfStmt() = delete;
};

struct GotoStmt final : Stmt {
    std::string identifier;

    explicit GotoStmt(std::string iden)
        : Stmt(Kind::Goto), identifier(std::move(iden)) {}

    GotoStmt(const i64 loc, std::string iden)
       : Stmt(loc, Kind::Goto), identifier(std::move(iden)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Goto; }

    GotoStmt() = delete;
};

struct CompoundStmt final : Stmt {
    std::unique_ptr<Block> block;

    explicit CompoundStmt(std::unique_ptr<Block> block)
        : Stmt(Kind::Compound), block(std::move(block)) {}

    CompoundStmt(const i64 loc, std::unique_ptr<Block> block)
        : Stmt(loc, Kind::Compound), block(std::move(block)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Compound; }

    CompoundStmt() = delete;
};

struct BreakStmt final : Stmt {
    std::string identifier;

    explicit BreakStmt()
        : Stmt(Kind::Break) {}

    explicit BreakStmt(const i64 loc)
        : Stmt(loc, Kind::Break) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Break; }
};

struct ContinueStmt final : Stmt {
    std::string identifier;

    explicit ContinueStmt()
        : Stmt(Kind::Continue) {}

    explicit ContinueStmt(const i64 loc)
        : Stmt(loc, Kind::Continue) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Continue; }
};

struct LabelStmt final : Stmt {
    std::string identifier;
    std::unique_ptr<Stmt> stmt;

    explicit LabelStmt(std::string name, std::unique_ptr<Stmt> stmt)
        : Stmt(Kind::Label), identifier(std::move(name)), stmt(std::move(stmt)) {}

    LabelStmt(const i64 loc, std::string name, std::unique_ptr<Stmt> stmt)
        : Stmt(loc, Kind::Label), identifier(std::move(name)), stmt(std::move(stmt)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Label; }

    LabelStmt() = delete;
};

struct CaseStmt final : Stmt {
    std::string identifier;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    CaseStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(Kind::Case), condition(std::move(condition)), body(std::move(body)) {}

    CaseStmt(const i64 loc, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(loc, Kind::Case), condition(std::move(condition)), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Case; }

    CaseStmt() = delete;
};

struct DefaultStmt final : Stmt {
    std::string identifier;
    std::unique_ptr<Stmt> body;

    explicit DefaultStmt(std::unique_ptr<Stmt> body)
        : Stmt(Kind::Default), body(std::move(body)) {}

    DefaultStmt(const i64 loc, std::unique_ptr<Stmt> body)
        : Stmt(loc, Kind::Default), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Default; }

    DefaultStmt() = delete;
};

struct WhileStmt final : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    std::string identifier;

    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(Kind::While), condition(std::move(condition)), body(std::move(body)) {}

    WhileStmt(const i64 loc, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(loc, Kind::While), condition(std::move(condition)), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::While; }

    WhileStmt() = delete;
};

struct DoWhileStmt final : Stmt {
    std::unique_ptr<Stmt> body;
    std::unique_ptr<Expr> condition;
    std::string identifier;

    DoWhileStmt(std::unique_ptr<Stmt> body, std::unique_ptr<Expr> condition)
        : Stmt(Kind::DoWhile), body(std::move(body)), condition(std::move(condition)) {}

    DoWhileStmt(const i64 loc, std::unique_ptr<Stmt> body, std::unique_ptr<Expr> condition)
        : Stmt(loc, Kind::DoWhile), body(std::move(body)), condition(std::move(condition)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::DoWhile; }

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

    ForStmt(const i64 loc, std::unique_ptr<Stmt> body)
        : Stmt(loc, Kind::For), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::For; }

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

    SwitchStmt(const i64 loc, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(loc, Kind::Switch), condition(std::move(condition)), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Switch; }

    SwitchStmt() = delete;
};

struct NullStmt final : Stmt {
    NullStmt()
        : Stmt(Kind::Null) {}

    explicit NullStmt(const i64 loc)
        : Stmt(loc, Kind::Null) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Stmt* stmt) { return stmt->kind == Kind::Null; }
};

struct Program {
    std::vector<std::unique_ptr<Declaration>> declarations;
    ~Program() = default;
    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

struct SingleInitializer final : Initializer {
    std::unique_ptr<Expr> expr;

    explicit SingleInitializer(std::unique_ptr<Expr>&& exp)
        : Initializer(Kind::Single), expr(std::move(exp)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Initializer* initializer) { return initializer->kind == Kind::Single; }
};

struct CompoundInitializer final : Initializer {
    std::vector<std::unique_ptr<Initializer>> initializers;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    explicit CompoundInitializer(std::vector<std::unique_ptr<Initializer>>&& initializers)
        : Initializer(Kind::Compound), initializers(std::move(initializers)) {}

    static bool classOf(const Initializer* initializer) { return initializer->kind == Kind::Compound; }
};

struct ZeroInitializer final : Initializer {
    const i64 size;

    explicit ZeroInitializer(const i64 size)
        : Initializer(Kind::Zero), size(size) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Initializer* initializer) { return initializer->kind == Kind::Zero; }
};

struct StringInitializer final : Initializer {
    const std::string value;
    const bool nullTerminated;
    const i64 location;

    explicit StringInitializer(const std::string& value, const  bool nullTerminated, const i64 location)
        : Initializer(Kind::String), value(value), nullTerminated(nullTerminated), location(location) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const Initializer* initializer) { return initializer->kind == Kind::String; }
};

} // namespace Parsing