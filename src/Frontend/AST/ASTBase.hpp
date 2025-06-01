#pragma once

#ifndef CC_PARSING_AST_BASE_HPP
#define CC_PARSING_AST_BASE_HPP

#include "ASTVisitor.hpp"
#include "ShortTypes.hpp"

#include <memory>

namespace Parsing {

struct Type {
    enum class Kind {
        Int, Long, Function
    };
    Kind kind;
    virtual ~Type() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;
    Type() = delete;
protected:
    explicit Type(const Kind kind)
        : kind(kind) {}
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

struct Expr {
    enum class Kind {
        Constant, Var, Cast, Unary, Binary, Assignment, Conditional, FunctionCall,
    };
    Kind kind;
    std::unique_ptr<Type> type = nullptr;

    virtual ~Expr() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    Expr() = delete;
protected:
    Expr(const Kind kind, std::unique_ptr<Type>&& type)
        : kind(kind), type(std::move(type)) {}
    explicit Expr(const Kind kind)
        : kind(kind) {}
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

struct Declaration {
    enum class Kind : u8 {
        VarDecl, FuncDecl
    };
    enum class StorageClass : u8 {
        None,
        Extern,
        Static
    };
    Kind kind;
    StorageClass storage = StorageClass::None;

    virtual ~Declaration() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    Declaration() = delete;
protected:
    explicit Declaration(const Kind kind, const StorageClass storageClass)
        : kind(kind), storage(storageClass) {}
};

} // Parsing

#endif // CC_PARSING_AST_BASE_HPP
