#pragma once

#ifndef CC_PARSING_INITIALIZER_HPP
#define CC_PARSING_INITIALIZER_HPP

#include <vector>

#include "ASTBase.hpp"

namespace Parsing {

struct SingleInit : Initializer {
    std::unique_ptr<Expr> exp;

    explicit SingleInit(std::unique_ptr<Expr>&& exp)
        : Initializer(Kind::Single), exp(std::move(exp)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    SingleInit() = delete;
};

struct CompoundInit : Initializer {
    std::vector<std::unique_ptr<Initializer>> exp;
    explicit CompoundInit(std::vector<std::unique_ptr<Initializer>>&& exp)
        : Initializer(Kind::Compound), exp(std::move(exp)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    CompoundInit() = delete;
};

} // Parsing
#endif // CC_PARSING_INITIALIZER_HPP