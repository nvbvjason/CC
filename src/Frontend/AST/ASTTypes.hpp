#pragma once

#ifndef CC_PARSING_TYPES_TREE_HPP
#define CC_PARSING_TYPES_TREE_HPP

#include "ASTVisitor.hpp"
#include "ASTBase.hpp"

#include <memory>
#include <vector>

namespace Parsing {

struct VarType : TypeBase {
    explicit VarType(const Type kind)
        : TypeBase(kind) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

struct FuncType : TypeBase {
    std::vector<std::unique_ptr<TypeBase>> params;
    std::unique_ptr<TypeBase> returnType;
    explicit FuncType(std::unique_ptr<TypeBase>&& rT, std::vector<std::unique_ptr<TypeBase>>&& params)
        : TypeBase(Type::Function), returnType(std::move(rT)), params(std::move(params)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
};

} // Parsing
#endif // CC_PARSING_TYPES_TREE_HPP