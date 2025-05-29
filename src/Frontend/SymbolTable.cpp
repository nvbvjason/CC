#include "ASTIr.hpp"
#include "SymbolTable.hpp"

#include <cassert>

SymbolTable::SymbolTable()
{
    addScope();
}

bool SymbolTable::contains(const std::string& name) const
{
    for (i64 i = m_entries.size() - 1; 0 <= i; --i) {
        const auto it = m_entries[i].find(name);
        if (it == m_entries[i].end())
            continue;
        return true;
    }
    return false;
}

SymbolTable::ReturnedVarEntry SymbolTable::lookupVar(const std::string& uniqueName) const
{
    const bool inArgs = isInArgs(uniqueName);
    for (i64 i = m_entries.size() - 1; 0 <= i; --i) {
        const auto it = m_entries[i].find(uniqueName);
        if (it == m_entries[i].end())
            continue;
        const bool correctType = it->second.type == SymbolType::Var;
        const bool fromCurrentScope = i == m_entries.size() - 1;
        const bool internal = it->second.isSet(State::InternalLinkage);
        const bool external = it->second.isSet(State::ExternalLinkage);
        const bool global = it->second.isSet(State::Global);
        const bool defined = it->second.isSet(State::Defined);
        const State initState = it->second.getInit();
        return {true, inArgs, correctType, fromCurrentScope, internal, external, global, defined, initState};
    }
    return {false, inArgs, false, false, false, false, false, false, State::None};
}

SymbolTable::ReturnedFuncEntry SymbolTable::lookupFunc(const std::string& uniqueName) const
{
    for (i64 i = m_entries.size() - 1; 0 <= i; --i) {
        const auto it = m_entries[i].find(uniqueName);
        if (it == m_entries[i].end())
            continue;
        const bool correctType = it->second.type == SymbolType::Func;
        const bool fromCurrentScope = i == m_entries.size() - 1;
        const bool internal = it->second.isSet(State::InternalLinkage);
        const bool external = it->second.isSet(State::ExternalLinkage);
        const bool global = it->second.isSet(State::Global);
        const bool defined = it->second.isSet(State::Defined);
        i32 argsSize = 0;
        if (m_funcs.contains(uniqueName))
            argsSize = m_funcs.at(uniqueName);
        return {argsSize, true, correctType, fromCurrentScope, internal, external, global, defined};
    }
    return {false, false, false, false, false, false, false, false};
}

std::string SymbolTable::getUniqueName(const std::string& unique) const
{
    for (i64 i = m_entries.size() - 1; 0 <= i; --i) {
        const auto it = m_entries[i].find(unique);
        if (it == m_entries[i].end())
            continue;
        return it->second.uniqueName;
    }
    assert(false && "Should always get called after contains never happen in SymbolTable::getUniqueName");
    std::unreachable();
}

void SymbolTable::setArgs(const std::vector<std::string>& args)
{
    m_args = args;
}

void SymbolTable::clearArgs()
{
    m_args.clear();
}

void SymbolTable::addVarEntry(const std::string& name,
                              const std::string& uniqueName,
                              const bool internal,
                              const bool external,
                              const bool global,
                              const bool defined,
                              const State initState)
{
    m_entries.back().insert(
        std::make_pair(name,
            Entry(uniqueName, SymbolType::Var, internal, external, global, defined, initState)));
}

void SymbolTable::addFuncEntry(const std::string& name,
                               const i32 argsSize,
                               const bool internal,
                               const bool external,
                               const bool global,
                               const bool defined)
{
    m_entries.back().insert(std::make_pair(name, Entry(name, SymbolType::Func, internal, external, global, defined)));
    m_funcs.insert(std::make_pair(name, argsSize));
}

void SymbolTable::addScope()
{
    m_entries.emplace_back();
}

void SymbolTable::removeScope()
{
    m_entries.pop_back();
}

bool SymbolTable::isFunc(const std::string& name) const
{
    const auto it = m_entries.front().find(name);
    if (it == m_entries.front().end())
        return false;
    return it->second.type == SymbolType::Func;
}