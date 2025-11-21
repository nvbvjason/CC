#pragma once

#include "ASTBase.hpp"
#include "ASTUtils.hpp"

#include <memory>
#include <vector>

namespace Semantics {

struct FuncEntry {
    std::vector<std::unique_ptr<Parsing::TypeBase>> paramTypes;
    std::unique_ptr<Parsing::TypeBase> returnType;
    Parsing::Declaration::StorageClass storage;
    bool defined;
    FuncEntry(const std::vector<std::unique_ptr<Parsing::TypeBase>>& params,
              std::unique_ptr<Parsing::TypeBase>&& returnType,
              const Parsing::Declaration::StorageClass storage,
              const bool defined)
        : returnType(std::move(returnType)), storage(storage), defined(defined)
    {
        for (const auto& paramType : params)
            paramTypes.emplace_back(std::move(Parsing::deepCopy(*paramType)));
    }
};

}