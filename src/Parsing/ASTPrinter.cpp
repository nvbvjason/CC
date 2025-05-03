#include "ASTPrinter.hpp"

namespace Parsing {

namespace {

std::string unaryOpToString(const UnaryExpr::Operator op)
{
    using Operator = UnaryExpr::Operator;
    switch (op) {
        case Operator::Complement:           return "~";
        case Operator::Negate:               return "-";
        case Operator::Not:                  return "!";
        case Operator::PostFixDecrement:     return "post --";
        case Operator::PrefixDecrement:      return "pre  --";
        case Operator::PostFixIncrement:     return "post ++";
        case Operator::PrefixIncrement:      return "pre  ++";
        default: return "?";
    }
}

std::string binaryOpToString(const BinaryExpr::Operator op)
{
    using Operator = BinaryExpr::Operator;
    switch (op) {
        case Operator::Add:             return "+";
        case Operator::Subtract:        return "-";
        case Operator::Multiply:        return "*";
        case Operator::Divide:          return "/";
        case Operator::Remainder:       return "%";
        case Operator::BitwiseAnd:      return "&";
        case Operator::BitwiseOr:       return "|";
        case Operator::BitwiseXor:      return "^";
        case Operator::LeftShift:       return "<<";
        case Operator::RightShift:      return ">>";
        case Operator::And:             return "&&";
        case Operator::Or:              return "||";
        case Operator::Equal:           return "==";
        case Operator::NotEqual:        return "!=";
        case Operator::LessThan:        return "<";
        case Operator::LessOrEqual:     return "<=";
        case Operator::GreaterThan  :   return ">";
        case Operator::GreaterOrEqual:  return ">=";
        default:                        return "?";
    }
}
std::string assignmentOpToString(const AssignmentExpr::Operator op)
{
    using Operator = AssignmentExpr::Operator;
    switch (op) {
        case Operator::Assign:              return "=";
        case Operator::PlusAssign:          return "+=";
        case Operator::MinusAssign:         return "-=";
        case Operator::DivideAssign:        return "/=";
        case Operator::MultiplyAssign:      return "*=";
        case Operator::ModuloAssign:        return "%=";
        case Operator::BitwiseAndAssign:    return "&=";
        case Operator::BitwiseOrAssign:     return "|=";
        case Operator::BitwiseXorAssign:    return "^=";
        case Operator::LeftShiftAssign:     return "<<=";
        case Operator::RightShiftAssign:    return ">>=";
        default: return "?";
    }
}

}

std::string ASTPrinter::print(const Program& program)
{
    oss.clear();
    oss << "Program:\n";
    if (program.function != nullptr)
        print(*program.function, 1);
    else
        oss << "  (empty)\n";
    return oss.str();
}

void ASTPrinter::print(const Function& function, const int indent)
{
    std::string indentStr(indent * 2, ' ');
    oss << indentStr << "Function: " << function.name << "\n";
    if (function.body.empty())
        oss << indentStr << "  (empty body)\n";
    for (const auto& blockItem : function.body)
        print(*blockItem, indent + 1);
}

void ASTPrinter::print(const BlockItem& blockItem, const int indent)
{;
    switch (blockItem.kind) {
        case BlockItem::Kind::Declaration: {
            const auto decl = dynamic_cast<const DeclarationBlockItem*>(&blockItem);
            print(*decl->decl ,indent);
            break;
        }
        case BlockItem::Kind::Statement: {
            const auto stmt = dynamic_cast<const StmtBlockItem*>(&blockItem);
            print(*stmt->stmt, indent);
            break;
        }
        default:
            break;
    }
}

void ASTPrinter::print(const Declaration& declaration, const int indent)
{
    std::string indentStr(indent * 2, ' ');
    oss << indentStr << "Declaration: " << declaration.name << "\n";
    if (declaration.init)
        print(*declaration.init, indent + 1);
}

void ASTPrinter::print(const Stmt& statement, const int indent)
{
    std::string indentStr(indent * 2, ' ');
    switch (statement.kind) {
        case Stmt::Kind::Null:
            oss << indentStr << "Null Statement\n";
            break;
        case Stmt::Kind::Return: {
            const auto returnStmt = dynamic_cast<const ReturnStmt*>(&statement);
            oss << indentStr << "Return Statement\n";
            print(*returnStmt->expression, indent + 1);
            break;
        }
        case Stmt::Kind::Expression: {
            const auto expressionStmt = dynamic_cast<const ExprStmt*>(&statement);
            oss << indentStr << "Expression Statement\n";
            print(*expressionStmt->expression, indent + 1);
            break;
        }
    }
}

void ASTPrinter::print(const Expr& expr, const int indent)
{
    const std::string indentStr(indent * 2, ' ');
    oss << indentStr << "Expression: " << "\n";
    switch (expr.kind) {
        case Expr::Kind::Constant: {
            const auto constantExpr = dynamic_cast<const ConstExpr*>(&expr);
            oss << indentStr << "Constant: " << constantExpr->value << "\n";
            break;
        }
        case Expr::Kind::Var: {
            const auto identifierExpr = dynamic_cast<const VarExpr*>(&expr);
            oss << indentStr << "Var: " << identifierExpr->name << "\n";
            break;
        }
        case Expr::Kind::Unary: {
            const auto unaryExpr = dynamic_cast<const UnaryExpr*>(&expr);
            oss << indentStr << "Unary: " << unaryOpToString(unaryExpr->op) << "\n";
            print(*unaryExpr->operand, indent + 1);
            break;
        }
        case Expr::Kind::Binary: {
            const auto binaryExpr = dynamic_cast<const BinaryExpr*>(&expr);
            oss << indentStr << "Binary: " << binaryOpToString(binaryExpr->op) << "\n";
            print(*binaryExpr->lhs, indent + 1);
            print(*binaryExpr->rhs, indent + 1);
            break;
        }
        case Expr::Kind::Assignment: {
            const auto assignmentExpr = dynamic_cast<const AssignmentExpr*>(&expr);
            oss << indentStr << "Assignment: " << assignmentOpToString(assignmentExpr->op) << "\n";
            print(*assignmentExpr->lhs, indent + 1);
            print(*assignmentExpr->rhs, indent + 1);
            break;
        }
    }
}
} // namespace Parsing