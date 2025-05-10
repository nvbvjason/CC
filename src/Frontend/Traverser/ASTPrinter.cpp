#include "ASTPrinter.hpp"

#include "ASTParser.hpp"

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

std::string storageClass(const Declaration::StorageClass storageClass)
{
    using StorageClass = Declaration::StorageClass;
    switch (storageClass) {
        case StorageClass::Static:       return "static";
        case StorageClass::Extern:       return "extern";
        case StorageClass::None:         return "none";
        default:
            return "storageClass not defined";
    }
}

}

std::string ASTPrinter::getString() const
{
    return oss.str();
}

void ASTPrinter::visit(const Program& program)
{
    addLine("Program:");
    for (const auto& funDecl : program.declarations)
        funDecl->accept(*this);
}

void ASTPrinter::visit(const VarDecl& varDecl)
{
    IndentGuard guard(m_indetLevel);
    addLine("VarDecl " + varDecl.name + ' ' + storageClass(varDecl.storageClass));
    if (varDecl.init)
        varDecl.init->accept(*this);
}

void ASTPrinter::visit(const FunDecl& funDecl)
{
    IndentGuard guard(m_indetLevel);
    addLine("FunDecl: " + funDecl.name + ' ' + storageClass(funDecl.storageClass));
    if (funDecl.body)
        funDecl.body->accept(*this);
}

void ASTPrinter::visit(const Block& block)
{
    IndentGuard guard(m_indetLevel);
    addLine("Block");
    for (const auto& blockItem : block.body)
        blockItem->accept(*this);
}

void ASTPrinter::visit(const StmtBlockItem& stmtBlockItem)
{
    IndentGuard guard(m_indetLevel);
    addLine("StmtBlockItem:");
    stmtBlockItem.stmt->accept(*this);
}

void ASTPrinter::visit(const DeclBlockItem& declBlockItem)
{
    IndentGuard guard(m_indetLevel);
    addLine("DeclBlockItem:");
    declBlockItem.decl->accept(*this);
}

void ASTPrinter::visit(const DeclForInit& declForInit)
{
    IndentGuard guard(m_indetLevel);
    addLine("DeclForInit: ");
    declForInit.decl->accept(*this);
}

void ASTPrinter::visit(const ExprForInit& exprForInit)
{
    IndentGuard guard(m_indetLevel);
    addLine("ExprForInit: ");
    exprForInit.expression->accept(*this);
}

void ASTPrinter::visit(const IfStmt& ifStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("IfStmt: ");
    ifStmt.condition->accept(*this);
    ifStmt.thenStmt->accept(*this);
    if (ifStmt.elseStmt) {
        addLine("ElseStmt: ");
        ifStmt.elseStmt->accept(*this);
    }
}

void ASTPrinter::visit(const GotoStmt& gotoStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("GotoStmt " + gotoStmt.identifier);
}

void ASTPrinter::visit(const ReturnStmt& returnStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("Return: ");
    returnStmt.expr->accept(*this);
}

void ASTPrinter::visit(const ExprStmt& exprStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("ExprStmt: ");
    exprStmt.expr->accept(*this);
}

void ASTPrinter::visit(const CompoundStmt& function)
{
    IndentGuard guard(m_indetLevel);
    addLine("CompoundStmt:");
    function.block->accept(*this);
}

void ASTPrinter::visit(const BreakStmt& breakStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("BreakStmt " + breakStmt.identifier);
}

void ASTPrinter::visit(const ContinueStmt& continueStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("ContinueStmt "  + continueStmt.identifier);
}

void ASTPrinter::visit(const LabelStmt& labelStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("LabelStmt "  + labelStmt.identifier);
}

void ASTPrinter::visit(const CaseStmt& caseStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("CaseStmt: " + caseStmt.identifier);
    caseStmt.condition->accept(*this);
    caseStmt.body->accept(*this);
}

void ASTPrinter::visit(const DefaultStmt& defaultStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("DefaultStmt: ");
    defaultStmt.body->accept(*this);
}

void ASTPrinter::visit(const WhileStmt& whileStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("WhileStmt: ");
    whileStmt.condition->accept(*this);
    whileStmt.body->accept(*this);
}

void ASTPrinter::visit(const DoWhileStmt& doWhileStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("DoWhileStmt: ");
    doWhileStmt.body->accept(*this);
    doWhileStmt.condition->accept(*this);
}

void ASTPrinter::visit(const ForStmt& forStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("ForStmt: ");
    if (forStmt.init)
        forStmt.init->accept(*this);
    if (forStmt.condition)
        forStmt.condition->accept(*this);
    if (forStmt.post)
        forStmt.post->accept(*this);
}

void ASTPrinter::visit(const SwitchStmt& switchStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("SwitchStmt: ");
    switchStmt.condition->accept(*this);
    switchStmt.body->accept(*this);
}

void ASTPrinter::visit(const UnaryExpr& unaryExpr)
{
    IndentGuard guard(m_indetLevel);
    addLine("(" + unaryOpToString(unaryExpr.op) + " ");
    unaryExpr.operand->accept(*this);
}

void ASTPrinter::visit(const BinaryExpr& binaryExpr)
{
    IndentGuard guard(m_indetLevel);
    addLine("Binary " + binaryOpToString(binaryExpr.op));
    binaryExpr.lhs->accept(*this);
    binaryExpr.rhs->accept(*this);
}

void ASTPrinter::visit(const AssignmentExpr& assignmentExpr)
{
    IndentGuard guard(m_indetLevel);
    assignmentExpr.lhs->accept(*this);
    addLine(assignmentOpToString(assignmentExpr.op));
    assignmentExpr.rhs->accept(*this);
}

void ASTPrinter::visit(const ConstExpr& constExpr)
{
    IndentGuard guard(m_indetLevel);
    addLine(std::to_string(constExpr.value));
}

void ASTPrinter::visit(const VarExpr& varExpr)
{
    IndentGuard guard(m_indetLevel);
    addLine(varExpr.name);
}

void ASTPrinter::visit(const ConditionalExpr& conditionalExpr)
{
    IndentGuard guard(m_indetLevel);
    conditionalExpr.condition->accept(*this);
    addLine("?");
    conditionalExpr.first->accept(*this);
    addLine(":");
    conditionalExpr.second->accept(*this);
    addLine(";");
}

void ASTPrinter::visit(const NullStmt& nullStmt)
{
    IndentGuard guard(m_indetLevel);
    addLine("NullStmt (;)");
}

void ASTPrinter::visit(const FunCallExpr& functionCallExpr)
{
    IndentGuard guard(m_indetLevel);
    addLine("Function Call");
    for (const auto& expr : functionCallExpr.args)
        expr->accept(*this);
}

void ASTPrinter::addLine(const std::string& line)
{
    oss << getIndent() << line << '\n';
}

std::string ASTPrinter::getIndent() const
{
    return std::string(m_indetLevel * 2, ' ');
}
} // namespace Parsing
