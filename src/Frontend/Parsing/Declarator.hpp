#pragma once

#ifndef CC_PARSING_DECLARATOR_HPP
#define CC_PARSING_DECLARATOR_HPP

#include "ASTBase.hpp"
#include "CodeGen/AsmAST.hpp"

namespace Parsing {

struct Declarator {
    enum class Kind {
        Identifier, Pointer, Function
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

    IdentifierDeclarator() = delete;
};

struct PointerDeclarator : Declarator {
    std::unique_ptr<Declarator> inner;

    explicit PointerDeclarator(std::unique_ptr<Declarator>&& declarator)
        : Declarator(Kind::Pointer), inner(std::move(declarator)) {}

    PointerDeclarator() = delete;
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

    FunctionDeclarator() = delete;
};

struct AbstractDeclarator {
    enum class Kind {
        Pointer, Base
    };
    Kind kind;

protected:
    explicit AbstractDeclarator(const Kind kind)
        : kind(kind) {}
};

struct AbstractPointer : AbstractDeclarator {
    std::unique_ptr<AbstractDeclarator> inner;
    explicit AbstractPointer(std::unique_ptr<AbstractDeclarator>&& i)
        : AbstractDeclarator(Kind::Pointer), inner(std::move(i)) {}
};

struct AbstractBase : AbstractDeclarator {
    explicit AbstractBase()
        : AbstractDeclarator(Kind::Base) {}
};

} // Parsing

#endif // CC_PARSING_DECLARATOR_HPP