#pragma once

#ifndef CC_PARSING_ABSTRACT_TREE_HPP
#define CC_PARSING_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <string>
#include <vector>

/*
    program = Program(function_definition)
    function_definition = Function(identifier names, block_item* body)
    block_item = S(statement) | D(declaration)
    declaration = Declaration(identifier name, exp? init)
    statement = Return(exp) | Expression(exp) | Null
    exp = Constant(int)
        | Var(identifier)
        | Unary(unary_operator, exp)
        | Binary(binary_operator, exp, exp)
        | Assignment(exp, exp)
    unary_operator = Complement | Negate | Not
    binary_operator = Add | Subtract | Multiply | Divide | Remainder |
                      BitwiseAnd | BitwiseOr | BitwiseXor |
                      LeftShift | RightShift |
                      And | Or |
                      Equal | NotEqual |
                      Less | LessThan | LessOrEqual
                      Greater | GreaterThan | GreaterOrEqual
*/

namespace Parsing {

struct Program;
struct Function;
struct BlockItem;
struct Declaration;
struct Stmt;
struct Expr;

struct Program {
    std::unique_ptr<Function> function = nullptr;
};

struct Function {
    std::string name;
    std::vector<std::unique_ptr<BlockItem>> body;
};

struct BlockItem {
    enum class Kind {
        Declaration, Statement
    };
    Kind kind;

    BlockItem() = delete;
    virtual ~BlockItem() = default;
protected:
    explicit BlockItem(const Kind kind)
        : kind(kind) {}
};

struct StmtBlockItem final : BlockItem {
    std::unique_ptr<Stmt> stmt;
    explicit StmtBlockItem(std::unique_ptr<Stmt> stmt)
        : BlockItem(Kind::Statement), stmt(std::move(stmt)) {}

    StmtBlockItem() = delete;
};

struct DeclarationBlockItem final : BlockItem {
    std::unique_ptr<Declaration> decl;
    explicit DeclarationBlockItem(std::unique_ptr<Declaration> decl)
        : BlockItem(Kind::Declaration), decl(std::move(decl)) {}

    DeclarationBlockItem() = delete;
};

struct Declaration {
    std::string name;
    std::unique_ptr<Expr> init = nullptr;
    explicit Declaration(std::string name)
        : name(std::move(name)) {}
    Declaration(std::string name, std::unique_ptr<Expr> init)
        : name(std::move(name)), init(std::move(init)) {}

    Declaration() = delete;
};

struct Stmt {
    enum class Kind {
        Return, Expression, Null
    };
    Kind kind;

    Stmt() = delete;
    virtual ~Stmt() = default;
protected:
    explicit Stmt(const Kind kind)
        : kind(kind) {}
};

struct ReturnStmt final : Stmt {
    std::unique_ptr<Expr> expression;
    explicit ReturnStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Return), expression(std::move(expr)) {}

    ReturnStmt() = delete;
};

struct ExprStmt final : Stmt {
    std::unique_ptr<Expr> expression;
    explicit ExprStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Expression), expression(std::move(expr)) {}

    ExprStmt() = delete;
};


struct NullStmt final : Stmt {
    NullStmt()
        : Stmt(Kind::Null) {}
};

struct Expr {
    enum class Kind {
        Constant, Var, Unary, Binary, Assignment
    };
    Kind kind;

    Expr() = delete;
    virtual ~Expr() = default;

protected:
    explicit Expr(const Kind kind)
        : kind(kind) {}
};

struct ConstExpr final : Expr {
    i32 value;
    explicit ConstExpr(const i32 value) noexcept
        : Expr(Kind::Constant), value(value) {}

    ConstExpr() = delete;
};

struct VarExpr final : Expr {
    std::string name;
    explicit VarExpr(std::string name) noexcept
        : Expr(Kind::Var), name(std::move(name)) {}

    VarExpr() = delete;
};

struct UnaryExpr final : Expr {
    enum class Operator {
        Complement, Negate, Not
    };
    Operator op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(const Operator op, std::unique_ptr<Expr> expr)
        : Expr(Kind::Unary), op(op), operand(std::move(expr)) {}

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
        GreaterThan, GreaterOrEqual,
        Assign
    };
    Operator op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    BinaryExpr(const Operator op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(Kind::Binary), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    BinaryExpr() = delete;
};

struct AssignmentExpr final : Expr {
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    AssignmentExpr(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(Kind::Assignment), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    AssignmentExpr() = delete;
};

} // Parsing

#endif // CC_PARSING_ABSTRACT_TREE_HPP