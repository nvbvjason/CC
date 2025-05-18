#pragma once

#ifndef CC_FRONTEND_SYMBOL_TABLE_HPP
#define CC_FRONTEND_SYMBOL_TABLE_HPP

#include "ShortTypes.hpp"

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

class SymbolTable {
public:
    struct ReturnedFuncEntry {
        i32 argSize;
        bool wrongType;
        bool contains;
        bool hasLinkage;
        ReturnedFuncEntry(const i32 argSize, const bool wrongType, const bool contains, const bool hasLinkage)
            : argSize(argSize), wrongType(wrongType), contains(contains), hasLinkage(hasLinkage) {}
    };
    struct ReturnedVarEntry {
        bool contains;
        bool inArgs;
        bool wrongType;
        bool fromCurrentScope;
        bool hasLinkage;
        ReturnedVarEntry(const bool contains,
                         const bool inArgs,
                         const bool wrongType,
                         const bool fromCurrentScope,
                         const bool hasLinkage)
            : contains(contains),
              inArgs(inArgs),
              wrongType(wrongType),
              fromCurrentScope(fromCurrentScope),
              hasLinkage(hasLinkage) {}
    };
private:
    enum class SymbolType {
        Var, Func
    };
    struct Entry {
        std::string uniqueName;
        SymbolType type;
        bool hasLinkage;
        Entry(std::string uniqueName, SymbolType type, const bool hasLinkage)
            :uniqueName(std::move(uniqueName)), type(type), hasLinkage(hasLinkage) {}
    };
    std::vector<std::unordered_map<std::string, Entry>> m_entries;
    std::unordered_map<std::string, i32> m_funcs;
    std::vector<std::string> m_args;
public:
    SymbolTable();
    [[nodiscard]] bool contains(const std::string& uniqueName) const;
    [[nodiscard]] ReturnedVarEntry lookupVar(const std::string& uniqueName) const;
    [[nodiscard]] ReturnedFuncEntry lookupFunc(const std::string& uniqueName) const;
    void setArgs(const std::vector<std::string>& args);
    void clearArgs();
    void addVarEntry(const std::string& name, const std::string& uniqueName, bool hasLinkage);
    void addFuncEntry(const std::string& name, i32 argsSize, bool hasLinkage);
    void addScope();
    void removeScope();

    [[nodiscard]] bool inFunction() const { return 1 < m_entries.size(); }
};

#endif // CC_FRONTEND_SYMBOL_TABLE_HPP