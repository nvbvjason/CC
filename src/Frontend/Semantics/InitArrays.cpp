#include "InitArrays.hpp"
#include "ASTBase.hpp"
#include "ASTTypes.hpp"
#include "DynCast.hpp"
#include "ASTDeepCopy.hpp"

#include <numeric>
#include <stack>

namespace Semantics {

struct Node {
    Parsing::Initializer* init;
    i64 at;
    Node() = delete;
    Node(Parsing::Initializer* init, const i64 at)
        : init(init), at{at} {}
};

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
    std::vector<std::unique_ptr<Parsing::Initializer>> staticInitializer;
    std::vector<i64> scaling = getScales(dimensions, size);
    i64 at = 0;
    std::stack<Node, std::vector<Node>> stack;
    stack.emplace(arrayInit, 0);
    do {
        auto[init, atCompound] = stack.top();
        stack.pop();
        switch (init->kind) {
            case Parsing::Initializer::Kind::Compound: {
                auto compoundInit = dynCast<Parsing::CompoundInitializer>(init);
                if (compoundInit->initializers.size() - 1 <= atCompound) {
                    const i64 dimension = dimensions[stack.size()];
                    staticInitializer.emplace_back(std::make_unique<Parsing::ZeroInitializer>(
                        dimension - atCompound));
                }
                else {
                    stack.emplace(init, atCompound + 1);
                    stack.emplace(compoundInit->initializers[0].get(), 0);
                }
                break;
            }
            case Parsing::Initializer::Kind::Single: {
                auto singleInit = dynCast<Parsing::SingleInitializer>(init);
                staticInitializer.emplace_back(std::make_unique<Parsing::SingleInitializer>(
                    Parsing::deepCopy(*singleInit->exp)));
                ++at;
                break;
            }
            default:
                std::abort();
        }
    } while (!stack.empty());
    if (at + 1 < size)
        staticInitializer.emplace_back(std::make_unique<Parsing::ZeroInitializer>(size - at - 1));
    auto newArrayInit = std::make_unique<Parsing::CompoundInitializer>(std::move(staticInitializer));
    array.init = std::move(newArrayInit);
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

std::vector<i64> getScales(const std::vector<i64>& dimensions, i64 size)
{
    std::vector<i64> scaling;
    for (const i64 dim : dimensions) {
        size /= dim;
        scaling.emplace_back(size);
    }
    return scaling;
}

} // Semantics