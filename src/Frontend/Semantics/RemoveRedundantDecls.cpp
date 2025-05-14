#include "RemoveRedundantDecls.hpp"

bool hasInternalLinkage(const Parsing::FunDecl& func)
{
    using Storage = Parsing::FunDecl::StorageClass;
    return func.storage == Storage::StaticGlobalTentative ||
           func.storage == Storage::StaticGlobalInitialized;
}

void Semantics::RemoveRedundantDecls::handleFunctions(std::unique_ptr<Parsing::Declaration>& decl)
{
    const auto funcDecl = static_cast<Parsing::FunDecl*>(decl.get());
    const auto it = functionsInternalLinkage.find(funcDecl->name);
    if (it == functionsInternalLinkage.end()) {
        const bool internalLinkage = hasInternalLinkage(*funcDecl);
        functionsInternalLinkage.insert(it, {funcDecl->name, internalLinkage});
        if (funcDecl->body == nullptr)
            decl = nullptr;
        return;
    }
    if (funcDecl->body == nullptr)
        decl = nullptr;
    else if (it->second)
        funcDecl->storage = Parsing::Declaration::StorageClass::StaticGlobalInitialized;
}

void Semantics::RemoveRedundantDecls::handleVarDecl(const std::unique_ptr<Parsing::Declaration>& decl)
{
    const auto varDecl = static_cast<Parsing::VarDecl*>(decl.get());
    auto it = varInternalLinkage.find(varDecl->name);
}

void Semantics::RemoveRedundantDecls::go(Parsing::Program& prog)
{
    using Kind = Parsing::Declaration::Kind;
    for (auto& decl : prog.declarations) {
        if (decl->kind == Kind::FuncDecl)
            handleFunctions(decl);
        if (decl->kind == Kind::VarDecl)
            handleVarDecl(decl);
    }
    std::erase_if(prog.declarations, [](const auto& ptr) { return ptr == nullptr; });
    functionsInternalLinkage.clear();
}
