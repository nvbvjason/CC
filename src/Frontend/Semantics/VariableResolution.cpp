#include "VariableResolution.hpp"

#include <unordered_set>

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
    ASTTraverser::visit(program);
    m_variableStack.pop();
}

void VariableResolution::visit(Parsing::FunDecl& funDecl)
{
    if (!m_valid)
        return;
    if (hasDuplicates(funDecl.params)) {
        m_valid = false;
        return;
    }
    if (m_variableStack.isDeclared(funDecl.name)) {
        m_valid = false;
        return;
    }
    if (m_funcDecls.contains(funDecl.name)) {
        if (funDecl.params.size() != m_funcDecls[funDecl.name].size()) {
            m_valid = false;
            return;
        }
    }
    if (m_inFunctionBody && funDecl.body != nullptr) {
        m_valid = false;
        return;
    }
    m_funcDecls[funDecl.name] = funDecl.params;
    if (funDecl.body == nullptr)
        return;
    if (m_definedFunctions.contains(funDecl.name)) {
        m_valid = false;
        return;
    }
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

void VariableResolution::visit(Parsing::ContinueStmt& continueStmt)
{
    if (!m_valid)
        return;
    if (continueStmt.identifier.empty())
        m_valid = false;
}

void VariableResolution::visit(Parsing::BreakStmt& breakStmt)
{
    if (!m_valid)
        return;
    if (breakStmt.identifier.empty())
        m_valid = false;
}

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    if (!m_valid)
         return;
    // if (m_funcDecls.contains(varDecl.name)) {
    //     m_valid = false;
    //     return;
    // }
    if (m_variableStack.isDeclared(varDecl.name)) {
        m_valid = false;
        return;
    }
    const std::string uniqueName = makeTemporary(varDecl.name);
    m_variableStack.addDecl(varDecl.name, uniqueName);
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
    auto varNamePtr = m_variableStack.find(varExpr.name);
    if (varNamePtr == nullptr) {
        m_valid = false;
        return;
    }
    varExpr.name = *varNamePtr;
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
    auto varNamePtr = m_variableStack.find(varExpr->name);
    if (varNamePtr == nullptr) {
        m_valid = false;
        return;
    }
    varExpr->name = *varNamePtr;
    assignmentExpr.rhs->accept(*this);
}

void VariableResolution::visit(Parsing::FunCallExpr& funCallExpr)
{
    if (!m_valid)
        return;
    if (!m_funcDecls.contains(funCallExpr.identifier)) {
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
} // Semantics