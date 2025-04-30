#include "AstPrinter.hpp"

namespace Parsing {

namespace {

std::string unaryOpToString(const UnaryExpr::Operator op)
{
    switch (op) {
        case UnaryExpr::Operator::Complement: return "~";
        case UnaryExpr::Operator::Negate: return "-";
        case UnaryExpr::Operator::Not: return "!";
        default: return "?";
    }
}

std::string binaryOpToString(const BinaryExpr::Operator op)
{
    switch (op) {
        case BinaryExpr::Operator::Add:             return "+";
        case BinaryExpr::Operator::Subtract:        return "-";
        case BinaryExpr::Operator::Multiply:        return "*";
        case BinaryExpr::Operator::Divide:          return "/";
        case BinaryExpr::Operator::Remainder:       return "%";
        case BinaryExpr::Operator::BitwiseAnd:      return "&";
        case BinaryExpr::Operator::BitwiseOr:       return "|";
        case BinaryExpr::Operator::BitwiseXor:      return "^";
        case BinaryExpr::Operator::LeftShift:       return "<<";
        case BinaryExpr::Operator::RightShift:      return ">>";
        case BinaryExpr::Operator::And:             return "&&";
        case BinaryExpr::Operator::Or:              return "||";
        case BinaryExpr::Operator::Equal:           return "==";
        case BinaryExpr::Operator::NotEqual:        return "!=";
        case BinaryExpr::Operator::LessThan:        return "<";
        case BinaryExpr::Operator::LessOrEqual:     return "<=";
        case BinaryExpr::Operator::GreaterThan  :   return ">";
        case BinaryExpr::Operator::GreaterOrEqual:  return ">=";
        default:                                    return "?";
    }
}
}

std::string ASTPrinter::print(const Program* program)
{
    oss << "Program:\n";
    if (program->function)
        print(*program->function, 1);
    else
        oss << "  (empty)\n";
    return oss.str();
}

void ASTPrinter::print(const Function& function, const int indent)
{
    std::string indentStr(indent * 2, ' ');
    oss << indentStr << "Function: " << function.name << "\n";
    if (function.body)
        print(*function.body, indent + 1);
    else
        oss << indentStr << "  (empty body)\n";
}

void ASTPrinter::print(const Statement& statement, const int indent)
{
    std::string indentStr(indent * 2, ' ');
    oss << indentStr << "Statement:\n";
    if (statement.expression)
        print(*statement.expression, indent + 1);
    else
        oss << indentStr << "  (empty expression)\n";
}

void ASTPrinter::print(Expr& expr, const int indent)
{
    const std::string indentStr(indent * 2, ' ');

    if (expr.kind == Expr::Kind::Constant) {
        const auto constant = dynamic_cast<ConstantExpr*>(&expr);
        oss << indentStr << "Constant: " << constant->value << "\n";
    }
    else if (expr.kind == Expr::Kind::Unary) {
        const auto unary = dynamic_cast<UnaryExpr*>(&expr);
        oss << indentStr << "auto binary = static_cast<BinaryExpr*>(&expr);: " << unaryOpToString(unary->op) << "\n";
        print(*unary->operand, indent + 1);
    }
    else if (expr.kind == Expr::Kind::Binary) {
        const auto binary = dynamic_cast<BinaryExpr*>(&expr);
        oss << indentStr << "Binary: " << binaryOpToString(binary->op) << "\n";
        print(*binary->lhs, indent + 1);
        print(*binary->rhs, indent + 1);
    }
}
} // nnamespace Parsing