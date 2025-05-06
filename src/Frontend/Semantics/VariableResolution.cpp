#include "VariableResolution.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <ranges>

namespace Semantics {
void VariableStack::pop()
{
    assert(!m_stack.empty() && "VariableStack underflow pop()");
    m_stack.pop_back();
}

void VariableStack::addDecl(const std::string& name, const std::string& value)
{
    assert(!m_stack.empty() && "VariableStack underflow addDecl()");
    m_stack.back().emplace(name, value);
}

void VariableStack::push()
{
    m_stack.emplace_back();
}

bool VariableStack::isDeclared(const std::string& name) const
{
    assert(!m_stack.empty() && "VariableStack underflow isDeclared()");
    return m_stack.back().contains(name);
}

bool VariableStack::tryRename(const std::string& oldName, const std::string& newName)
{
    for (auto & it : std::ranges::reverse_view(m_stack))
        if (it.contains(oldName)) {
            it[oldName] = newName;
            return true;
        }
    return false;
}

bool VariableStack::contains(const std::string& name) const noexcept
{
    return find(name) != nullptr;
}

std::string* VariableStack::find(const std::string& name) noexcept
{
    for (auto& scope : std::ranges::reverse_view(m_stack))
        if (auto it = scope.find(name); it != scope.end())
            return &it->second;
    return nullptr;
}

const std::string* VariableStack::find(const std::string& name) const noexcept
{
    return const_cast<VariableStack*>(this)->find(name);
}

bool VariableResolution::resolve()
{
    m_valid = true;
    m_counter = 0;
    program.accept(*this);
    return m_valid;
}

void VariableResolution::visit(Parsing::Block& block)
{
    m_variableStack.push();
    for (auto& blockItem : block.body)
        blockItem->accept(*this);
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

void VariableResolution::visit(Parsing::ForStmt& function)
{
    m_variableStack.push();
    if (function.init != nullptr)
        function.init->accept(*this);
    if (function.condition != nullptr)
        function.condition->accept(*this);
    if (function.post != nullptr)
        function.post->accept(*this);
    function.body->accept(*this);
    m_variableStack.pop();
}

void VariableResolution::visit(Parsing::VarDecl& varDecl)
{
    if (!m_valid)
        return;
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

std::string VariableResolution::makeTemporary(const std::string& name)
{
    return name + '.' + std::to_string(m_counter++);
}
} // Semantics