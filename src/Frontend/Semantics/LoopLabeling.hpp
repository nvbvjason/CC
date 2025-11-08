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
    std::vector<Parsing::VarDecl*> varDecls;
    Type conditionType = Type::I32;
    std::string breakLabel;
    std::string continueLabel;
    std::string switchLabel;
public:
    std::tuple<std::vector<Error>, std::vector<Parsing::VarDecl*>> programValidate(
        Parsing::Program& program);

    void visit(Parsing::VarDecl& varDecl) override;

    void visit(Parsing::BreakStmt&) override;
    void visit(Parsing::ContinueStmt&) override;
    void visit(Parsing::DefaultStmt&) override;
    void visit(Parsing::CaseStmt&) override;
    void visit(Parsing::WhileStmt&) override;
    void visit(Parsing::DoWhileStmt&) override;
    void visit(Parsing::ForStmt&) override;
    void visit(Parsing::SwitchStmt&) override;
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

} // Semantics