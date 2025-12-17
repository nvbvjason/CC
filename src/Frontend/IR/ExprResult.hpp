#pragma once

#include "ASTIr.hpp"

#include <utility>

namespace Ir {

    struct ExprResult {
        enum class Kind : u8 {
            PlainOperand, DereferencedPointer, SubObject
        };
        const Kind kind;

        ExprResult() = delete;
    protected:
        explicit ExprResult(const Kind kind)
            : kind(kind) {}
    };

    struct PlainOperand : ExprResult {
        const Value* value;

        explicit PlainOperand(const Value* value)
            : ExprResult(Kind::PlainOperand), value(std::move(value)) {}

        static bool classOf(const ExprResult* expr) { return expr->kind == Kind::PlainOperand; }

        PlainOperand() = delete;
    };

    struct DereferencedPointer : ExprResult {
        const Value* ptr;
        const Type referredToType;
        DereferencedPointer(const Value* p, const Type rt)
            : ExprResult(Kind::DereferencedPointer), ptr(p), referredToType(rt) {}

        static bool classOf(const ExprResult* expr) { return expr->kind == Kind::DereferencedPointer; }

        DereferencedPointer() = delete;
    };

    struct SubObject : ExprResult {
        const Identifier base;
        const ReferringTo referringTo;
        const i64 offset;

        SubObject(Identifier base, const ReferringTo referringTo, const i64 offset)
            : ExprResult(Kind::SubObject), base(std::move(base)), referringTo(referringTo), offset(offset) {}

        static bool classOf(const ExprResult* expr) { return expr->kind == Kind::SubObject; }

        SubObject() = delete;
    };

} // Ir