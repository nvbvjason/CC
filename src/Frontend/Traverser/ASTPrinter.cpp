#include "ASTPrinter.hpp"
#include "ASTParser.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"

namespace Parsing {

namespace {

std::string varTypeToString(const Type kind)
{
    switch (kind) {
        case Type::Char:    return "char";
        case Type::I8:      return "int8";
        case Type::U8:      return "uint8";
        case Type::I32:     return "int";
        case Type::U32:     return "uint";
        case Type::I64:     return "long";
        case Type::U64:     return "ulong";
        case Type::Double:  return "double";
        case Type::Pointer: return "pointer";
        case Type::Array:   return "array";
        default:            return "unknown";
    }
}

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
        default: return "unknown";
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
        case Operator::Modulo:          return "%";
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
        default:                        return "unknown";
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
        default:                            return "unknown";
    }
}

std::string storageClass(const Declaration::StorageClass storageClass)
{
    using StorageClass = Declaration::StorageClass;
    switch (storageClass) {
        case StorageClass::None:            return "None";
        case StorageClass::Static:          return "Static";
        case StorageClass::Extern:          return "Extern";
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
    addLine("VarDecl " + varDecl.name + ' ' + storageClass(varDecl.storage) + ' ' + varTypeToString(varDecl.type->type));
    ConstASTTraverser::visit(varDecl);
}

void ASTPrinter::visit(const FuncDeclaration& funDecl)
{
    IndentGuard guard(m_indentLevel);
    addLine("FunDecl: " + funDecl.name + ' ' + storageClass(funDecl.storage));
    auto type = dynCast<const Parsing::FuncType>(funDecl.type.get());
    addLine("ReturnType " + varTypeToString(type->returnType->type));
    std::string args = "args: ";
    for (i64 i = 0; i < funDecl.params.size(); ++i) {
        if (i != 0)
            args += ", ";
        args += varTypeToString(type->params[i]->type) + " " + funDecl.params[i];
    }
    addLine(args);
    ConstASTTraverser::visit(funDecl);
    addLine("");
    addLine("");
}

void ASTPrinter::visit(const Block& block)
{
    IndentGuard guard(m_indentLevel);
    addLine("Block");
    ConstASTTraverser::visit(block);
}

void ASTPrinter::visit(const ZeroInitializer& zeroInitializer)
{
    IndentGuard guard(m_indentLevel);
    addLine(std::to_string(zeroInitializer.size) + " zero init");
}

void ASTPrinter::visit(const StringInitializer& stringInitializer)
{
    IndentGuard guard(m_indentLevel);
    if (stringInitializer.nullTerminated)
        addLine(stringInitializer.value + " is null terminated " + " string init");
    else
        addLine(stringInitializer.value + " is not null terminated " + " string init");
}

void ASTPrinter::visit(const VarType& varType)
{
    addLine(varTypeToString(varType.type));
    ConstASTTraverser::visit(varType);
}

void ASTPrinter::visit(const FuncType& functionType)
{
    ConstASTTraverser::visit(functionType);
}

void ASTPrinter::visit(const PointerType& pointerType)
{
    ConstASTTraverser::visit(pointerType);
}

void ASTPrinter::visit(const ArrayType& arrayType)
{
    ConstASTTraverser::visit(arrayType);
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
    if (unaryExpr.type)
        addLine("Unary " + unaryOpToString(unaryExpr.op) + " " + varTypeToString(unaryExpr.type->type));
    else
        addLine("Unary " + unaryOpToString(unaryExpr.op));
    ConstASTTraverser::visit(unaryExpr);
}

void ASTPrinter::visit(const StringExpr& stringExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("StringExpr: " + stringExpr.value);
    ConstASTTraverser::visit(stringExpr);
}

void ASTPrinter::visit(const CastExpr& castExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("Cast (" + varTypeToString(castExpr.type->type) + ')');
    ConstASTTraverser::visit(castExpr);
}

void ASTPrinter::visit(const BinaryExpr& binaryExpr)
{
    IndentGuard guard(m_indentLevel);
    if (binaryExpr.type)
        addLine("Binary " + binaryOpToString(binaryExpr.op) + " " + varTypeToString(binaryExpr.type->type));
    else
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
    if (constExpr.type->type == Type::I8)
        addLine(std::to_string(std::get<i8>(constExpr.value)) + ' ' + varTypeToString(Type::I8));
    else if (constExpr.type->type == Type::U8)
        addLine(std::to_string(std::get<u8>(constExpr.value)) + ' ' + varTypeToString(Type::U8));
    else if (constExpr.type->type == Type::Char)
        addLine(std::to_string(std::get<char>(constExpr.value)) + ' ' + varTypeToString(Type::Char));
    else if (constExpr.type->type == Type::I32)
        addLine(std::to_string(std::get<i32>(constExpr.value)) + ' ' + varTypeToString(Type::I32));
    else if (constExpr.type->type == Type::I64)
        addLine(std::to_string(std::get<i64>(constExpr.value)) + ' ' + varTypeToString(Type::I64));
    else if (constExpr.type->type == Type::U32)
        addLine(std::to_string(std::get<u32>(constExpr.value)) + ' ' + varTypeToString(Type::U32));
    else if (constExpr.type->type == Type::U64)
        addLine(std::to_string(std::get<u64>(constExpr.value)) + ' ' + varTypeToString(Type::U64));
    else if (constExpr.type->type == Type::Double)
        addLine(std::to_string(std::get<double>(constExpr.value)) + ' ' + varTypeToString(Type::Double));
    ConstASTTraverser::visit(constExpr);
}

void ASTPrinter::visit(const VarExpr& varExpr)
{
    IndentGuard guard(m_indentLevel);
    if (varExpr.type)
        addLine(varExpr.name + " " + varTypeToString(varExpr.type->type));
    else
        addLine(varExpr.name);
    ConstASTTraverser::visit(varExpr);
}

void ASTPrinter::visit(const TernaryExpr& conditionalExpr)
{
    IndentGuard guard(m_indentLevel);
    conditionalExpr.condition->accept(*this);
    addLine("?");
    conditionalExpr.trueExpr->accept(*this);
    addLine(":");
    conditionalExpr.falseExpr->accept(*this);
    addLine(";");
}

void ASTPrinter::visit(const NullStmt& nullStmt)
{
    IndentGuard guard(m_indentLevel);
    addLine("NullStmt (;)");
}

void ASTPrinter::visit(const FuncCallExpr& functionCallExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("Function Call: " + functionCallExpr.name);
    ConstASTTraverser::visit(functionCallExpr);
}

void ASTPrinter::visit(const AddrOffExpr& addrOffExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("AddressOf" );
    ConstASTTraverser::visit(addrOffExpr);
}

void ASTPrinter::visit(const DereferenceExpr& dereferenceExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("Dereference");
    ConstASTTraverser::visit(dereferenceExpr);
}

void ASTPrinter::visit(const SubscriptExpr& subscriptExpr)
{
    IndentGuard guard(m_indentLevel);
    addLine("Subscript");
    ConstASTTraverser::visit(subscriptExpr);
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