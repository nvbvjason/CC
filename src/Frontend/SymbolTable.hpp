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
        bool contains;
        bool wrongType;
        bool fromCurrentScope;
        bool hasLinkage;
        ReturnedFuncEntry(const i32 argSize,
                          const bool contains,
                          const bool wrongType,
                          const bool fromCurrentScope,
                          const bool hasLinkage)
            : argSize(argSize),
              contains(contains),
              wrongType(wrongType),
              fromCurrentScope(fromCurrentScope),
              hasLinkage(hasLinkage) {}
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
    std::string getUniqueName(const std::string& unique) const;
    void setArgs(const std::vector<std::string>& args);
    void clearArgs();
    void addVarEntry(const std::string& name, const std::string& uniqueName, bool hasLinkage);
    void addFuncEntry(const std::string& name, i32 argsSize, bool hasLinkage);
    void addScope();
    void removeScope();

    [[nodiscard]] i32 argSize(const std::string& funcName) const { return m_funcs.at(funcName); }
    [[nodiscard]] bool isInArgs(const std::string& name) const { return std::ranges::find(m_args, name) != m_args.end(); }
    [[nodiscard]] bool inFunc() const { return 1 < m_entries.size(); }
    [[nodiscard]] bool isFunc(const std::string& name) const;
};

#endif // CC_FRONTEND_SYMBOL_TABLE_HPP