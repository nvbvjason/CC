#pragma once

#ifndef CC_PARSING_TYPES_TREE_HPP
#define CC_PARSING_TYPES_TREE_HPP

#include "ASTVisitor.hpp"
#include "ASTBase.hpp"

#include <memory>
#include <vector>

namespace Parsing {

struct VarType : Type {
    explicit VarType(const Kind kind)
        : Type(kind) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct FuncType : Type {
    std::vector<std::unique_ptr<Type>> params;
    std::unique_ptr<Type> returnType;
    explicit FuncType(std::unique_ptr<Type>&& rT, std::vector<std::unique_ptr<Type>>&& params)
        : Type(Kind::Function), returnType(std::move(rT)), params(std::move(params)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

} // Parsing
#endif // CC_PARSING_TYPES_TREE_HPP