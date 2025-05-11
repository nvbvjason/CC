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
     m_stack.back()[name] = Variable(value, type, storageClass);
}

void ScopeStack::addExternGlobal(const std::string& name)
{
    assert(!m_stack.empty() && "VariableStack underflow addDecl()");
    m_stack.front().emplace(name, Variable(name, Variable::Type::Int, StorageClass::ExternLocal));
}

std::tuple<Variable, bool> ScopeStack::showIden(const std::string& name) const
{
    for (i64 i = m_stack.size() - 1; 0 < i; --i) {
        const auto it = m_stack[i].find(name);
        if (it == m_stack[i].end())
            continue;
        return {it->second, true};
    }
    return {Variable("", Variable::Type::Function, StorageClass::AutoLocalScope), false};
}

std::tuple<Variable, bool> ScopeStack::showIdenInnermost(const std::string& name) const
{
    if (const auto it = m_stack.back().find(name); it != m_stack.back().end())
        return {it->second, true};
    return {Variable("", Variable::Type::Function, StorageClass::AutoLocalScope), false};
}

std::tuple<Variable, bool> ScopeStack::showIdenGlobal(const std::string& name) const
{
    if (const auto it = m_stack.front().find(name); it != m_stack.front().end())
        return {it->second, true};
    return {Variable("", Variable::Type::Function, StorageClass::AutoLocalScope), false};
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
    if (storageClass == StorageClass::GlobalDefinition &&
        it->second.storage == StorageClass::GlobalDefinition)
        return false;
    return false;
}

bool ScopeStack::tryDeclare(const std::string& name,
                            const Variable::Type type,
                            const Parsing::Declaration::StorageClass storageClass) const
{
    assert(!m_stack.empty() && "VariableStack underflow isDeclared()");
    if (m_args.contains(name))
        return false;
    for (i64 i = m_stack.size() - 1; 0 < i; --i) {
        const auto it = m_stack[i].find(name);
        if (it == m_stack[i].end())
            continue;
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
        if (it->second.storage == StorageClass::ExternLocal && i == 0)
            return {"", false};
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
    if (it->second.storage == StorageClass::ExternFunction &&
        storageClass == StorageClass::ExternFunction)
        return false;
    return true;
}
} // Semantics