#include "VariableStack.hpp"
#include "ShortTypes.hpp"

#include <cassert>

#include "Frontend/Parsing/Operators.hpp"

namespace Semantics {

void VariableStack::pop()
{
    assert(!m_stack.empty() && "VariableStack underflow pop()");
    m_stack.pop_back();
}

void VariableStack::push()
{
    m_stack.emplace_back();
}

void VariableStack::addDecl(const std::string& name,
                            const std::string& value,
                            const Variable::Type type,
                            const Parsing::Declaration::StorageClass storageClass)
{
    assert(!m_stack.empty() && "VariableStack underflow addDecl()");
    m_stack.back().emplace(name, Variable(value, type, storageClass));
}

bool VariableStack::tryDeclare(const std::string& name,
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

std::string VariableStack::tryCall(const std::string& name,
                                   const Variable::Type type) const
{
    if (m_args.contains(name))
        return name;
    assert(!m_stack.empty() && "VariableStack underflow tryCall()");
    for (i64 i = m_stack.size() - 1; 0 <= i; --i) {
        const auto it = m_stack[i].find(name);
        if (it == m_stack[i].end())
            continue;
        if (it->second.type != type && i == m_stack.size() - 1)
            return "";
        if (it->second.type != type)
            continue;
        return it->second.name;
    }
    return "";
}

void VariableStack::addArgs(const std::vector<std::string>& args)
{
    for (const auto& arg : args)
        m_args.emplace(arg);
}

void VariableStack::clearArgs()
{
    m_args.clear();
}

bool VariableStack::inArg(const std::string& name) const noexcept
{
    return m_args.contains(name);
}

bool VariableStack::cannotDeclareInInnerMost(const std::string& name,
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