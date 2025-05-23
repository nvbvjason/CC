#pragma once

#ifndef CC_PARSING_ABSTRACT_TREE_HPP
#define CC_PARSING_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"
#include "ASTVisitor.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Parsing {

struct Program {
    std::vector<std::unique_ptr<Declaration>> declarations;

    ~Program();

    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

struct Declaration {
    enum class Kind : u8 {
        VarDecl, FuncDecl
    };
    enum class StorageClass : u8 {
        AutoLocalScope,             // no specifier at local scope (e.g., int x; in a function)
        GlobalScopeDeclaration,     // no specifier at global scope (e.g., int a; at file scope)
        StaticGlobalTentative,      // global to file (e.g., static int x;)
        StaticGlobalInitialized,    // global to file (e.g., static int x = 5;)
        StaticLocal,                // local scope persistent (e.g., static int x; in a function)
        StaticLocalFunction,        // ILLEGAL
        ExternFunction,             // normal function rules (e.g., extern void foo();)
        ExternLocal,                // block scope shadowing (e.g., extern int a; in a block)
        ExternGlobal,               // without definition (e.g., extern int a;)
        ExternGlobalInitialized,    // useless keyword WARNING (e.g., extern int a = 5;)
        GlobalDefinition,           // global at file scope int a = 5;
    };
    Kind kind;
    StorageClass storage;

    virtual ~Declaration() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    Declaration() = delete;
protected:
    explicit Declaration(const Kind kind, const StorageClass storageClass)
        : kind(kind), storage(storageClass) {}
};

struct VarDecl final : Declaration {
    std::string name;
    std::unique_ptr<Expr> init = nullptr;
    explicit  VarDecl(const StorageClass storageClass, std::string name)
        : Declaration(Kind::VarDecl, storageClass), name(std::move(name)) {}

    ~VarDecl() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    VarDecl() = delete;
};

struct FunDecl final : Declaration {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<Block> body = nullptr;

    explicit FunDecl(const StorageClass storageClass, std::string name)
        : Declaration(Kind::FuncDecl, storageClass), name(std::move(name)) {}

    ~FunDecl() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    FunDecl() = delete;
};

struct Block {
    std::vector<std::unique_ptr<BlockItem>> body;

    ~Block();

    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

struct BlockItem {
    enum class Kind : u8 {
        Declaration, Statement
    };
    Kind kind;

    virtual ~BlockItem() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    BlockItem() = delete;
protected:
    explicit BlockItem(const Kind kind)
        : kind(kind) {}
};

struct StmtBlockItem final : BlockItem {
    std::unique_ptr<Stmt> stmt;
    explicit StmtBlockItem(std::unique_ptr<Stmt> stmt)
        : BlockItem(Kind::Statement), stmt(std::move(stmt)) {}

    ~StmtBlockItem() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    StmtBlockItem() = delete;
};

struct DeclBlockItem final : BlockItem {
    std::unique_ptr<Declaration> decl;
    explicit DeclBlockItem(std::unique_ptr<Declaration> decl)
        : BlockItem(Kind::Declaration), decl(std::move(decl)) {}

    ~DeclBlockItem() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    DeclBlockItem() = delete;
};

struct ForInit {
    enum class Kind {
        Declaration, Expression
    };
    Kind kind;

    virtual ~ForInit() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    ForInit() = delete;
protected:
    explicit ForInit(const Kind kind)
        : kind(kind) {}
};

struct DeclForInit final : ForInit {
    std::unique_ptr<VarDecl> decl;
    explicit DeclForInit(std::unique_ptr<VarDecl> decl)
        : ForInit(Kind::Declaration), decl(std::move(decl)) {}

    ~DeclForInit() override;

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

    ~ExprForInit() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct Stmt {
    enum class Kind {
        Return, Expression, If, Goto, Compound,
        Break, Continue, Label, Case, Default, While, DoWhile, For, Switch,
        Null
    };
    Kind kind;

    virtual ~Stmt() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    Stmt() = delete;
protected:
    explicit Stmt(const Kind kind)
        : kind(kind) {}
};

struct ReturnStmt final : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ReturnStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Return), expr(std::move(expr)) {}

    ~ReturnStmt() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    ReturnStmt() = delete;
};

struct ExprStmt final : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Expression), expr(std::move(expr)) {}

    ~ExprStmt() override;

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

    ~IfStmt() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    IfStmt() = delete;
};

struct GotoStmt final : Stmt {
    std::string identifier;
    explicit GotoStmt(std::string iden)
        : Stmt(Kind::Goto), identifier(std::move(iden)) {}

    ~GotoStmt() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    GotoStmt() = delete;
};

struct CompoundStmt final : Stmt {
    std::unique_ptr<Block> block;
    explicit CompoundStmt(std::unique_ptr<Block> block)
        : Stmt(Kind::Compound), block(std::move(block)) {}

