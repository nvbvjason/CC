#include "VarTable.hpp"

#include "ASTUtils.hpp"
#include "TypeConversion.hpp"

Parsing::TypeBase* VarTable::getMemberType(const std::string& structuredName, const std::string& memberName) const
{
    const auto it = entries.find(structuredName);
    if (it == entries.end())
        return nullptr;
    const auto itMember = it->second.memberMap.find(memberName);
    if (itMember == it->second.memberMap.end())
        return nullptr;
    return itMember->second.type.get();
}

i32 VarTable::getAlignment(const Parsing::TypeBase* const type) const
{
    if (isStructuredTypeBase(*type)) {
        const auto structuredType = dynamic_cast<const Parsing::StructuredType*>(type);
        const auto it = entries.find(structuredType->identifier);
        if (it == entries.end())
            std::abort();
        return it->second.alignment;
    }
    if (type->kind == Parsing::TypeBase::Kind::Pointer)
        return 8;
    if (type->kind == Parsing::TypeBase::Kind::Array) {
        const Type innerType = Parsing::getArrayType(type);
        return getTypeSize(innerType);
    }
    return getTypeSize(type->type);
}

i64 VarTable::getSize(const Parsing::TypeBase* type) const
{
    if (isStructuredTypeBase(*type)) {
        const auto structuredType = dynamic_cast<const Parsing::StructuredType*>(type);
        const auto it = entries.find(structuredType->identifier);
        if (it == entries.end())
            std::abort();
        return it->second.size;
    }
    if (type->kind == Parsing::TypeBase::Kind::Pointer)
        return 8;
    if (type->kind == Parsing::TypeBase::Kind::Array)
        return Parsing::getArraySize(type);
    return getTypeSize(type->type);
}

void VarTable::addEntry(const Parsing::StructuredDecl& structuredDecl)
{
    i64 structSize = 0;
    i32 structuredAlignment = 1;
    std::vector<MemberEntry> members;
    std::unordered_map<std::string, MemberEntry> memberMap;
    for (const auto& member : structuredDecl.members) {
        const i32 memberAlignment = getAlignment(member->type.get());
        const i64 memberOffset = structuredDecl.isUnion() ? 0 : roundUp(structSize, memberAlignment);
        members.emplace_back(
            member->identifier,
            Parsing::deepCopy(*member->type),
            memberOffset,
            structuredAlignment);
        memberMap.emplace(member->identifier, MemberEntry(
            member->identifier,
            Parsing::deepCopy(*member->type),
            memberOffset,
            structuredAlignment));
        structuredAlignment = std::max(structuredAlignment, memberAlignment);
        structSize = memberOffset + getSize(member->type.get());
    }
    structSize = roundUp(structSize, structuredAlignment);
    entries.emplace(structuredDecl.identifier, StructuredEntry(
        std::move(members),
        std::move(memberMap),
        structSize,
        structuredAlignment));
}

i64 roundUp(const i64 structSize, const i32 memberAlignment)
{
    const i32 diff = structSize % memberAlignment;
    return structSize + memberAlignment - diff;
}