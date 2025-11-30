#include "TypeTable.hpp"
#include "TypeConversion.hpp"
#include "ASTUtils.hpp"
#include "DynCast.hpp"

void TypeTable::addEntry(const std::string& uniqueName,
                         const Parsing::StructuredDecl& structuredDecl,
                         std::vector<Error>& errors)
{
    i64 structSize = 0;
    i64 structuredAlignment = 1;
    std::vector<MemberEntry> members;
    std::unordered_map<std::string, MemberEntry> memberMap;
    if (uniqueName == "contains_struct_array.9.tmp") {
        structuredAlignment = 1;
    }
    for (const auto& member : structuredDecl.members) {
        if (member->type->type == Type::Void) {
            errors.emplace_back("Cannot have void type as structured member", member->location);
            return;
        }
        if (isIncompleteTypeBase(*member->type)) {
            errors.emplace_back("Cannot use incomplete type in structured definition", member->location);
            return;
        }
        const i64 memberAlignment = getAlignment(member->type.get());
        const i64 memberOffset = structuredDecl.isUnion() ? 0 : roundUp(structSize, memberAlignment);
        const i64 memberSize = getSize(member->type.get());
        emplaceMember(structuredAlignment, memberOffset, members, memberMap, member);
        structuredAlignment = std::max(structuredAlignment, memberAlignment);
        structSize = memberOffset + memberSize;
    }
    structSize = roundUp(structSize, structuredAlignment);
    entries.emplace(uniqueName, StructuredEntry(
        std::move(members),
        std::move(memberMap),
        structSize,
        structuredAlignment,
        structuredDecl.type));
}

i64 roundUp(const i64 structSize, const i64 memberAlignment)
{
    const i64 mod = structSize % memberAlignment;
    if (mod == 0)
        return structSize;
    const i64 diff = memberAlignment - mod;
    return structSize + diff;
}

bool TypeTable::isDefined(const Parsing::StructuredType& type) const
{
    const auto it = entries.find(type.identifier);
    if (it == entries.end())
        return false;
    return it->second.type == type.type;;
}

Parsing::TypeBase* TypeTable::getMemberType(const std::string& structuredName, const std::string& memberName) const
{
    const auto it = entries.find(structuredName);
    if (it == entries.end())
        return nullptr;
    const auto itMember = it->second.memberMap.find(memberName);
    if (itMember == it->second.memberMap.end())
        return nullptr;
    return itMember->second.type.get();
}

const StructuredEntry* TypeTable::getEntry(const std::string& iden) const
{
    const auto it = entries.find(iden);
    if (it == entries.end())
        return nullptr;
    return &it->second;
}

i64 TypeTable::getStructuredAlignment(const Parsing::TypeBase* const type) const
{
    const auto structuredType = dynCast<const Parsing::StructuredType>(type);
    const auto it = entries.find(structuredType->identifier);
    if (it == entries.end())
        std::abort();
    return it->second.alignment;
}

i64 TypeTable::getAlignment(const Parsing::TypeBase* const type) const
{
    if (isStructuredTypeBase(*type))
        return getStructuredAlignment(type);
    if (type->kind == Parsing::TypeBase::Kind::Pointer)
        return 8;
    if (type->kind == Parsing::TypeBase::Kind::Array) {
        const Parsing::TypeBase* innerType = Parsing::getArrayBaseType(*type);
        return getAlignment(innerType);
    }
    return getTypeSize(type->type);
}

i64 TypeTable::getStructuredSize(const Parsing::TypeBase* type) const
{
    const auto structuredType = dynamic_cast<const Parsing::StructuredType*>(type);
    const auto it = entries.find(structuredType->identifier);
    if (it == entries.end())
        std::abort();
    return it->second.size;
}

i64 TypeTable::getSize(const Parsing::TypeBase* type) const
{
    if (isStructuredTypeBase(*type))
        return getStructuredSize(type);
    if (type->kind == Parsing::TypeBase::Kind::Pointer)
        return 8;
    if (type->kind == Parsing::TypeBase::Kind::Array) {
        const i64 arrayLength = Parsing::getArrayLength(type);
        const Parsing::TypeBase* innerType = Parsing::getArrayBaseType(*type);
        const i64 typeSize = getSize(innerType);
        const i64 size = arrayLength * typeSize;
        return size;
    }
    return getTypeSize(type->type);
}

i64 TypeTable::getOffset(const std::string& structuredName, const std::string& memberName) const
{
    const auto it = entries.find(structuredName);
    if (it == entries.end())
        std::abort();
    const auto memberIt = it->second.memberMap.find(memberName);
    if (memberIt == it->second.memberMap.end())
        std::abort();
    return memberIt->second.offset;
}

bool TypeTable::isPointerToInCompleteStructuredType(const Parsing::TypeBase& typeBase) const
{
    if (typeBase.kind != Parsing::TypeBase::Kind::Pointer)
        return false;
    const auto pointerType = dynCast<const Parsing::PointerType>(&typeBase);
    return isInCompleteStructuredType(*pointerType->referenced);
}

bool TypeTable::isInCompleteStructuredType(const Parsing::TypeBase& typeBase) const
{
    if (!isStructuredTypeBase(typeBase))
        return false;
    const auto structuredType = dynCast<const Parsing::StructuredType>(&typeBase);
    return !isDefined(*structuredType);
}

bool TypeTable::isIncompleteTypeBase(const Parsing::TypeBase& typeBase) const
{
    const Parsing::TypeBase* travType = &typeBase;
    if (isInCompleteStructuredType(*travType))
        return true;
    while (travType->type == Type::Array || travType->type == Type::Pointer) {
        switch (travType->type) {
            case Type::Array: {
                const auto arrayType = dynCast<const Parsing::ArrayType>(travType);
                if (isInCompleteStructuredType(*arrayType->elementType))
                    return true;
                travType = arrayType->elementType.get();
                break;
            }
            case Type::Pointer: {
                const auto pointerType = dynCast<const Parsing::PointerType>(travType);
                travType = pointerType->referenced.get();
                break;
            }
            default:
                return false;
        }
    }
    return false;
}

void TypeTable::emplaceMember(
    i64 structuredAlignment,
    const i64 memberOffset,
    std::vector<MemberEntry>& members,
    std::unordered_map<std::string, MemberEntry>& memberMap,
    const std::unique_ptr<Parsing::MemberDecl>& member)
{
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
}