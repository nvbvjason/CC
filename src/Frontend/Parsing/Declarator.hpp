#pragma once

#include "ASTBase.hpp"
#include "CodeGen/AsmAST.hpp"

namespace Parsing {

struct AbstractDeclarator {
    enum class Kind {
        Pointer, Base, Array
    };
    Kind kind;

    AbstractDeclarator() = delete;

protected:
    explicit AbstractDeclarator(const Kind kind)
        : kind(kind) {}
};

struct AbstractPointer : AbstractDeclarator {
    std::unique_ptr<AbstractDeclarator> inner;

    explicit AbstractPointer(std::unique_ptr<AbstractDeclarator>&& i)
        : AbstractDeclarator(Kind::Pointer), inner(std::move(i)) {}

    static bool classOf(const AbstractDeclarator* abstractDeclarator) { return abstractDeclarator->kind == Kind::Pointer; }

    AbstractPointer() = delete;
};

struct AbstractArrayDeclarator : AbstractDeclarator {
    std::unique_ptr<AbstractDeclarator> abstractDeclarator;
    std::unique_ptr<Expr> size;

    explicit AbstractArrayDeclarator(std::unique_ptr<AbstractDeclarator>&& abstractDeclarator, std::unique_ptr<Expr>&& size)
        : AbstractDeclarator(Kind::Array), abstractDeclarator(std::move(abstractDeclarator)), size(std::move(size)) {}

    static bool classOf(const AbstractDeclarator* declarator) { return declarator->kind == Kind::Array; }

    AbstractArrayDeclarator() = delete;
};

struct AbstractBase : AbstractDeclarator {
    explicit AbstractBase()
        : AbstractDeclarator(Kind::Base) {}

    static bool classOf(const AbstractDeclarator* declarator) { return declarator->kind == Kind::Base; }
};

struct Declarator {
    enum class Kind {
        Identifier, Pointer, Function, Array
    };
    Kind kind;

    Declarator() = delete;
protected:
    explicit Declarator(const Kind kind)
        : kind(kind) {}
};

struct IdentifierDeclarator : Declarator {
    std::string identifier;
    explicit IdentifierDeclarator(std::string&& identifier)
        : Declarator(Kind::Identifier), identifier(std::move(identifier)) {}

    static bool classOf(const Declarator* declarator) { return declarator->kind == Kind::Identifier; }

    IdentifierDeclarator() = delete;
};

struct PointerDeclarator : Declarator {
    std::unique_ptr<Declarator> inner;

    explicit PointerDeclarator(std::unique_ptr<Declarator>&& declarator)
        : Declarator(Kind::Pointer), inner(std::move(declarator)) {}

    static bool classOf(const Declarator* declarator) { return declarator->kind == Kind::Pointer; }

    PointerDeclarator() = delete;
};

struct ArrayDeclarator : Declarator {
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<Expr> size;

    explicit ArrayDeclarator(std::unique_ptr<Declarator>&& declarator, std::unique_ptr<Expr>&& size)
        : Declarator(Kind::Array), declarator(std::move(declarator)), size(std::move(size)) {}

    static bool classOf(const Declarator* declarator) { return declarator->kind == Kind::Array; }

    ArrayDeclarator() = delete;
};

struct ParamInfo {
    std::unique_ptr<TypeBase> type;
    std::unique_ptr<Declarator> declarator;

    ParamInfo(std::unique_ptr<TypeBase>&& type, std::unique_ptr<Declarator>&& declarator)
        : type(std::move(type)), declarator(std::move(declarator)) {}

    ParamInfo() = delete;
};

struct FunctionDeclarator : Declarator {
    std::vector<ParamInfo> params;
    std::unique_ptr<Declarator> declarator;

    explicit FunctionDeclarator(std::unique_ptr<Declarator>&& declarator, std::vector<ParamInfo>&& params)
        : Declarator(Kind::Function), params(std::move(params)), declarator(std::move(declarator)) {}

    static bool classOf(const Declarator* declarator) { return declarator->kind == Kind::Function; }

    FunctionDeclarator() = delete;
};

} // Parsing