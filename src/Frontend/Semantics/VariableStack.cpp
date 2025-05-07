#include "VariableStack.hpp"

#include <cassert>
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
} // Semantics