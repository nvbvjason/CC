#pragma once

#include "ASTTraverser.hpp"
#include "ASTParser.hpp"
#include "ShortTypes.hpp"
#include "Error.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Semantics {

template<typename TargetType>
void processSwitchCase(const Parsing::ConstExpr* constantExpr,
                       std::unordered_map<std::string,
                       std::vector<std::variant<i32, i64, u32, u64>>>& switchCases,
                       const std::string& switchLabel,
                       Parsing::CaseStmt& caseStmt,
                       std::vector<Error>& errors)
{
    TargetType value = constantExpr->getValue<TargetType>();
    const auto it = switchCases.find(switchLabel);
    for (const std::variant<i32, i64, u32, u64>& v : it->second) {
        if (value == std::get<TargetType>(v)) {
            errors.emplace_back("Duplicate case value in switch statement ", constantExpr->location);
            return;
        }
    }
    it->second.emplace_back(value);
    caseStmt.identifier += std::to_string(value);
}

class LoopLabeling : public Parsing::ASTTraverser {
    std::vector<Error> errors;
    std::unordered_set<std::string> m_default;
    std::unordered_map<std::string, std::vector<std::variant<i32, i64, u32, u64>>> switchCases;
    Type conditionType = Type::I32;
    std::string breakLabel;
    std::string continueLabel;
    std::string switchLabel;
public:
    std::vector<Error> programValidate(Parsing::Program& program);

    void visit(Parsing::VarDecl& varDecl) override;

    void visit(Parsing::BreakStmt& breakStmt) override;
    void visit(Parsing::ContinueStmt& continueStmt) override;
    void visit(Parsing::DefaultStmt& defaultStmt) override;
    void visit(Parsing::CaseStmt&) override;
    void visit(Parsing::WhileStmt& whileStmt) override;
    void visit(Parsing::DoWhileStmt& doWhileStmt) override;
    void visit(Parsing::ForStmt& forStmt) override;
    void visit(Parsing::SwitchStmt& switchStmt) override;
private:
    static bool isOutsideSwitchStmt(const Parsing::CaseStmt& caseStmt);
    static bool isNonConstantInSwitchCase(const Parsing::CaseStmt& caseStmt);
    static std::string makeTemporary(const std::string& name);
};

inline bool LoopLabeling::isOutsideSwitchStmt(const Parsing::CaseStmt& caseStmt)
{
    return caseStmt.identifier.empty();
}

inline bool LoopLabeling::isNonConstantInSwitchCase(const Parsing::CaseStmt& caseStmt)
{
    return caseStmt.condition->kind != Parsing::Expr::Kind::Constant;
}

inline std::string LoopLabeling::makeTemporary(const std::string& name)
{
    static i32 m_counter = 0;
    return name + '.' + std::to_string(m_counter++);
}

void initArray(Parsing::VarDecl& array);
std::vector<i64> getDimensions(const Parsing::VarDecl& array);
std::vector<i64> getScales(const std::vector<i64>& dimensions, i64 size);
bool isZeroSingleInit(const Parsing::Initializer& init);
std::vector<std::unique_ptr<Parsing::Initializer>> getZeroInits(
    const std::vector<std::unique_ptr<Parsing::Initializer>>& staticInitializer,
    const std::vector<i64>& dimensions,
    const std::vector<std::vector<i64>>& emplacedPositions);
std::unique_ptr<Parsing::Initializer> emplaceNewSingleInit(
    Type innerArrayType, Parsing::SingleInitializer& singleInit);
std::tuple<std::vector<std::unique_ptr<Parsing::Initializer>>, std::vector<std::vector<i64>>>
    getSingleInitAndPositions(Type innerArrayType, Parsing::Initializer* arrayInit);
i64 getDistance(const std::vector<i64>& positionBefore,
                const std::vector<i64>& position,
                const std::vector<i64>& dimensions);
i64 getPosition(const std::vector<i64>& position, const std::vector<i64>& dimensions);
void emplaceZeroInitIfNecessary(const std::vector<i64>& dimensions,
                                std::vector<i64>& positionBefore,
                                const std::vector<i64>& position,
                                std::vector<std::unique_ptr<Parsing::Initializer>>& newInitializers);

} // Semantics