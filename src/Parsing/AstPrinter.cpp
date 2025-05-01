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
        case BinaryExpr::Operator::Assign:          return "=";
        default:                                    return "?";
    }
}
}

std::string ASTPrinter::print(const Program* program)
{
    oss.clear();
    oss << "Program:\n";
    if (program->function)
        print(program->function.get(), 1);
    else
        oss << "  (empty)\n";
    return oss.str();
}

void ASTPrinter::print(const Function* function, const int indent)
{
    std::string indentStr(indent * 2, ' ');
    oss << indentStr << "Function: " << function->name << "\n";
    if (function->body.empty())
        oss << indentStr << "  (empty body)\n";
    for (const auto& blockItem : function->body)
        print(blockItem.get(), indent + 1);
}

void ASTPrinter::print(const BlockItem* blockItem, const int indent)
{;
    switch (blockItem->kind) {
        case BlockItem::Kind::Declaration:
            print(dynamic_cast<const Declaration*>(blockItem), indent);
            break;
        case BlockItem::Kind::Statement:
            print(dynamic_cast<const Statement*>(blockItem), indent);
            break;
        default:
            break;
    }
}

void ASTPrinter::print(const Declaration* declaration, const int indent)
{
    std::string indentStr(indent * 2, ' ');
    oss << indentStr << "Declaration: " << declaration->name << "\n";
    if (declaration->init)
        print(declaration->init.get(), indent + 1);
}

void ASTPrinter::print(const Statement* statement, const int indent)
{
    std::string indentStr(indent * 2, ' ');
    switch (statement->kind) {
        case Statement::Kind::Null:
            oss << indentStr << "Null Statement\n";
            break;
        case Statement::Kind::Return:
            oss << indentStr << "Return Statement\n";
            print(statement->expression.get(), indent + 1);
            break;
        case Statement::Kind::Expression:
            oss << indentStr << "Expression Statement\n";
            print(statement->expression.get(), indent + 1);
            break;
    }
}

void ASTPrinter::print(const Expr* expr, const int indent)
{
    const std::string indentStr(indent * 2, ' ');
    oss << indentStr << "Expression: " << "\n";
    switch (expr->kind) {
        case Expr::Kind::Constant: {
            const auto constantExpr = dynamic_cast<const ConstantExpr*>(expr);
            oss << indentStr << "Constant: " << constantExpr->value << "\n";
            break;
        }
        case Expr::Kind::Var: {
            const auto identifierExpr = dynamic_cast<const VarExpr*>(expr);
            oss << indentStr << "Var: " << identifierExpr->name << "\n";
            break;
        }
        case Expr::Kind::Unary: {
            const auto unaryExpr = dynamic_cast<const UnaryExpr*>(expr);
            oss << indentStr << "Unary: " << unaryOpToString(unaryExpr->op) << "\n";
            break;
        }
        case Expr::Kind::Binary: {
            const auto binaryExpr = dynamic_cast<const BinaryExpr*>(expr);
            oss << indentStr << "Binary: " << binaryOpToString(binaryExpr->op) << "\n";
            print(binaryExpr->lhs.get(), indent + 1);
            print(binaryExpr->rhs.get(), indent + 1);
            break;
        }
        case Expr::Kind::Assignment: {
            const auto assignmentExpr = dynamic_cast<const AssignmentExpr*>(expr);
            oss << indentStr << "Assignment: " << "\n";
            print(assignmentExpr->lhs.get(), indent + 1);
            print(assignmentExpr->rhs.get(), indent + 1);
            break;
        }
    }
}
} // namespace Parsing