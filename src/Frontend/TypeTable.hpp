#pragma once

#include "ShortTypes.hpp"
#include "ASTParser.hpp"
#include "Error.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct MemberEntry {
    std::string name;
    std::unique_ptr<Parsing::TypeBase> type;
    const i64 offset;
    const i64 size;
    MemberEntry(std::string name,
                std::unique_ptr<Parsing::TypeBase> type,
                const i64 offset,
                const i64 size)
        : name(std::move(name)), type(std::move(type)), offset(offset), size(size) {}
};

struct StructuredEntry {
    std::vector<MemberEntry> members;
    std::unordered_map<std::string, MemberEntry> memberMap;
    const i64 size;
    const i64 alignment;
    const Type type;
    StructuredEntry(std::vector<MemberEntry>&& members,
                    std::unordered_map<std::string, MemberEntry>&& memberMap,
                    const i64 size,
                    const i64 alignment,
                    const Type type)
        : members(std::move(members)),
          memberMap(std::move(memberMap)),
          size(size),
          alignment(alignment),
          type(type) {}
};

class TypeTable {
public:
    std::unordered_map<std::string, StructuredEntry> entries;
    [[nodiscard]] bool isDefined(const Parsing::StructuredType& type) const;
    [[nodiscard]] Parsing::TypeBase* getMemberType(
        const std::string& structuredName,
        const std::string& memberName) const;
    [[nodiscard]] const StructuredEntry* getEntry(const std::string& iden) const;

    bool hasMemberType(const std::string& structuredName, const std::string& memberName) const
    {
        return getMemberType(structuredName, memberName) != nullptr;
    }
    void addEntry(const std::string& uniqueName,
                  const Parsing::StructuredDecl& structuredDecl,
                  std::vector<Error>& errors);
    [[nodiscard]] i64 getOffset(const std::string& structuredName, const std::string& memberName) const;
    [[nodiscard]] bool isInCompleteStructuredType(const Parsing::TypeBase& typeBase) const;
    [[nodiscard]] bool isPointerToInCompleteStructuredType(const Parsing::TypeBase& typeBase) const;

    [[nodiscard]] i64 getAlignment(const Parsing::TypeBase* type) const;
    [[nodiscard]] i64 getSize(const Parsing::TypeBase* type) const;
    [[nodiscard]] i64 getStructuredSize(const Parsing::TypeBase* type) const;
    [[nodiscard]] i64 getStructuredAlignment(const Parsing::TypeBase* type) const;
    [[nodiscard]] bool isIncompleteTypeBase(const Parsing::TypeBase& typeBase) const;
};

i64 roundUp(i64 structSize, i64 memberAlignment);