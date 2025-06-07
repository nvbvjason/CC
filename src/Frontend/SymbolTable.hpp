#pragma once

#ifndef CC_FRONTEND_SYMBOL_TABLE_HPP
#define CC_FRONTEND_SYMBOL_TABLE_HPP

#include "ShortTypes.hpp"
#include "Types/Type.hpp"

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

#include "ASTParser.hpp"

class SymbolTable {
public:
    enum class State : u16 {
        None                = 0,
        Contains            = 1 << 0,
        FromCurrentScope    = 1 << 1,
        InternalLinkage     = 1 << 2,
        ExternalLinkage     = 1 << 3,
        Global              = 1 << 4,
        Defined             = 1 << 5,
        InArgs              = 1 << 6,
    };
    template <typename>
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
        void clear(State flag) noexcept
        {
            flags = static_cast<State>(static_cast<u32>(flags) & ~static_cast<u32>(flag));
        }
        void clearAll() noexcept
        {
            flags = State::None;
        }
    };
    struct ReturnedFuncEntry : FlagBase<ReturnedFuncEntry>  {
        Type type;
        i32 argSize;
        ReturnedFuncEntry(const Type type,
                          const i32 argSize,
                          const bool contains,
                          const bool fromCurrentScope,
                          const bool internal,
                          const bool external,
                          const bool isGlobal,
                          const bool defined)
            : type(type), argSize(argSize)
        {
            if (contains)
                set(State::Contains);
            if (fromCurrentScope)
                set(State::FromCurrentScope);
            if (internal)
                set(State::InternalLinkage);
            if (external)
                set(State::ExternalLinkage);
            if (isGlobal)
                set(State::Global);
            if (defined)
                set(State::Defined);
        }
    };
    struct ReturnedVarEntry : FlagBase<ReturnedVarEntry>  {
        Type type;
        ReturnedVarEntry(const Type t,
                         const bool contains,
                         const bool inArgs,
                         const bool fromCurrentScope,
                         const bool internal,
                         const bool external,
                         const bool global,
                         const bool defined)
        {
            type = t;
            if (contains)
                set(State::Contains);
            if (inArgs)
                set(State::InArgs);
            if (fromCurrentScope)
                set(State::FromCurrentScope);
            if (internal)
                set(State::InternalLinkage);
            if (external)
                set(State::ExternalLinkage);
            if (global)
                set(State::Global);
            if (defined)
                set(State::Defined);
        }
    };
private:
    struct Entry : FlagBase<Entry>  {
        std::string uniqueName;
        State returnFlag = State::None;
        Type type;
        Entry(std::string uniqueName,
              const Type type,
              const bool internal,
              const bool external,
              const bool global,
              const bool defined)
            :uniqueName(std::move(uniqueName)), type(type)
        {
            if (internal)
                set(State::InternalLinkage);
            if (external)
                set(State::ExternalLinkage);
            if (global)
                set(State::Global);
            if (defined)
                set(State::Defined);
        }
    };
    std::vector<std::unordered_map<std::string, Entry>> m_entries;
    std::unordered_map<std::string, i32> m_funcs;
    std::vector<std::string> m_args;
    std::vector<Type> m_argTypes;
public:
    SymbolTable();
    [[nodiscard]] bool contains(const std::string& name) const;
    [[nodiscard]] ReturnedVarEntry lookupVar(const std::string& uniqueName) const;
    [[nodiscard]] ReturnedFuncEntry lookupFunc(const std::string& uniqueName) const;
    std::string getUniqueName(const std::string& unique) const;
    void setArgs(const Parsing::FunDecl& funDecl);
    void clearArgs();
    void addVarEntry(const std::string& name,
                     const std::string& uniqueName,
                     Type type,
                     bool internal, bool external, bool global, bool defined);
    void addFuncEntry(const std::string& name, i32 argsSize, bool internal, bool external, bool global, bool defined);
    void addScope();
    void removeScope();

    [[nodiscard]] i32 argSize(const std::string& funcName) const { return m_funcs.at(funcName); }
    [[nodiscard]] bool isInArgs(const std::string& name) const { return std::ranges::find(m_args, name) != m_args.end(); }
    [[nodiscard]] bool inFunc() const { return 1 < m_entries.size(); }
    [[nodiscard]] bool isFunc(const std::string& name) const;
};

#endif // CC_FRONTEND_SYMBOL_TABLE_HPP