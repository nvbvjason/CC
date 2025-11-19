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

struct StructuredType final : TypeBase {
    std::string identifier;
    const i64 location;
    bool isComplete = false;

    explicit StructuredType(const Type type, std::string identifier, const i64 location)
        : TypeBase(type, Kind::Structured),
          identifier(std::move(identifier)),
          location(location) {}

    StructuredType(StructuredType&& structuredType) noexcept
        : TypeBase(structuredType.type, Kind::Structured),
          identifier(std::move(structuredType.identifier)),
          location(structuredType.location) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }

    static bool classOf(const TypeBase* typeBase) { return typeBase->kind == Kind::Structured; }
};
} // Parsing