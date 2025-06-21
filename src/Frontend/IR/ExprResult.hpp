#pragma once

#ifndef CC_IR_EXPR_RESULT_HPP
#define CC_IR_EXPR_RESULT_HPP

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

    explicit PlainOperand(std::shared_ptr<Value>&& value)
        : ExprResult(Kind::PlainOperand), value(std::move(value)) {}

    PlainOperand() = delete;
};

struct DereferencedPointer : ExprResult {
    std::shared_ptr<Value> value;
    explicit DereferencedPointer(std::shared_ptr<Value>&& value)
        : ExprResult(Kind::DereferencedPointer), value(std::move(value)) {}

    DereferencedPointer() = delete;
};

} // Ir

#endif // CC_IR_EXPR_RESULT_HPP