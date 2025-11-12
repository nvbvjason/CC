#pragma once

#include "ShortTypes.hpp"
#include "ASTParser.hpp"

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

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
        void clear(State flag) noexcept
        {
            flags = static_cast<State>(static_cast<u32>(flags) & ~static_cast<u32>(flag));
        }
        void clearAll() noexcept
        {
            flags = State::None;
        }
        [[nodiscard]] bool contains() const noexcept { return isSet(State::Contains); }
        [[nodiscard]] bool isFromCurrentScope() const noexcept { return isSet(State::FromCurrentScope); }
        [[nodiscard]] bool hasInternalLinkage() const noexcept { return isSet(State::InternalLinkage); }
        [[nodiscard]] bool hasExternalLinkage() const noexcept { return isSet(State::ExternalLinkage); }
        [[nodiscard]] bool isGlobal() const noexcept { return isSet(State::Global); }
        [[nodiscard]] bool isDefined() const noexcept { return isSet(State::Defined); }
        [[nodiscard]] bool isInArgs() const noexcept { return isSet(State::InArgs); }
    private:
        [[nodiscard]] bool isSet(State flag) const noexcept
        {
            return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
        }
    };
    struct ReturnedEntry : FlagBase<ReturnedEntry>  {
        std::unique_ptr<Parsing::TypeBase> typeBase;
        ReturnedEntry(std::unique_ptr<Parsing::TypeBase>&& t,
                         const bool contains,
                         const bool inArgs,
                         const bool fromCurrentScope,
                         const bool internal,
                         const bool external,
                         const bool global,
                         const bool defined)
        {
            typeBase = std::move(t);
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
        std::unique_ptr<Parsing::TypeBase> varType;
        Entry(std::string uniqueName,
              std::unique_ptr<Parsing::TypeBase>&& typeBase,
              const bool internal,
              const bool external,
              const bool global,
              const bool defined)
            : uniqueName(std::move(uniqueName))
        {
            varType = std::move(typeBase);
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
    std::vector<std::unique_ptr<Parsing::TypeBase>> m_argTypes;
public:
    SymbolTable();
    [[nodiscard]] bool contains(const std::string& name) const;
    [[nodiscard]] ReturnedEntry lookup(const std::string& uniqueName) const;
    std::string getUniqueName(const std::string& unique) const;
    void setArgs(Parsing::FuncDeclaration& funDecl);
    void clearArgs();
    void addEntry(const std::string& name,
                  const std::string& uniqueName,
                  const Parsing::TypeBase& typeBase,
                  bool internal, bool external, bool global, bool defined);
    void addScope();
    void removeScope();

    [[nodiscard]] i32 argSize(const std::string& funcName) const { return m_funcs.at(funcName); }
    [[nodiscard]] bool isInArgs(const std::string& name) const { return std::ranges::find(m_args, name) != m_args.end(); }
    [[nodiscard]] bool inFunc() const { return 1 < m_entries.size(); }
    [[nodiscard]] bool isFunc(const std::string& name) const;
};