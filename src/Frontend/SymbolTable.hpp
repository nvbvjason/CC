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
    enum class ReturnFlag : u32 {
        None             = 1 << 0,
        Contains         = 1 << 1,
        CorrectType      = 1 << 2,
        FromCurrentScope = 1 << 3,
        HasLinkage       = 1 << 4,
        IsGlobal         = 1 << 5,
        Defined          = 1 << 6,
        Tentative        = 1 << 7,
        HasInitializer   = 1 << 8,
        InArgs           = 1 << 9,
    };
template <typename Derived>
struct FlagBase {
    ReturnFlag flags = ReturnFlag::None;
    void set(ReturnFlag flag) noexcept
    {
        flags = static_cast<ReturnFlag>(static_cast<u32>(flags) | static_cast<u32>(flag));
    }
    [[nodiscard]] bool isSet(ReturnFlag flag) const noexcept
    {
        return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
    }
    void clear(ReturnFlag flag) noexcept
    {
        flags = static_cast<ReturnFlag>(static_cast<u32>(flags) & ~static_cast<u32>(flag));
    }
    [[nodiscard]] bool allSet(ReturnFlag mask) const noexcept
    {
        return (static_cast<u32>(flags) & static_cast<u32>(mask)) == static_cast<u32>(mask);
    }
};
    struct ReturnedFuncEntry : FlagBase<ReturnedFuncEntry>  {
        i32 argSize;
        ReturnedFuncEntry(const i32 argSize,
                          const bool contains,
                          const bool correctType,
                          const bool fromCurrentScope,
                          const bool hasLinkage,
                          const bool isGlobal)
            : argSize(argSize)
        {
            if (contains)
                set(ReturnFlag::Contains);
            if (correctType)
                set(ReturnFlag::CorrectType);
            if (fromCurrentScope)
                set(ReturnFlag::FromCurrentScope);
            if (hasLinkage)
                set(ReturnFlag::HasLinkage);
            if (isGlobal)
                set(ReturnFlag::IsGlobal);
        }
    };
    struct ReturnedVarEntry : FlagBase<ReturnedVarEntry>  {
        ReturnedVarEntry(const bool contains,
                         const bool inArgs,
                         const bool correctType,
                         const bool fromCurrentScope,
                         const bool hasLinkage)
        {
            if (contains)
                set(ReturnFlag::Contains);
            if (correctType)
                set(ReturnFlag::CorrectType);
            if (inArgs)
                set(ReturnFlag::InArgs);
            if (fromCurrentScope)
                set(ReturnFlag::FromCurrentScope);
            if (hasLinkage)
                set(ReturnFlag::HasLinkage);
        }
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