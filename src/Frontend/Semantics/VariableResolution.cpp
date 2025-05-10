#include "VariableResolution.hpp"

namespace Semantics {

bool VariableResolution::resolve()
{
    m_valid = true;
    m_counter = 0;
    program.accept(*this);
    return m_valid;
}

void VariableResolution::visit(Parsing::Program& program)
{
    m_variableStack.push();
    for (auto& decl : program.declarations)
        if (decl->kind == Parsing::Declaration::Kind::VariableDeclaration) {
            auto varDecl = static_cast<Parsing::VarDecl*>(decl.get());
            m_variableStack.addDecl(varDecl->name,
                                    varDecl->name,
                                    Variable::Type::Int,
                                    varDecl->storageClass);
        }
    for (auto& decl : program.declarations) {
        if (decl->kind == Parsing::Declaration::Kind::VariableDeclaration)
            continue;
        auto funcDecl = static_cast<Parsing::FunDecl*>(decl.get());
        VariableResolution::visit(*funcDecl);
    }
    m_variableStack.pop();
}

bool VariableResolution::isIllegalFuncDeclaration(const Parsing::FunDecl& funDecl)
{
    if (hasDuplicates(funDecl.params)) {
        m_valid = false;
        return true;
    }
    if (!m_variableStack.tryDeclare(funDecl.name, Variable::Type::Function, funDecl.storageClass)) {
        m_valid = false;
        return true;
    }
    if (m_funcDecls.contains(funDecl.name)) {
        if (funDecl.params.size() != m_funcDecls[funDecl.name].size()) {
            m_valid = false;
            return true;
        }
    }
    if (functionDefinitionInOtherFunctionBody(funDecl)) {
        m_valid = false;
        return true;
    }
    if (functionAlreadyDefined(funDecl)) {
        m_valid = false;
        return true;
    }
    return false;
}

void VariableResolution::visit(Parsing::FunDecl& funDecl)
{
    if (!m_valid)
        return;
    if (isIllegalFuncDeclaration(funDecl))
        return;
    m_funcDecls[funDecl.name] = funDecl.params;
    m_variableStack.addDecl(funDecl.name,
                            makeTemporary(funDecl.name),
                                Variable::Type::Function,
                                funDecl.storageClass);
    if (funDecl.body == nullptr)
        return;
    m_definedFunctions.insert(funDecl.name);
    m_inFunctionBody = true;
    m_variableStack.addArgs(funDecl.params);
    ASTTraverser::visit(funDecl);
    m_variableStack.clearArgs();
    m_inFunctionBody = false;
}

void VariableResolution::visit(Parsing::Block& block)
{
    m_variableStack.push();
    ASTTraverser::visit(block);
    m_variableStack.pop();
}

void VariableResolution::visit(Parsing::ForStmt& function)
{
    m_variableStack.push();
    ASTTraverser::visit(function);
    m_variableStack.pop();
}

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    if (!m_valid)
         return;
    if (m_variableStack.cannotDeclareInInnerMost(varDecl.name, varDecl.storageClass)) {
        m_valid = false;
        return;
    }
    if (!m_variableStack.tryDeclare(varDecl.name, Variable::Type::Int, varDecl.storageClass)) {
        m_valid = false;
        return;
    }
    const std::string uniqueName = makeTemporary(varDecl.name);
    m_variableStack.addDecl(varDecl.name, uniqueName, Variable::Type::Int, varDecl.storageClass);
    varDecl.name = uniqueName;
    if (varDecl.init != nullptr)
        varDecl.init->accept(*this);
}

void VariableResolution::visit(Parsing::VarExpr& varExpr)
{
    if (!m_valid)
        return;
    if (m_variableStack.inArg(varExpr.name))
        return;
    const std::string varName = m_variableStack.tryCall(varExpr.name, Variable::Type::Int);
    if (varName.empty()) {
        m_valid = false;
        return;
    }
    varExpr.name = varName;
}

void VariableResolution::visit(Parsing::AssignmentExpr& assignmentExpr)
{
    if (!m_valid)
        return;
    if (assignmentExpr.lhs->kind != Parsing::Expr::Kind::Var) {
        m_valid = false;
        return;
    }
    auto varExpr = static_cast<Parsing::VarExpr*>(assignmentExpr.lhs.get());
    const std::string varName = m_variableStack.tryCall(varExpr->name, Variable::Type::Int);
    if (varName.empty()) {
        m_valid = false;
        return;
    }
    varExpr->name = varName;
    assignmentExpr.rhs->accept(*this);
}

void VariableResolution::visit(Parsing::FunCallExpr& funCallExpr)
{
    if (!m_valid)
        return;
    const std::string varName = m_variableStack.tryCall(funCallExpr.identifier, Variable::Type::Function);
    if (varName.empty()) {
        m_valid = false;
        return;
    }
    if (m_funcDecls[funCallExpr.identifier].size() != funCallExpr.args.size()) {
        m_valid = false;
        return;
    }
    ASTTraverser::visit(funCallExpr);
}

std::string VariableResolution::makeTemporary(const std::string& name)
{
    return name + '.' + std::to_string(m_counter++);
}

bool VariableResolution::hasDuplicates(const std::vector<std::string>& vec)
{
    std::unordered_set<std::string> seen;
    for (const auto& str : vec) {
        if (seen.contains(str))
            return true;
        seen.insert(str);
    }
    return false;
}

bool VariableResolution::functionAlreadyDefined(const Parsing::FunDecl& funDecl) const
{
    return funDecl.body != nullptr && m_definedFunctions.contains(funDecl.name);
}

bool VariableResolution::functionDefinitionInOtherFunctionBody(const Parsing::FunDecl& funDecl) const
{
    return m_inFunctionBody && funDecl.body != nullptr;
}
} // Semantics