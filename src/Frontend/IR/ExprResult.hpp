#pragma once

#include <memory>

#include "ASTIr.hpp"

namespace Ir {

struct ExprResult {
    enum class Kind {
        PlainOperand, DereferencedPointer,
    };
    Kind kind;

    ExprResult() = delete;
protected:
    explicit ExprResult(const Kind kind)
        : kind(kind) {}
};

struct PlainOperand : ExprResult {
    std::shared_ptr<Value> value;

    explicit PlainOperand(std::shared_ptr<Value> value)
        : ExprResult(Kind::PlainOperand), value(std::move(value)) {}

    PlainOperand() = delete;
};

struct DereferencedPointer : ExprResult {
    std::shared_ptr<Value> ptr;
    Type referredToType;
    DereferencedPointer(std::shared_ptr<Value> p, const Type rt)
        : ExprResult(Kind::DereferencedPointer), ptr(std::move(p)), referredToType(rt) {}

    DereferencedPointer() = delete;
};

} // Ir