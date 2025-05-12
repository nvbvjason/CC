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
        case StorageClass::AutoLocalScope:           return "AutoLocalScope";
        case StorageClass::GlobalScopeDeclaration:   return "AutoGlobalScope";
        case StorageClass::StaticGlobalInitialized:  return "StaticGlobalInitialized";
        case StorageClass::StaticGlobalTentative:    return "StaticGlobalTentative";
        case StorageClass::StaticLocal:              return "StaticLocal";
        case StorageClass::ExternFunction:           return "ExternFunction";
        case StorageClass::ExternLocal:              return "ExternLocal";
        case StorageClass::ExternGlobal:             return "ExternGlobal";
        case StorageClass::ExternGlobalInitialized:  return "ExternGlobalInitialized";
        case StorageClass::GlobalDefinition:         return "GlobalDefinition";
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
    ConstASTTraverser::visit(program);
}

void ASTPrinter::visit(const VarDecl& varDecl)
{
    IndentGuard guard(m_indentLevel);
    addLine("VarDecl " + varDecl.name + ' ' + storageClass(varDecl.storage));
    ConstASTTraverser::visit(varDecl);
}

void ASTPrinter::visit(const FunDecl& funDecl)
{
    IndentGuard guard(m_indentLevel);
    addLine("FunDecl: " + funDecl.name + ' ' + storageClass(funDecl.storage));
    std::string args = "args:";
    for (const std::string arg : funDecl.params)
        args += " " + arg + " ";
    addLine(args);
    ConstASTTraverser::visit(funDecl);
}

void ASTPrinter::visit(const Block& block)
{
    IndentGuard guard(m_indentLevel);
    addLine("Block");
    ConstASTTraverser::visit(block);
}

void ASTPrinter::visit(const StmtBlockItem& stmtBlockItem)
{
    IndentGuard guard(m_indentLevel);
    addLine("StmtBlockItem: ");
    ConstASTTraverser::visit(stmtBlockItem);
}

void ASTPrinter::visit(const DeclBlockItem& declBlockItem)
{
    IndentGuard guard(m_indentLevel);
    addLine("DeclBlockItem: ");
    ConstASTTraverser::visit(declBlockItem);
}

void ASTPrinter::visit(const DeclForInit& declForInit)
{
    IndentGuard guard(m_indentLevel);
    addLine("DeclForInit: ");
    ConstASTTraverser::visit(declForInit);
}

void ASTPrinter::visit(const ExprForInit& exprForInit)
{
    IndentGuard guard(m_indentLevel);
    addLine("ExprForInit: ");
    ConstASTTraverser::visit(exprForInit);
}

void ASTPrinter::visit(const IfStmt& ifStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("IfStmt: ");
    ConstASTTraverser::visit(ifStmt);
}

void ASTPrinter::visit(const GotoStmt& gotoStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("GotoStmt " + gotoStmt.identifier);
    ConstASTTraverser::visit(gotoStmt);
}

void ASTPrinter::visit(const ReturnStmt& returnStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("Return: ");
    ConstASTTraverser::visit(returnStmt);
}

void ASTPrinter::visit(const ExprStmt& exprStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("ExprStmt: ");
    ConstASTTraverser::visit(exprStmt);
}

void ASTPrinter::visit(const CompoundStmt& function)
{
    IndentGuard guard(m_indentLevel);
    addLine("CompoundStmt:");
    ConstASTTraverser::visit(function);
}

void ASTPrinter::visit(const BreakStmt& breakStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("BreakStmt " + breakStmt.identifier);
}

void ASTPrinter::visit(const ContinueStmt& continueStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("ContinueStmt "  + continueStmt.identifier);
}

void ASTPrinter::visit(const LabelStmt& labelStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("LabelStmt "  + labelStmt.identifier);
}

void ASTPrinter::visit(const CaseStmt& caseStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("CaseStmt: " + caseStmt.identifier);
    ConstASTTraverser::visit(caseStmt);
}

void ASTPrinter::visit(const DefaultStmt& defaultStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("DefaultStmt: ");
    ConstASTTraverser::visit(defaultStmt);
}

void ASTPrinter::visit(const WhileStmt& whileStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("WhileStmt: ");
    ConstASTTraverser::visit(whileStmt);
}

void ASTPrinter::visit(const DoWhileStmt& doWhileStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("DoWhileStmt: ");
    ConstASTTraverser::visit(doWhileStmt);
}

void ASTPrinter::visit(const ForStmt& forStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("ForStmt: ");
    ConstASTTraverser::visit(forStmt);
}

void ASTPrinter::visit(const SwitchStmt& switchStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("SwitchStmt: ");
    ConstASTTraverser::visit(switchStmt);
}

void ASTPrinter::visit(const UnaryExpr& unaryExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("(" + unaryOpToString(unaryExpr.op) + " ");
    ConstASTTraverser::visit(unaryExpr);
}

void ASTPrinter::visit(const BinaryExpr& binaryExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("Binary " + binaryOpToString(binaryExpr.op));
    ConstASTTraverser::visit(binaryExpr);
}

void ASTPrinter::visit(const AssignmentExpr& assignmentExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("AssignmentExpr: " + assignmentOpToString(assignmentExpr.op));
    assignmentExpr.lhs->accept(*this);
    addLine(assignmentOpToString(assignmentExpr.op));
    assignmentExpr.rhs->accept(*this);
}

void ASTPrinter::visit(const ConstExpr& constExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine(std::to_string(constExpr.value));
    ConstASTTraverser::visit(constExpr);
}

void ASTPrinter::visit(const VarExpr& varExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine(varExpr.name);
    ConstASTTraverser::visit(varExpr);
}

void ASTPrinter::visit(const ConditionalExpr& conditionalExpr)
{
    IndentGuard guard(m_indentLevel);
    conditionalExpr.condition->accept(*this);
    addLine("?");
    conditionalExpr.first->accept(*this);
    addLine(":");
    conditionalExpr.second->accept(*this);
    addLine(";");
}

void ASTPrinter::visit(const NullStmt& nullStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("NullStmt (;)");
}

void ASTPrinter::visit(const FunCallExpr& functionCallExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("Function Call: " + functionCallExpr.identifier);
    ConstASTTraverser::visit(functionCallExpr);
}

void ASTPrinter::addLine(const std::string& line)
{
    oss << getIndent() << line << '\n';
}

std::string ASTPrinter::getIndent() const
{
    return std::string(m_indentLevel * m_indentMultiplier, ' ');
}
} // namespace Parsing