    ~CompoundStmt() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    CompoundStmt() = delete;
};

struct BreakStmt final : Stmt {
    std::string identifier;

    explicit BreakStmt()
        : Stmt(Kind::Break) {}

    ~BreakStmt() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct ContinueStmt final : Stmt {
    std::string identifier;

    explicit ContinueStmt()
        : Stmt(Kind::Continue) {}

    ~ContinueStmt() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct LabelStmt final : Stmt {
    std::string identifier;
    std::unique_ptr<Stmt> stmt;
    explicit LabelStmt(std::string name, std::unique_ptr<Stmt> stmt)
        : Stmt(Kind::Label), identifier(std::move(name)), stmt(std::move(stmt)) {}

    ~LabelStmt() override;

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

    ~CaseStmt() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    CaseStmt() = delete;
};

struct DefaultStmt final : Stmt {
    std::string identifier;
    std::unique_ptr<Stmt> body;
    explicit DefaultStmt(std::unique_ptr<Stmt> body)
        : Stmt(Kind::Default), body(std::move(body)) {}

    ~DefaultStmt() override;

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

    ~WhileStmt() override;

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

    ~DoWhileStmt() override;

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

    ~ForStmt() override;

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
    std::vector<std::unique_ptr<ConstExpr>> cases;
    bool hasDefault = false;

    ~SwitchStmt() override;

    SwitchStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : Stmt(Kind::Switch), condition(std::move(condition)), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    SwitchStmt() = delete;
};

struct NullStmt final : Stmt {
    NullStmt()
        : Stmt(Kind::Null) {}

    ~NullStmt() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct Expr {
    enum class Kind {
        Constant, Var, Unary, Binary, Assignment, Conditional,
        FunctionCall,
    };
    Kind kind;

    virtual ~Expr() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    Expr() = delete;
protected:
    explicit Expr(const Kind kind)
        : kind(kind) {}
};

struct ConstExpr final : Expr {
    i32 value;
    explicit ConstExpr(const i32 value) noexcept
        : Expr(Kind::Constant), value(value) {}

    ~ConstExpr() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    ConstExpr() = delete;
};

struct VarExpr final : Expr {
    std::string name;
    explicit VarExpr(std::string name) noexcept
        : Expr(Kind::Var), name(std::move(name)) {}

    ~VarExpr() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    VarExpr() = delete;
};

struct UnaryExpr final : Expr {
    enum class Operator {
        Complement, Negate, Not, Plus,
        PrefixIncrement, PostFixIncrement,
        PrefixDecrement, PostFixDecrement
    };
    Operator op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(const Operator op, std::unique_ptr<Expr> expr)
        : Expr(Kind::Unary), op(op), operand(std::move(expr)) {}

    ~UnaryExpr() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    UnaryExpr() = delete;
};

struct BinaryExpr final : Expr {
    enum class Operator {
        Add, Subtract, Multiply, Divide, Remainder,
        BitwiseAnd, BitwiseOr, BitwiseXor,
        LeftShift, RightShift,
        And, Or,
        Equal, NotEqual,
        LessThan, LessOrEqual,
        GreaterThan, GreaterOrEqual
    };
    Operator op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    BinaryExpr(const Operator op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(Kind::Binary), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    ~BinaryExpr() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    BinaryExpr() = delete;
};

struct AssignmentExpr final : Expr {
    enum class Operator {
        Assign, PlusAssign, MinusAssign,
        MultiplyAssign, DivideAssign, ModuloAssign,
        BitwiseAndAssign, BitwiseOrAssign, BitwiseXorAssign,
        LeftShiftAssign, RightShiftAssign
    };
    Operator op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    AssignmentExpr(const Operator op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(Kind::Assignment), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    ~AssignmentExpr() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    AssignmentExpr() = delete;
};

struct ConditionalExpr final : Expr {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> first;
    std::unique_ptr<Expr> second;
    ConditionalExpr(std::unique_ptr<Expr> condition,
                    std::unique_ptr<Expr> first,
                    std::unique_ptr<Expr> second)
        : Expr(Kind::Conditional), condition(std::move(condition)),
                                   first(std::move(first)),
                                   second(std::move(second)) {}

    ~ConditionalExpr() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    ConditionalExpr() = delete;
};

struct FunCallExpr final : Expr {
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;
    FunCallExpr(std::string identifier, std::vector<std::unique_ptr<Expr>> args)
        : Expr(Kind::FunctionCall), name(std::move(identifier)), args(std::move(args)) {}

    ~FunCallExpr() override;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    FunCallExpr() = delete;
};

} // Parsing

#endif // CC_PARSING_ABSTRACT_TREE_HPP