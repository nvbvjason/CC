#pragma once

#include "ShortTypes.hpp"
#include "ASTParser.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct MemberEntry {
    std::string name;
    std::unique_ptr<Parsing::TypeBase> type;
    const i64 offset;
    const i32 size;
    MemberEntry(const std::string& name,
                std::unique_ptr<Parsing::TypeBase> type,
                const i64 offset,
                const i32 size)
        : name(name), type(std::move(type)), offset(offset), size(size) {}
};

struct StructuredEntry {
    std::vector<MemberEntry> members;
    std::unordered_map<std::string, MemberEntry> memberMap;
    const i64 size;
    const i32 alignment;
    StructuredEntry(std::vector<MemberEntry>&& members,
                    std::unordered_map<std::string, MemberEntry>&& memberMap,
                    const i64 size,
                    const i32 alignment)
        : members(std::move(members)), memberMap(std::move(memberMap)), size(size), alignment(alignment) {}
};

class VarTable {
    std::unordered_map<std::string, StructuredEntry> entries;
public:
    bool isDefined(const std::string& name) const { return entries.contains(name); }
    Parsing::TypeBase* getMemberType(const std::string& structuredName, const std::string& memberName) const;
    void emplaceMove(const std::string& name, StructuredEntry&& entry)
    {
        entries.emplace(name, std::move(entry));
    }
    bool hasMemberType(const std::string& structuredName, const std::string& memberName) const
    {
        return getMemberType(structuredName, memberName) != nullptr;
    }
    [[nodiscard]] i32 getAlignment(const Parsing::TypeBase* type) const;
    [[nodiscard]] i64 getSize(const Parsing::TypeBase* type) const;
    void addEntry(const Parsing::StructuredDecl& structuredDecl);
};

i64 roundUp(i64 structSize, i32 memberAlignment);