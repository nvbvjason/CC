#pragma once

#include "ASTVisitor.hpp"
#include "ShortTypes.hpp"
#include "Types/Type.hpp"

#include <memory>

namespace Parsing {

struct ASTNode {
    const i64 location = 0;
    explicit ASTNode(const i64 location)
        : location(location) {}
};

struct TypeBase {
    enum class Kind {
        Var, Func, Pointer, Array
    };
    Kind kind;
    Type type;

    virtual ~TypeBase() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    TypeBase() = delete;
protected:
    explicit TypeBase(const Type type, const Kind kind)
        : kind(kind), type(type) {}
};

struct BlockItem : ASTNode {
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
        : ASTNode(0l), kind(kind) {}
    BlockItem(const i64 loc, const Kind kind)
        : ASTNode(loc), kind(kind) {}
};

struct ForInit : ASTNode {
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
        : ASTNode(0l), kind(kind) {}
    ForInit(const i64 loc, const Kind kind)
        : ASTNode(loc), kind(kind) {}
};

struct Expr : ASTNode {
    enum class Kind {
        Constant, Var, Cast, Unary, Binary, Assignment, Ternary, FunctionCall,
        Dereference, AddrOf, Subscript
    };
    Kind kind;
    std::unique_ptr<TypeBase> type = nullptr;

    virtual ~Expr() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;

    Expr() = delete;
protected:
    Expr(const i64 location, const Kind kind, std::unique_ptr<TypeBase>&& type)
        : ASTNode(location), kind(kind), type(std::move(type)) {}
    Expr(const Kind kind, std::unique_ptr<TypeBase>&& type)
        : ASTNode(0l), kind(kind), type(std::move(type)) {}
    Expr(const i64 location, const Kind kind)
        : ASTNode(location), kind(kind) {}
    explicit Expr(const Kind kind)
        : ASTNode(0l), kind(kind) {}
};

struct Stmt : ASTNode {
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
        : ASTNode(0l), kind(kind) {}
    Stmt(const i64 loc, const Kind kind)
        : ASTNode(loc), kind(kind) {}
};

struct Declaration : ASTNode {
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
    Declaration(const Kind kind, const StorageClass storageClass)
        : ASTNode(0l), kind(kind), storage(storageClass) {}
    Declaration(const i64 loc, const Kind kind, const StorageClass storageClass)
        : ASTNode(loc), kind(kind), storage(storageClass) {}
};

} // Parsing