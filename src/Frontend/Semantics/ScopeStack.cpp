#include "ScopeStack.hpp"
#include "ShortTypes.hpp"

#include <cassert>

#include "Frontend/Parsing/Operators.hpp"

namespace Semantics {

void ScopeStack::pop()
{
    assert(!m_stack.empty() && "VariableStack underflow pop()");
    m_stack.pop_back();
}

void ScopeStack::push()
{
    m_stack.emplace_back();
}

void ScopeStack::addDecl(const std::string& name,
                         const std::string& value,
                         const Variable::Type type,
                         const Parsing::Declaration::StorageClass storageClass)
{
    assert(!m_stack.empty() && "VariableStack underflow addDecl()");
    auto it = m_stack.back().find(name);
    if (it->second.storage == St)
    m_stack.back().emplace(name, Variable(value, type, storageClass));
}

bool ScopeStack::tryDeclareGlobal(const std::string& name,
                                  const Variable::Type type,
                                  const Parsing::Declaration::StorageClass storageClass) const
{
    assert(m_stack.size() == 1 && "VariableStack underflow tryDeclareGlobal()");
    if (storageClass == StorageClass::GlobalScopeDeclaration)
        return true;
    if (storageClass == StorageClass::ExternGlobal)
        return true;
    auto it = m_stack.back().find(name);
    if (it == m_stack.back().end())
        return true;
    return false;
}

bool ScopeStack::tryDeclare(const std::string& name,
                            const Variable::Type type,
                            const Parsing::Declaration::StorageClass storageClass) const
{
    assert(!m_stack.empty() && "VariableStack underflow isDeclared()");
    if (m_args.contains(name))
        return false;
    for (i64 i = m_stack.size() - 1; 0 <= i; --i) {
        const auto it = m_stack[i].find(name);
        if (it == m_stack[i].end())
            continue;
        if ((it->second.storage != storageClass ||
             it->second.type != type) &&
            i == m_stack.size() - 1)
            return false;
        return true;
    }
    return true;
}

std::tuple<std::string, bool> ScopeStack::tryCall(const std::string& callName,
                                                  const Variable::Type type) const
{
    if (m_args.contains(callName))
        return {callName, true};
    assert(!m_stack.empty() && "VariableStack underflow tryCall()");
    for (i64 i = m_stack.size() - 1; 0 <= i; --i) {
        const auto it = m_stack[i].find(callName);
        if (it == m_stack[i].end())
            continue;
        if (it->second.type != type && i == m_stack.size() - 1)
            return {"", false};
        if (it->second.type != type)
            continue;
        return {it->second.name, true};
    }
    return {"", false};
}

void ScopeStack::addArgs(const std::vector<std::string>& args)
{
    for (const auto& arg : args)
        m_args.emplace(arg);
}

void ScopeStack::clearArgs()
{
    m_args.clear();
}

bool ScopeStack::inArgs(const std::string& name) const noexcept
{
    return m_args.contains(name);
}

bool ScopeStack::existInInnerMost(const std::string& name,
                                  const StorageClass storageClass) const
{
    const auto it = m_stack.back().find(name);
    if (it == m_stack.back().end())
        return false;
    if (it->second.storage == StorageClass::ExternLocal &&
        storageClass == StorageClass::ExternLocal)
        return false;
    return true;
}
} // Semantics