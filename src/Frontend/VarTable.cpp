#include "VarTable.hpp"
#include "TypeConversion.hpp"
#include "ASTUtils.hpp"
#include "DynCast.hpp"

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

const StructuredEntry* VarTable::lookupEntry(const std::string& iden) const
{
    const auto it = entries.find(iden);
    if (it == entries.end())
        return nullptr;
    return &it->second;
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

void VarTable::addEntry(const std::string& uniqueName,
                        const Parsing::StructuredDecl& structuredDecl,
                        std::vector<Error>& errors)
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
        if (isIncompleteTypeBase(*member->type)) {
            errors.emplace_back("Cannot use incomplete type in structured definition", member->location);
            return;
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
    entries.emplace(uniqueName, StructuredEntry(
        std::move(members),
        std::move(memberMap),
        structSize,
        structuredAlignment));
}

bool VarTable::isPointerToInCompleteStructuredType(const Parsing::TypeBase& typeBase) const
{
    if (typeBase.kind != Parsing::TypeBase::Kind::Pointer)
        return false;
    const auto pointerType = dynCast<const Parsing::PointerType>(&typeBase);
    return isInCompleteStructuredType(*pointerType->referenced);
}

bool VarTable::isInCompleteStructuredType(const Parsing::TypeBase& typeBase) const
{
    if (!isStructuredTypeBase(typeBase))
        return false;
    const auto structuredType = dynCast<const Parsing::StructuredType>(&typeBase);
    return !isDefined(structuredType->identifier);
}

i64 roundUp(const i64 structSize, const i32 memberAlignment)
{
    const i64 diff = structSize % memberAlignment;
    return structSize + memberAlignment - diff;
}

bool VarTable::isIncompleteTypeBase(const Parsing::TypeBase& typeBase) const
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