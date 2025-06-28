#include "Declarator.hpp"

#include "ASTTypes.hpp"

namespace Parsing {
std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>> declaratorProcess(
    std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase)
{
    switch (declarator->kind) {
        case Declarator::Kind::Array:
            return declaratorArrayProcess(std::move(declarator), std::move(typeBase));
        case Declarator::Kind::Identifier:
            return declaratorIdentifierProcess(std::move(declarator), std::move(typeBase));
        case Declarator::Kind::Pointer:
            return declaratorPointerProcess(std::move(declarator), std::move(typeBase));
        case Declarator::Kind::Function:
            return declaratorFunctionProcess(std::move(declarator), std::move(typeBase));
        default:
            return {"", nullptr, std::vector<std::string>()};
    }
    std::abort();
}

std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>> declaratorFunctionProcess(
    std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase)
{
    const auto funcDecl = static_cast<FunctionDeclarator*>(declarator.get());
    if (funcDecl->declarator->kind != Declarator::Kind::Identifier)
        return {};
    std::vector<std::unique_ptr<TypeBase>> paramTypes;
    std::vector<std::string> params;
    for (ParamInfo& param : funcDecl->params) {
        auto [iden, typeBase, _] =
                declaratorProcess(std::move(param.declarator), std::move(param.type));
        if (typeBase->kind == Type::Function)
            return {};
        params.emplace_back(std::move(iden));
        paramTypes.emplace_back(std::move(typeBase));
    }
    const auto idenDecl = static_cast<IdentifierDeclarator*>(funcDecl->declarator.get());
    auto funcType = std::make_unique<FuncType>(std::move(typeBase), std::move(paramTypes));
    return std::make_tuple(std::move(idenDecl->identifier), std::move(funcType), std::move(params));
}

std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>> declaratorArrayProcess(
    std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase)
{
    const auto arrayDeclarator = static_cast<ArrayDeclarator*>(declarator.get());
    auto array = std::make_unique<ArrayType>(std::move(typeBase), arrayDeclarator->size);
    return declaratorProcess(std::move(arrayDeclarator->declarator), std::move(array));
}

std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>> declaratorPointerProcess(
        std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase)
{
    auto derivedType = std::make_unique<PointerType>(std::move(typeBase));
    const auto pointerDecl = static_cast<PointerDeclarator*>(declarator.get());
    return declaratorProcess(std::move(pointerDecl->inner), std::move(derivedType));
}

std::tuple<std::string, std::unique_ptr<TypeBase>, std::vector<std::string>> declaratorIdentifierProcess(
        std::unique_ptr<Declarator>&& declarator, std::unique_ptr<TypeBase>&& typeBase)
{
    const auto iden = static_cast<IdentifierDeclarator*>(declarator.get());
    return std::make_tuple(std::move(iden->identifier), std::move(typeBase), std::move(std::vector<std::string>()));
}

std::unique_ptr<TypeBase> abstractDeclaratorProcess(
    std::unique_ptr<AbstractDeclarator>&& abstractDeclarator, std::unique_ptr<TypeBase>&& typeBase)
{
    switch (abstractDeclarator->kind) {
        case AbstractDeclarator::Kind::Array:
            return abstractDeclaratorArrayProcess(abstractDeclarator, typeBase);
        case AbstractDeclarator::Kind::Pointer:
            return abstarctDeclaratorPointerProcess(abstractDeclarator, typeBase);
        case AbstractDeclarator::Kind::Base:
            return typeBase;
        default:
            return nullptr;
    }
    std::abort();
}

std::unique_ptr<TypeBase> abstarctDeclaratorPointerProcess(
    std::unique_ptr<AbstractDeclarator>& abstractDeclarator, std::unique_ptr<TypeBase>& typeBase)
{
    typeBase = std::make_unique<PointerType>(std::move(typeBase));
    const auto inner = static_cast<AbstractPointer*>(abstractDeclarator.get());
    abstractDeclarator = std::move(inner->inner);
    return abstractDeclaratorProcess(std::move(abstractDeclarator), std::move(typeBase));
}

std::unique_ptr<TypeBase> abstractDeclaratorArrayProcess(
    std::unique_ptr<AbstractDeclarator>& abstractDeclarator, std::unique_ptr<TypeBase>& typeBase)
{
    auto abstractArray = static_cast<AbstractArray*>(abstractDeclarator.get());
    typeBase = std::make_unique<ArrayType>(std::move(typeBase), abstractArray->size);
    abstractDeclarator = std::move(abstractArray->inner);
    return abstractDeclaratorProcess(std::move(abstractDeclarator), std::move(typeBase));
}
} // Parsing