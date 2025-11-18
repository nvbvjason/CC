#pragma once

#include "ASTParser.hpp"
#include "Error.hpp"

namespace Semantics {

void initCharacterArray(Parsing::VarDecl& varDecl,
                        const Parsing::SingleInitializer& singleInit,
                        const Parsing::ArrayType& arrayType);
void initArray(Parsing::VarDecl& array, std::vector<Error>& errors);
std::vector<i64> getDimensions(const Parsing::VarDecl& array);
void createInitsWithPositionsSingle(Type innerArrayType,
                                    std::vector<Error>& errors,
                                    const std::vector<i64>& dimensions,
                                    std::vector<std::unique_ptr<Parsing::Initializer>>& staticInitializer,
                                    std::vector<std::vector<i64>>& emplacedPositions,
                                    const std::vector<i64>& position,
                                    Parsing::SingleInitializer& singleInit);
std::vector<i64> getScales(const std::vector<i64>& dimensions, i64 size);
std::tuple<std::vector<std::unique_ptr<Parsing::Initializer>>, std::vector<std::vector<i64>>>
    flattenWithPositions(Type innerArrayType,
                         Parsing::Initializer* arrayInit,
                         std::vector<Error>& errors,
                         const std::vector<i64>& dimensions);
bool isZeroSingleInit(const Parsing::Initializer& init);
std::vector<std::unique_ptr<Parsing::Initializer>> getZeroInits(
    const std::vector<std::unique_ptr<Parsing::Initializer>>& staticInitializer,
    const std::vector<i64>& dimensions,
    const std::vector<std::vector<i64>>& emplacedPositions);
std::unique_ptr<Parsing::Initializer> emplaceNewSingleInit(
    Type innerArrayType, Parsing::SingleInitializer& singleInit);
i64 getDistance(const std::vector<i64>& positionBefore,
                const std::vector<i64>& position,
                const std::vector<i64>& dimensions);
i64 getPosition(const std::vector<i64>& position, const std::vector<i64>& dimensions);
void emplaceZeroInitIfNecessary(const std::vector<i64>& dimensions,
                                std::vector<i64>& positionBefore,
                                const std::vector<i64>& position,
                                std::vector<std::unique_ptr<Parsing::Initializer>>& newInitializers);

} // Semantics