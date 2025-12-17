#include "SymbolTable.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTUtils.hpp"

#include <cassert>

SymbolTable::SymbolTable()
{
    addScope();
}

bool SymbolTable::contains(const std::string& name) const
{
    for (size_t i = entries.size(); 0 < i--;)
        if (entries[i].contains(name))
            return true;
    return false;
}

SymbolTable::ReturnedEntry SymbolTable::lookupEntry(const std::string& uniqueName) const
{
    const bool inArgs = isInArgs(uniqueName);
    if (inArgs) {
        for (i32 i = 0; i < argTypes.size(); ++i)
            if (uniqueName == args[i])
                return {Parsing::deepCopy(*argTypes[i]), true, true, false, false, false, false, false};
    }
    for (size_t i = entries.size(); 0 < i--;) {
        const auto it = entries[i].find(uniqueName);
        if (it == entries[i].end())
            continue;
        const bool fromCurrentScope = i == entries.size() - 1;
        const bool internal = it->second.hasInternalLinkage();
        const bool external = it->second.hasExternalLinkage();
        const bool global = it->second.isGlobal();
        const bool defined = it->second.isDefined();
        return {Parsing::deepCopy(*it->second.varType), true, inArgs, fromCurrentScope, internal, external, global, defined};
    }
    return {nullptr, false, inArgs, false, false, false, false, false};
}

SymbolTable::ReturnedStructuredEntry SymbolTable::lookupStructuredEntry(const std::string& name) const
{
    for (size_t i = structuredEntries.size(); 0 < i--;) {
        const auto it = structuredEntries[i].find(name);
        if (it == structuredEntries[i].end())
            continue;
        const bool fromCurrentScope = i == structuredEntries.size() - 1;
        const bool defined = it->second.isDefined();
        const std::string uniqueName = it->second.uniqueName;
        return {Parsing::deepCopy(*it->second.varType), uniqueName, true, fromCurrentScope, defined};
    }
    return {nullptr, "", false, false, false};
}

std::string SymbolTable::getUniqueName(const std::string& unique) const
{
    for (size_t i = entries.size(); 0 < i--;) {
        const auto it = entries[i].find(unique);
        if (it == entries[i].end())
            continue;
        return it->second.uniqueName;
    }
    std::abort();
}

void SymbolTable::setArgs(const Parsing::FuncDecl& funDecl)
{
    args = funDecl.params;
    argTypes.clear();
    const auto funcType = dynCast<Parsing::FuncType>(funDecl.type.get());
    for (auto& param : funcType->params) {
        if (param->type == Type::Array) {
            const auto arrayType = dynCast<Parsing::ArrayType>(param.get());
            auto pointerType = std::make_unique<Parsing::PointerType>(
                Parsing::deepCopy(*arrayType->elementType));
            param = std::move(pointerType);
        }
        argTypes.emplace_back(Parsing::deepCopy(*param));
    }
}

void SymbolTable::clearArgs()
{
    args.clear();
}

void SymbolTable::addEntry(const std::string& name,
                           const std::string& uniqueName,
                           const Parsing::TypeBase& typeBase,
                           const bool internal,
                           const bool external,
                           const bool global,
                           const bool defined)
{
    entries.back().insert_or_assign(name, Entry(
        uniqueName, Parsing::deepCopy(typeBase),
        internal, external, global, defined)
    );
}

void SymbolTable::addStructuredEntry(const std::string& name,
                                     const std::string& uniqueName,
                                     const Parsing::TypeBase& typeBase,
                                     const bool defined)
{
    structuredEntries.back().insert_or_assign(name, StructuredEntry(
        uniqueName,
        Parsing::deepCopy(typeBase),
        defined)
    );
}

void SymbolTable::addScope()
{
    entries.emplace_back();
    structuredEntries.emplace_back();
}

void SymbolTable::removeScope()
{
    entries.pop_back();
    structuredEntries.pop_back();
}

bool SymbolTable::isFunc(const std::string& name) const
{
    const auto it = entries.front().find(name);
    if (it == entries.front().end())
        return false;
    return it->second.varType->type == Type::Function;
}