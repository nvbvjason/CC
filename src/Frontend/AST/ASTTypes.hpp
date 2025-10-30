#pragma once

#include "ASTVisitor.hpp"
#include "ASTBase.hpp"

#include <memory>
#include <vector>

namespace Parsing {

struct VarType : TypeBase {
    explicit VarType(const Type type)
        : TypeBase(type, Kind::Var) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Var; }
};

struct FuncType : TypeBase {
    std::vector<std::unique_ptr<TypeBase>> params;
    std::unique_ptr<TypeBase> returnType;
    explicit FuncType(std::unique_ptr<TypeBase>&& rT, std::vector<std::unique_ptr<TypeBase>>&& params)
        : TypeBase(Type::Function, Kind::Func), params(std::move(params)), returnType(std::move(rT)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Func; }
};

struct PointerType : TypeBase {
    std::unique_ptr<TypeBase> referenced;

    explicit PointerType(std::unique_ptr<TypeBase>&& r)
        : TypeBase(Type::Pointer, Kind::Pointer), referenced(std::move(r)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Pointer; }
};

struct ArrayType : TypeBase {
    std::unique_ptr<TypeBase> elementType;
    std::unique_ptr<Expr> size;

    explicit ArrayType(std::unique_ptr<TypeBase>&& rT, std::unique_ptr<Expr>&& size)
        : TypeBase(Type::Array, Kind::Array), elementType(std::move(rT)), size(std::move(size)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Array; }
};

[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const TypeBase& typeBase);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const VarType& typeBase);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const FuncType& funcType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const PointerType& pointerType);

[[nodiscard]] bool areEquivalent(const TypeBase& left, const TypeBase& right);
[[nodiscard]] bool areEquivalent(const VarType& left, const VarType& right);
[[nodiscard]] bool areEquivalent(const FuncType& left, const FuncType& right);
[[nodiscard]] bool areEquivalent(const PointerType& left, const PointerType& right);

} // Parsing