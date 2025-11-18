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

class Labeling : public Parsing::ASTTraverser {
    std::vector<Error> errors;
    std::unordered_set<std::string> defaultCase;
    std::unordered_map<std::string, std::vector<std::variant<i32, i64, u32, u64>>> switchCases;
    std::unordered_map<std::string, std::vector<i64>> m_labels;
    std::unordered_set<Parsing::GotoStmt*> m_goto;
    std::string m_funName;
    Type conditionType = Type::I32;
    std::string breakLabel;
    std::string continueLabel;
    std::string switchLabel;
public:
    std::vector<Error> programValidate(Parsing::Program& program);

    void visit(Parsing::FuncDecl& funDecl) override;

    void visit(Parsing::VarDecl& varDecl) override;

    void visit(Parsing::GotoStmt& gotoStmt) override;
    void visit(Parsing::LabelStmt& labelStmt) override;
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
    void emplaceError(std::string&& message, const i64 location)
    {
        errors.emplace_back(std::move(message), location);
    }
};

inline bool Labeling::isOutsideSwitchStmt(const Parsing::CaseStmt& caseStmt)
{
    return caseStmt.identifier.empty();
}

inline bool Labeling::isNonConstantInSwitchCase(const Parsing::CaseStmt& caseStmt)
{
    return caseStmt.condition->kind != Parsing::Expr::Kind::Constant;
}

inline std::string Labeling::makeTemporary(const std::string& name)
{
    static i32 m_counter = 0;
    return name + '.' + std::to_string(m_counter++);
}
} // Semantics