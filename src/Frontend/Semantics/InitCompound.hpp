#pragma once

#include <stack>

#include "ASTParser.hpp"
#include "Error.hpp"

namespace Semantics {

class InitCompound {
    struct Node {
        Parsing::Initializer* init;
        const std::vector<i64> position;
        Node() = delete;
        Node(Parsing::Initializer* init, const std::vector<i64>& position)
            : init(init), position(position) {}
    };

    Parsing::VarDecl& varDecl;
    std::vector<Error>& errors;
    std::vector<i64> dimensions;
    std::stack<Node, std::vector<Node>> stack;
public:
    explicit InitCompound(Parsing::VarDecl& varDecl, std::vector<Error>& errors);
    void initCharacterArray(const Parsing::SingleInitializer& singleInit,
                            const Parsing::ArrayType& arrayType) const;
    void initArray();
    void initString(Type innerArrayType, Parsing::Initializer* init) const;

private:
    void createInitsWithPositionsSingle(Type innerArrayType,
                                        std::vector<std::unique_ptr<Parsing::Initializer>>& staticInitializer,
                                        std::vector<std::vector<i64>>& emplacedPositions,
                                        const std::vector<i64>& position,
                                        Parsing::SingleInitializer& singleInit) const;
    void createInitsWithPositionsCompound(
        const std::vector<i64>& position,
        const Parsing::CompoundInitializer& compoundInit);
    std::tuple<std::vector<std::unique_ptr<Parsing::Initializer>>, std::vector<std::vector<i64>>>
    flattenWithPositions(Type innerArrayType,
                         Parsing::Initializer* arrayInit);
    std::vector<std::unique_ptr<Parsing::Initializer>> getZeroInits(
        const std::vector<std::unique_ptr<Parsing::Initializer>>& staticInitializer,
        const std::vector<std::vector<i64>>& emplacedPositions);
    [[nodiscard]] i64 getDistance(
        const std::vector<i64>& positionBefore,
        const std::vector<i64>& position
    ) const;
    [[nodiscard]] i64 getPosition(const std::vector<i64>& position) const;
};

void initStringWithI32(const std::string& value,
                       std::vector<std::unique_ptr<Parsing::Initializer>>& stringInitializer);
std::unique_ptr<Parsing::Initializer> emplaceNewSingleInit(
    Type innerArrayType, Parsing::SingleInitializer& singleInit);
bool isZeroSingleInit(const Parsing::Initializer& init);
std::vector<i64> getDimensions(const Parsing::VarDecl& array);
} // Semantics