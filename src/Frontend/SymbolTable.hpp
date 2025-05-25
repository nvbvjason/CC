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
    enum class State : u16 {
        None                = 0,
        Contains            = 1 << 0,
        CorrectType         = 1 << 1,
        FromCurrentScope    = 1 << 2,
        HasLinkage          = 1 << 3,
        Global              = 1 << 4,
        Defined             = 1 << 5,
        InArgs              = 1 << 6,

        Init_Mask           = 0b11 << 14,
        Init_HasInitializer = 0b01 << 14,
        Init_Tentative      = 0b10 << 14,
        Init_NoInitializer  = 0b11 << 14,
    };
    template <typename Derived>
    struct FlagBase {
        State flags = State::None;
        void set(State flag) noexcept
        {
            flags = static_cast<State>(static_cast<u32>(flags) | static_cast<u32>(flag));
        }
        [[nodiscard]] bool isSet(State flag) const noexcept
        {
            return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
        }
        [[nodiscard]] bool isInitSet(State flag) const noexcept
        {
            return (static_cast<u32>(flags) & static_cast<u32>(State::Init_Mask)) == static_cast<u32>(flag);
        }
        void clear(State flag) noexcept
        {
            flags = static_cast<State>(static_cast<u32>(flags) & ~static_cast<u32>(flag));
        }
        void clearAll() noexcept
        {
            flags = State::None;
        }
        void setInit(State flag)
        {
            flags = static_cast<State>(static_cast<u32>(flags) & ~static_cast<u32>(State::Init_Mask)
                | (static_cast<u32>(flag) & static_cast<u32>(State::Init_Mask)));
        }
        [[nodiscard]] State getInit() const
        {
            return static_cast<State>(static_cast<u32>(flags) & static_cast<u32>(State::Init_Mask));
        }
    };
    struct ReturnedFuncEntry : FlagBase<ReturnedFuncEntry>  {
        i32 argSize;
        ReturnedFuncEntry(const i32 argSize,
                          const bool contains,
                          const bool correctType,
                          const bool fromCurrentScope,
                          const bool hasLinkage,
                          const bool isGlobal,
                          const bool defined)
            : argSize(argSize)
        {
            if (contains)
                set(State::Contains);
            if (correctType)
                set(State::CorrectType);
            if (fromCurrentScope)
                set(State::FromCurrentScope);
            if (hasLinkage)
                set(State::HasLinkage);
            if (isGlobal)
                set(State::Global);
            if (defined)
                set(State::Defined);
        }
    };
    struct ReturnedVarEntry : FlagBase<ReturnedVarEntry>  {
        ReturnedVarEntry(const bool contains,
                         const bool inArgs,
                         const bool correctType,
                         const bool fromCurrentScope,
                         const bool hasLinkage,
                         const bool isGlobal,
                         const bool defined,
                         const State initState)
        {
            if (contains)
                set(State::Contains);
            if (correctType)
                set(State::CorrectType);
            if (inArgs)
                set(State::InArgs);
            if (fromCurrentScope)
                set(State::FromCurrentScope);
            if (hasLinkage)
                set(State::HasLinkage);
            if (isGlobal)
                set(State::Global);
            if (defined)
                set(State::Defined);
            setInit(initState);
        }
    };
private:
    enum class SymbolType  {
        Var, Func
    };
    struct Entry : FlagBase<Entry>  {
        std::string uniqueName;
        State returnFlag;
        SymbolType type;
        Entry(std::string uniqueName,
              const SymbolType type,
              const bool hasLinkage,
              const bool isGlobal,
              const bool defined)
            :uniqueName(std::move(uniqueName)), type(type)
        {
            if (hasLinkage)
                set(State::HasLinkage);
            if (isGlobal)
                set(State::Global);
            if (defined)
                set(State::Defined);
        }
        Entry(std::string uniqueName,
              const SymbolType type,
              const bool hasLinkage,
              const bool isGlobal,
              const bool defined,
              const State initState) : Entry(std::move(uniqueName), type, hasLinkage, isGlobal, defined)
        {
            set(initState);
        }
    };
    std::vector<std::unordered_map<std::string, Entry>> m_entries;
    std::unordered_map<std::string, i32> m_funcs;
    std::vector<std::string> m_args;
public:
    SymbolTable();
    [[nodiscard]] bool contains(const std::string& name) const;
    [[nodiscard]] ReturnedVarEntry lookupVar(const std::string& uniqueName) const;
    [[nodiscard]] ReturnedFuncEntry lookupFunc(const std::string& uniqueName) const;
    std::string getUniqueName(const std::string& unique) const;
    void setArgs(const std::vector<std::string>& args);
    void clearArgs();
    void addVarEntry(const std::string& name,
                     const std::string& uniqueName,
                     bool hasLinkage, bool isGlobal, bool defined,
                     State initState);
    void addFuncEntry(const std::string& name, i32 argsSize, bool hasLinkage, bool global, bool defined);
    void addScope();
    void removeScope();

    [[nodiscard]] i32 argSize(const std::string& funcName) const { return m_funcs.at(funcName); }
    [[nodiscard]] bool isInArgs(const std::string& name) const { return std::ranges::find(m_args, name) != m_args.end(); }
    [[nodiscard]] bool inFunc() const { return 1 < m_entries.size(); }
    [[nodiscard]] bool isFunc(const std::string& name) const;
};

#endif // CC_FRONTEND_SYMBOL_TABLE_HPP