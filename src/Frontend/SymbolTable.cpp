#include "SymbolTable.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTDeepCopy.hpp"

#include <cassert>

SymbolTable::SymbolTable()
{
    addScope();
}

bool SymbolTable::contains(const std::string& name) const
{
    for (size_t i = m_entries.size(); 0 < i--;)
        if (m_entries[i].contains(name))
            return true;
    return false;
}

SymbolTable::ReturnedEntry SymbolTable::lookup(const std::string& uniqueName) const
{
    const bool inArgs = isInArgs(uniqueName);
    if (inArgs) {
        for (i32 i = 0; i < m_argTypes.size(); ++i)
            if (uniqueName == m_args[i])
                return {Parsing::deepCopy(*m_argTypes[i]), true, true, false, false, false, false, false};
    }
    for (size_t i = m_entries.size(); 0 < i--;) {
        const auto it = m_entries[i].find(uniqueName);
        if (it == m_entries[i].end())
            continue;
        const bool fromCurrentScope = i == m_entries.size() - 1;
        const bool internal = it->second.hasInternalLinkage();
        const bool external = it->second.hasExternalLinkage();
        const bool global = it->second.isGlobal();
        const bool defined = it->second.isDefined();
        return {Parsing::deepCopy(*it->second.varType), true, inArgs, fromCurrentScope, internal, external, global, defined};
    }
    return {nullptr, false, inArgs, false, false, false, false, false};
}

std::string SymbolTable::getUniqueName(const std::string& unique) const
{
    for (size_t i = m_entries.size(); 0 < i--;) {
        const auto it = m_entries[i].find(unique);
        if (it == m_entries[i].end())
            continue;
        return it->second.uniqueName;
    }
    assert(false && "Should always get called after contains never happen in SymbolTable::getUniqueName");
}

void SymbolTable::setArgs(const Parsing::FuncDeclaration& funDecl)
{
    m_args = funDecl.params;
    m_argTypes.clear();
    const auto funcType = dynCast<const Parsing::FuncType>(funDecl.type.get());
    for (const std::unique_ptr<Parsing::TypeBase>& param : funcType->params)
        m_argTypes.emplace_back(Parsing::deepCopy(*param));
}

void SymbolTable::clearArgs()
{
    m_args.clear();
}

void SymbolTable::addEntry(const std::string& name,
                           const std::string& uniqueName,
                           const Parsing::TypeBase& typeBase,
                           const bool internal,
                           const bool external,
                           const bool global,
                           const bool defined)
{

    m_entries.back().insert_or_assign(name,Entry(
        uniqueName, Parsing::deepCopy(typeBase),
        internal, external, global, defined)
        );
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
    return it->second.varType->type == Type::Function;
}