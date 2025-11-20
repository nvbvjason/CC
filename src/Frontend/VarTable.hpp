#pragma once

#include "ShortTypes.hpp"
#include "ASTParser.hpp"
#include "Error.hpp"

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
    [[nodiscard]] bool isDefined(const std::string& name) const { return entries.contains(name); }
    [[nodiscard]] Parsing::TypeBase* getMemberType(
        const std::string& structuredName,
        const std::string& memberName) const;
    [[nodiscard]] const StructuredEntry* lookupEntry(const std::string& iden) const;

    bool hasMemberType(const std::string& structuredName, const std::string& memberName) const
    {
        return getMemberType(structuredName, memberName) != nullptr;
    }
    void addEntry(const Parsing::StructuredDecl& structuredDecl, std::vector<Error>& errors);
    [[nodiscard]] bool isInCompleteStructuredType(const Parsing::TypeBase& typeBase) const;
    [[nodiscard]] bool isPointerToInCompleteStructuredType(const Parsing::TypeBase& typeBase) const;

    [[nodiscard]] i32 getAlignment(const Parsing::TypeBase* type) const;
    [[nodiscard]] i64 getSize(const Parsing::TypeBase* type) const;
    [[nodiscard]] i64 getStructuredSize(const Parsing::TypeBase* type) const;
    [[nodiscard]] i32 getStructuredAlignment(const Parsing::TypeBase* type) const;
};

i64 roundUp(i64 structSize, i32 memberAlignment);