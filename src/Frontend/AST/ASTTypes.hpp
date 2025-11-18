#pragma once

#include "ASTVisitor.hpp"
#include "ASTBase.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace Parsing {

struct VarType final : TypeBase {
    explicit VarType(const Type type)
        : TypeBase(type, Kind::Var) {}

    explicit VarType(VarType&& varType) noexcept
        : TypeBase(varType.type, Kind::Var) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Var; }
};

struct FuncType final : TypeBase {
    std::vector<std::unique_ptr<TypeBase>> params;
    std::unique_ptr<TypeBase> returnType;

    explicit FuncType(std::unique_ptr<TypeBase>&& rT, std::vector<std::unique_ptr<TypeBase>>&& params)
        : TypeBase(Type::Function, Kind::Func), params(std::move(params)), returnType(std::move(rT)) {}

    explicit FuncType(FuncType&& funcType) noexcept
        : TypeBase(Type::Function, Kind::Func),
          params(std::move(funcType.params)), returnType(std::move(funcType.returnType)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Func; }
};

struct PointerType final : TypeBase {
    std::unique_ptr<TypeBase> referenced;

    explicit PointerType(std::unique_ptr<TypeBase>&& r)
        : TypeBase(Type::Pointer, Kind::Pointer), referenced(std::move(r)) {}

    explicit PointerType(PointerType&& pointerType) noexcept
        : TypeBase(Type::Pointer, Kind::Pointer),
          referenced(std::move(pointerType.referenced)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Pointer; }
};

struct ArrayType final : TypeBase {
    std::unique_ptr<TypeBase> elementType;
    const i64 size;

    explicit ArrayType(std::unique_ptr<TypeBase>&& elementType, const i64 size)
        : TypeBase(Type::Array, Kind::Array), elementType(std::move(elementType)), size(size) {}

    explicit ArrayType(ArrayType&& arrayType) noexcept
        : TypeBase(Type::Array, Kind::Array),
          elementType(std::move(arrayType.elementType)),
          size(arrayType.size) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Array; }
};

struct StructType final : TypeBase {
    const std::string identifier;

    explicit StructType(std::string identifier)
        : TypeBase(Type::Struct, Kind::Struct), identifier(std::move(identifier)) {}

    explicit StructType(StructType&& structType) noexcept
        : TypeBase(Type::Struct, Kind::Struct), identifier(std::move(structType.identifier)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Struct; }
};

struct UnionType final : TypeBase {
    const std::string identifier;

    explicit UnionType(std::string identifier)
        : TypeBase(Type::Union, Kind::Union), identifier(std::move(identifier)) {}

    explicit UnionType(UnionType&& unionType) noexcept
        : TypeBase(Type::Union, Kind::Union), identifier(std::move(unionType.identifier)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Union; }
};
} // Parsing