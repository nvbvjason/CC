#pragma once

#ifndef CC_PARSING_ABSTRACT_TREE_HPP
#define CC_PARSING_ABSTRACT_TREE_HPP

#include "ShortTypes.hpp"
#include "ASTVisitor.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

/*
    program = Program(function_definition)
    function_definition = Function(identifier names, block body)
    block = Block(block_item)
    block_item = S(statement) | D(declaration)
    declaration = Declaration(identifier name, exp? init)
    for_init = InitDecl(declaration) | Init(Expr)
    statement = Return(exp)
              | Expression(exp)
              | If(exp condition, statement then, statement? else)
              | Compound(block)
              | Break(identifier label)
              | Continue(identifier label)
              | While(exp condition, statement body, identifier label)
              | DoWhile(statement body, exp condition, identifier label)
              | For(for_init init, exp? condition, exp? post, statement body, identifier label)
              | Null
    exp = Constant(int)
        | Var(identifier)
        | Unary(unary_operator, exp)
        | Binary(binary_operator, exp, exp)
        | Assignment(assign_operator, exp, exp)
        | Conditional(exp condition, exp, exp)
    unary_operator = Complement | Negate | Not
                   | PrefixIncrement | PostFixIncrement
                   | PrefixDecrement | PostFixDecrement
    binary_operator = Add | Subtract | Multiply | Divide | Remainder
                    | BitwiseAnd | BitwiseOr | BitwiseXor
                    | LeftShift | RightShift
                    | And | Or
                    | Equal | NotEqual
                    | Less | LessThan | LessOrEqual
                    | Greater | GreaterThan | GreaterOrEqual
    assign_operator = Assign | PlusAssign | MinusAssign
                    | MultiplyAssign | DivideAssign | ModuloAssign
                    | BitwiseAndAssign | BitwiseOrAssign | BitwiseXorAssign
                    | LeftShiftAssign | RightShiftAssign
*/

namespace Parsing {

struct Program {
    std::unique_ptr<Function> function = nullptr;

    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

struct Function {
    std::string name;
    std::unique_ptr<Block> body;

    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

struct Block {
    std::vector<std::unique_ptr<BlockItem>> body;

    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

struct BlockItem {
    enum class Kind {
        Declaration, Statement
    };
    Kind kind;

    BlockItem() = delete;
    virtual ~BlockItem() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;
protected:
    explicit BlockItem(const Kind kind)
        : kind(kind) {}
};

struct StmtBlockItem final : BlockItem {
    std::unique_ptr<Stmt> stmt;
    explicit StmtBlockItem(std::unique_ptr<Stmt> stmt)
        : BlockItem(Kind::Statement), stmt(std::move(stmt)) {}

    StmtBlockItem() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct DeclBlockItem final : BlockItem {
    std::unique_ptr<Declaration> decl;
    explicit DeclBlockItem(std::unique_ptr<Declaration> decl)
        : BlockItem(Kind::Declaration), decl(std::move(decl)) {}

    DeclBlockItem() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct Declaration {
    std::string name;
    std::unique_ptr<Expr> init = nullptr;
    explicit Declaration(std::string name)
        : name(std::move(name)) {}
    Declaration(std::string name, std::unique_ptr<Expr> init)
        : name(std::move(name)), init(std::move(init)) {}

    Declaration() = delete;

    void accept(ASTVisitor& visitor) { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const { visitor.visit(*this); }
};

struct ForInit {
    enum class Kind {
        Declaration, Expression
    };
    Kind kind;

    ForInit() = delete;
    virtual ~ForInit() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;
protected:
    explicit ForInit(const Kind kind)
        : kind(kind) {}
};

struct DeclForInit final : ForInit {
    std::unique_ptr<Declaration> decl;
    explicit DeclForInit(std::unique_ptr<Declaration> decl)
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

struct Stmt {
    enum class Kind {
        If, Return, Expression, Compound,
        Break, Continue, While, DoWhile, For,
        Null
    };
    Kind kind;

    Stmt() = delete;
    virtual ~Stmt() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;
protected:
    explicit Stmt(const Kind kind)
        : kind(kind) {}
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

struct ReturnStmt final : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ReturnStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Return), expr(std::move(expr)) {}

    ReturnStmt() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct ExprStmt final : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> expr)
        : Stmt(Kind::Expression), expr(std::move(expr)) {}

    ExprStmt() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
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

    explicit BreakStmt(std::string iden)
        : Stmt(Kind::Break), identifier(std::move(iden)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct ContinueStmt final : Stmt {
    std::string identifier;

    explicit ContinueStmt(std::string iden)
        : Stmt(Kind::Continue), identifier(std::move(iden)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct WhileStmt final : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    std::string identifier;
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body, std::string iden)
        : Stmt(Kind::While), condition(std::move(condition)), body(std::move(body)), identifier(std::move(iden)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    WhileStmt() = delete;
};

struct DoWhileStmt final : Stmt {
    std::unique_ptr<Stmt> body;
    std::unique_ptr<Expr> condition;
    std::string identifier;
    DoWhileStmt(std::unique_ptr<Stmt> body, std::unique_ptr<Expr> condition, std::string iden)
        : Stmt(Kind::DoWhile), body(std::move(body)), condition(std::move(condition)), identifier(std::move(iden)) {}

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
    ForStmt(std::unique_ptr<Stmt> body,std::string iden)
        : Stmt(Kind::For), body(std::move(body)), identifier(std::move(iden)){}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    ForStmt() = delete;
};

struct NullStmt final : Stmt {
    NullStmt()
        : Stmt(Kind::Null) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct Expr {
    enum class Kind {
        Constant, Var, Unary, Binary, Assignment, Conditional,
    };
    Kind kind;

    Expr() = delete;
    virtual ~Expr() = default;

    virtual void accept(ASTVisitor& visitor) = 0;
    virtual void accept(ConstASTVisitor& visitor) const = 0;
protected:
    explicit Expr(const Kind kind)
        : kind(kind) {}
};

struct ConstExpr final : Expr {
    i32 value;
    explicit ConstExpr(const i32 value) noexcept
        : Expr(Kind::Constant), value(value) {}

    ConstExpr() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct VarExpr final : Expr {
    std::string name;
    explicit VarExpr(std::string name) noexcept
        : Expr(Kind::Var), name(std::move(name)) {}

    VarExpr() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

};

struct UnaryExpr final : Expr {
    enum class Operator {
        Complement, Negate, Not,
        PrefixIncrement, PostFixIncrement,
        PrefixDecrement, PostFixDecrement
    };
    Operator op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(const Operator op, std::unique_ptr<Expr> expr)
        : Expr(Kind::Unary), op(op), operand(std::move(expr)) {}

    UnaryExpr() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
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

    BinaryExpr() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
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

    AssignmentExpr() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
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
    ConditionalExpr() = delete;

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

} // Parsing

#endif // CC_PARSING_ABSTRACT_TREE_HPP