#include "InitArrays.hpp"
#include "ASTBase.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"

#include <numeric>

namespace Semantics {

void initArrays(std::vector<Parsing::VarDecl*>& arrays)
{
    for (auto& varDecl : arrays)
        initArray(*varDecl);
}

void initArray(Parsing::VarDecl& array)
{
    std::vector<i64> dimensions = getDimensions(array);
    const i64 size = std::accumulate(dimensions.begin(), dimensions.end(), i64{1}, std::multiplies<>{});
    auto arrayInit = array.init.get();
    auto compoundInit = dynCast<Parsing::CompoundInitializer>(arrayInit);
    std::vector<std::unique_ptr<Parsing::Initializer>> staticInitializer;
    i64 at = 0;

}

std::vector<i64> getDimensions(const Parsing::VarDecl& array)
{
    std::vector<i64> dimensions;
    Parsing::TypeBase* type = array.type.get();
    do {
        const auto arrayType = dynCast<Parsing::ArrayType>(type);
        dimensions.emplace_back(arrayType->size);
        type = arrayType->elementType.get();
    } while (type->kind == Parsing::TypeBase::Kind::Array);
    return dimensions;
}

} // Semantics