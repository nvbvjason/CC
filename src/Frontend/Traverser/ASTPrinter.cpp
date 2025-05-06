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

std::string ASTPrinter::getString() const
{
    return oss.str();
}

void ASTPrinter::visit(const Program& program)
{
    oss << "Program:\n";
    program.function->accept(*this);
}

void ASTPrinter::visit(const Function& function)
{
    oss << "  Function: " << function.name << "\n";
    function.body->accept(*this);
}

void ASTPrinter::visit(const Block& block)
{
    oss << "  Body:\n";
    for (const auto& blockItem : block.body)
        blockItem->accept(*this);
}

void ASTPrinter::visit(const StmtBlockItem& stmtBlockItem)
{
    oss << "    StmtBlockItem:\n";
    stmtBlockItem.stmt->accept(*this);
}

void ASTPrinter::visit(const DeclBlockItem& declBlockItem)
{
    oss << "    DeclBlockItem:\n";
    declBlockItem.decl->accept(*this);
}

void ASTPrinter::visit(const Declaration& declaration)
{
    oss << "      Declaration: " << declaration.name;
    if (declaration.init) {
        oss << " = ";
        declaration.init->accept(*this);
    }
    oss << "\n";
}

void ASTPrinter::visit(const DeclForInit& declForInit)
{
    oss << "      DeclForInit: ";
    declForInit.decl->accept(*this);
    oss << "\n";
}

void ASTPrinter::visit(const ExprForInit& exprForInit)
{
    oss << "      ExprForInit: ";
    exprForInit.expression->accept(*this);
    oss << "\n";
}

void ASTPrinter::visit(const IfStmt& ifStmt)
{
    oss << "      IfStmt: ";
    ifStmt.condition->accept(*this);
    oss << "\n";
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt) {
        oss << "      ElseStmt: ";
        ifStmt.elseStmt->accept(*this);
    }
}

void ASTPrinter::visit(const GotoStmt& gotoStmt)
{
    oss << "      GotoStmt "  << gotoStmt.identifier << '\n';
}

void ASTPrinter::visit(const ReturnStmt& returnStmt)
{
    oss << "      Return: ";
    returnStmt.expr->accept(*this);
    oss << "\n";
}

void ASTPrinter::visit(const ExprStmt& exprStmt)
{
    oss << "      ExprStmt: ";
    exprStmt.expr->accept(*this);
    oss << "\n";
}

void ASTPrinter::visit(const CompoundStmt& function)
{
    oss << "      CompoundStmt:\n";
    function.block->accept(*this);
}

void ASTPrinter::visit(const BreakStmt& breakStmt)
{
    oss << "      BreakStmt "  << breakStmt.identifier << '\n';
}

void ASTPrinter::visit(const ContinueStmt& continueStmt)
{
    oss << "      ContinueStmt "  << continueStmt.identifier << '\n';
}

void ASTPrinter::visit(const LabelStmt& labelStmt)
{
    oss << "      GotoStmt "  << labelStmt.identifier << '\n';
}

void ASTPrinter::visit(const CaseStmt& caseStmt)
{
    oss << "      CaseStmt: ";
    caseStmt.condition->accept(*this);
    caseStmt.body->accept(*this);
    oss << "\n";
}

void ASTPrinter::visit(const DefaultStmt& defaultStmt)
{
    oss << "      DefaultStmt: ";
    defaultStmt.body->accept(*this);
    oss << "\n";
}

void ASTPrinter::visit(const WhileStmt& whileStmt)
{
    oss << "      WhileStmt: ";
    whileStmt.condition->accept(*this);
    oss << "\n";
    whileStmt.body->accept(*this);
}

void ASTPrinter::visit(const DoWhileStmt& doWhileStmt)
{
    oss << "      DoWhileStmt: ";
    doWhileStmt.body->accept(*this);
    oss << "\n";
    doWhileStmt.condition->accept(*this);
}

void ASTPrinter::visit(const ForStmt& forStmt)
{
    oss << "      ForStmt: ";
    if (forStmt.init) {
        oss << " ";
        forStmt.init->accept(*this);
    }
    if (forStmt.condition) {
        oss << " ";
        forStmt.condition->accept(*this);
    }
    if (forStmt.post) {
        oss << " ";
        forStmt.post->accept(*this);
    }
}

void ASTPrinter::visit(const SwitchStmt& switchStmt)
{
    oss << "      SwitchStmt: ";
    switchStmt.condition->accept(*this);
    oss << "\n";
    switchStmt.body->accept(*this);
}

void ASTPrinter::visit(const UnaryExpr& unaryExpr)
{
    oss << "(" << unaryOpToString(unaryExpr.op) << " ";
    unaryExpr.operand->accept(*this);
    oss << ")";
}

void ASTPrinter::visit(const BinaryExpr& binaryExpr)
{
    oss << "(";
    binaryExpr.lhs->accept(*this);
    oss << " " << binaryOpToString(binaryExpr.op) << " ";
    binaryExpr.rhs->accept(*this);
    oss << ")";
}

void ASTPrinter::visit(const AssignmentExpr& assignmentExpr)
{
    oss << "(";
    assignmentExpr.lhs->accept(*this);
    oss << assignmentOpToString(assignmentExpr.op);
    assignmentExpr.rhs->accept(*this);
    oss << ")";
}

void ASTPrinter::visit(const ConstExpr& constExpr)
{
    oss << constExpr.value;
}

void ASTPrinter::visit(const VarExpr& varExpr)
{
    oss << varExpr.name;
}

void ASTPrinter::visit(const ConditionalExpr& conditionalExpr)
{
    conditionalExpr.condition->accept(*this);
    oss << "?";
    conditionalExpr.first->accept(*this);
    oss << ":";
    conditionalExpr.second->accept(*this);
    oss << ";";
}

void ASTPrinter::visit(const NullStmt& nullStmt)
{
    oss << "      NullStmt (;)\n";
}
} // namespace Parsing