#include "VarTable.hpp"
#include "TypeConversion.hpp"
#include "ASTUtils.hpp"

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

i32 VarTable::getStructuredAlignment(const Parsing::TypeBase* const type) const
{
    const auto structuredType = dynamic_cast<const Parsing::StructuredType*>(type);
    const auto it = entries.find(structuredType->identifier);
    if (it == entries.end())
        std::abort();
    return it->second.alignment;
}

i32 VarTable::getAlignment(const Parsing::TypeBase* const type) const
{
    if (isStructuredTypeBase(*type))
        return getStructuredAlignment(type);
    if (type->kind == Parsing::TypeBase::Kind::Pointer)
        return 8;
    if (type->kind == Parsing::TypeBase::Kind::Array) {
        const Parsing::TypeBase* innerType = Parsing::getArrayBaseType(*type);
        if (isStructuredType(innerType->type))
            return getStructuredAlignment(innerType);
        return getTypeSize(innerType->type);
    }
    return getTypeSize(type->type);
}

i64 VarTable::getStructuredSize(const Parsing::TypeBase* type) const
{
    const auto structuredType = dynamic_cast<const Parsing::StructuredType*>(type);
    const auto it = entries.find(structuredType->identifier);
    if (it == entries.end())
        std::abort();
    return it->second.size;
}

i64 VarTable::getSize(const Parsing::TypeBase* type) const
{
    if (isStructuredTypeBase(*type))
        return getStructuredSize(type);
    if (type->kind == Parsing::TypeBase::Kind::Pointer)
        return 8;
    if (type->kind == Parsing::TypeBase::Kind::Array)
        return Parsing::getArraySize(type);
    return getTypeSize(type->type);
}

void VarTable::addEntry(const Parsing::StructuredDecl& structuredDecl, std::vector<Error>& errors)
{
    i64 structSize = 0;
    i32 structuredAlignment = 1;
    std::vector<MemberEntry> members;
    std::unordered_map<std::string, MemberEntry> memberMap;
    for (const auto& member : structuredDecl.members) {
        if (member->type->type == Type::Void) {
            errors.emplace_back("Cannot have void type as structured member", member->location);
            return;
        }
        if (isStructuredTypeBase(*member->type) && !isDefined(member->identifier)) {
            errors.emplace_back("Cannot have undefined type as structured member", member->location);
            return;
        }
        if (member->type->kind == Parsing::TypeBase::Kind::Array) {
            const Parsing::TypeBase* innerType = Parsing::getArrayBaseType(*member->type);
            if (isStructuredTypeBase(*innerType) && !isDefined(member->identifier)) {
                errors.emplace_back
                ("Cannot have undefined type as structured member in array",
                    member->location);
                return;
            }
        }
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
    const i64 diff = structSize % memberAlignment;
    return structSize + memberAlignment - diff;
}