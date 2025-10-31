#include "TypeCheckAndConvert.hpp"
#include "ASTDeepCopy.hpp"
#include "ASTParser.hpp"
#include "DynCast.hpp"
#include "ASTTypes.hpp"
#include "Declarator.hpp"

namespace Semantics {

void TypeCheckAndConvert::convert(Parsing::Program& program)
{
    ASTTraverser::visit(program);
}

void TypeCheckAndConvert::visit(Parsing::FunDeclaration& funDeclaration)
{
    auto funcType = dynCast<Parsing::FuncType>(funDeclaration.type.get());
    std::vector<std::unique_ptr<Parsing::TypeBase>> paramsTypes;
    for (auto & param : funcType->params) {
        if (param->type == Type::Array) {
            auto arrayType = dynCast<Parsing::ArrayType>(param.get());
            auto pointerType = std::make_unique<Parsing::PointerType>(Parsing::deepCopy(*arrayType->elementType));
            param = std::move(pointerType);
        }
    }
    if (funDeclaration.body)
        funDeclaration.body->accept(*this);
}

void TypeCheckAndConvert::visit(Parsing::ReturnStmt& stmt)
{
    stmt.expr = convertArrayType(*stmt.expr);
}

void TypeCheckAndConvert::visit(Parsing::ExprStmt& stmt)
{
    stmt.expr = convertArrayType(*stmt.expr);
}

void TypeCheckAndConvert::visit(Parsing::IfStmt& ifStmt)
{
    ifStmt.condition = convertArrayType(*ifStmt.condition);
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt)
        ifStmt.elseStmt->accept(*this);
}

void TypeCheckAndConvert::visit(Parsing::CaseStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

void TypeCheckAndConvert::visit(Parsing::WhileStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

void TypeCheckAndConvert::visit(Parsing::DoWhileStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

void TypeCheckAndConvert::visit(Parsing::ForStmt& stmt)
{
    if (stmt.init)
        stmt.init->accept(*this);
    if (stmt.condition)
        stmt.condition = convertArrayType(*stmt.condition);
    if (stmt.post)
        stmt.post = convertArrayType(*stmt.post);
    stmt.body->accept(*this);
}

void TypeCheckAndConvert::visit(Parsing::SwitchStmt& stmt)
{
    stmt.condition = convertArrayType(*stmt.condition);
    stmt.body->accept(*this);
}

std::unique_ptr<Parsing::Expr> convertArrayType(const Parsing::Expr& expr)
{
    if (expr.type && expr.type->type == Type::Array && expr.kind != Parsing::Expr::Kind::AddrOf) {
        if (expr.kind != Parsing::Expr::Kind::AddrOf) {
            auto arrayType = dynCast<Parsing::ArrayType>(expr.type.get());
            auto addressOf = std::make_unique<Parsing::AddrOffExpr>(Parsing::deepCopy(expr));
            addressOf->type = Parsing::deepCopy(*arrayType->elementType);
            return addressOf;
        }
    }
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::Expr& expr)
{
    using ExprKind = Parsing::Expr::Kind;
    switch (expr.kind) {
        case ExprKind::Constant: {
            const auto constExpr = dynCast<Parsing::ConstExpr>(&expr);
            return convert(*constExpr);
        }
        case ExprKind::Var: {
            const auto varExpr = dynCast<Parsing::VarExpr>(&expr);
            return convert(*varExpr);
        }
        case ExprKind::Cast: {
            const auto cast = dynCast<Parsing::CastExpr>(&expr);
            return convert(*cast);
        }
        case ExprKind::Unary: {
            const auto unary = dynCast<Parsing::UnaryExpr>(&expr);
            return convert(*unary);
        }
        case ExprKind::Binary: {
            const auto binary = dynCast<Parsing::BinaryExpr>(&expr);
            return convert(*binary);
        }
        case ExprKind::Assignment: {
            const auto assignment = dynCast<Parsing::AssignmentExpr>(&expr);
            return convert(*assignment);
        }
        case ExprKind::Ternary: {
            const auto ternary = dynCast<Parsing::TernaryExpr>(&expr);
            return convert(*ternary);
        }
        case ExprKind::FunctionCall: {
            const auto functionCall = dynCast<Parsing::FuncCallExpr>(&expr);
            return convert(*functionCall);
        }
        case ExprKind::Dereference: {
            const auto deref = dynCast<Parsing::DereferenceExpr>(&expr);
            return convert(*deref);
        }
        case ExprKind::AddrOf: {
            const auto addrOf = dynCast<Parsing::AddrOffExpr>(&expr);
            return convert(*addrOf);
        }
        case ExprKind::Subscript: {
            const auto subscript = dynCast<Parsing::SubscriptExpr>(&expr);
            return convert(*subscript);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<Parsing::Expr> convert(const Parsing::ConstExpr& expr)
{
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(const Parsing::VarExpr& expr)
{
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::CastExpr& expr)
{
    expr.expr = convertArrayType(*expr.expr);
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::UnaryExpr& expr)
{
    expr.operand = convertArrayType(*expr.operand);
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::BinaryExpr& expr)
{
    expr.lhs = convertArrayType(*expr.lhs);
    expr.rhs = convertArrayType(*expr.rhs);
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::AssignmentExpr& expr)
{
    expr.lhs = convertArrayType(*expr.lhs);
    expr.rhs = convertArrayType(*expr.rhs);
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::TernaryExpr& expr)
{
    expr.condition = convertArrayType(*expr.condition);
    expr.trueExpr = convertArrayType(*expr.trueExpr);
    expr.falseExpr = convertArrayType(*expr.falseExpr);
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::FuncCallExpr& expr)
{
    std::vector<std::unique_ptr<Parsing::Expr>> args;
    for (const auto& arg : expr.args)
        args.push_back(convertArrayType(*arg));
    expr.args = std::move(args);
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::DereferenceExpr& expr)
{
    expr.reference = convertArrayType(*expr.reference);
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::AddrOffExpr& expr)
{
    expr.reference = convertArrayType(*expr.reference);
    return Parsing::deepCopy(expr);
}

std::unique_ptr<Parsing::Expr> convert(Parsing::SubscriptExpr& expr)
{
    expr.referencing = convertArrayType(*expr.referencing);
    expr.index = convertArrayType(*expr.index);
    return Parsing::deepCopy(expr);
}

} // Semantics