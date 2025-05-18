#include "SymbolTable.hpp"
#include "ASTIr.hpp"

SymbolTable::SymbolTable()
{
    addScope();
}

bool SymbolTable::contains(const std::string& uniqueName) const
{
    for (i64 i = m_entries.size() - 1; 0 <= i; --i) {
        const auto it = m_entries[i].find(uniqueName);
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
        const bool wrongType = it->second.type != SymbolType::Var;
        const bool fromCurrentScope = i == m_entries.size() - 1;
        return {true, inArgs, wrongType, fromCurrentScope, it->second.hasLinkage};
    }
    return {false, inArgs, false, false, false};
}

SymbolTable::ReturnedFuncEntry SymbolTable::lookupFunc(const std::string& uniqueName) const
{
    for (i64 i = m_entries.size() - 1; 0 <= i; --i) {
        const auto it = m_entries[i].find(uniqueName);
        if (it == m_entries[i].end())
            continue;
        const bool wrongType = it->second.type != SymbolType::Func;
        const bool fromCurrentScope = i == m_entries.size() - 1;
        i32 args = 0;
        if (m_funcs.contains(uniqueName))
            args = m_funcs.at(uniqueName);
        return {args, true, wrongType, fromCurrentScope, it->second.hasLinkage};
    }
    return {false, false, false, false, false};
}

void SymbolTable::setArgs(const std::vector<std::string>& args)
{
    m_args = args;
}

void SymbolTable::clearArgs()
{
    m_args.clear();
}

void SymbolTable::addVarEntry(const std::string& name, const std::string& uniqueName, const bool hasLinkage)
{
    m_entries.back().insert(std::make_pair(name, Entry(uniqueName, SymbolType::Var, hasLinkage)));
}

void SymbolTable::addFuncEntry(const std::string& name, const i32 argsSize, const bool hasLinkage)
{
    m_entries.back().insert(std::make_pair(name, Entry(name, SymbolType::Func, hasLinkage)));
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
