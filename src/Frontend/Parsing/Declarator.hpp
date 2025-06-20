#pragma once

#ifndef DECLARATOR_HPP
#define DECLARATOR_HPP
#include "ASTBase.hpp"
#include "CodeGen/AsmAST.hpp"

namespace Parsing {

struct Declarator {
    enum class Kind {
        Identifier, Pointer, Function
    };
    Kind kind;

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
    std::unique_ptr<Declarator> declarator;

    explicit PointerDeclarator(std::unique_ptr<Declarator>&& declarator)
        : Declarator(Kind::Pointer), declarator(std::move(declarator)) {}

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
        : Declarator(Kind::Function), declarator(std::move(declarator)), params(std::move(params)) {}

    FunctionDeclarator() = delete;
};

} // Parsing

#endif //DECLARATOR_HPP