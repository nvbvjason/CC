#pragma once

#ifndef CC_SEMANTICS_SWITCH_HPP
#define CC_SEMANTICS_SWITCH_HPP

#include "ASTTraverser.hpp"
#include "ShortTypes.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Semantics {

class LoopLabeling : public Parsing::ASTTraverser {
    bool m_valid = true;
    std::unordered_set<std::string> m_default;
    std::unordered_map<std::string, std::vector<i32>> m_case;
    std::string m_breakLabel;
    std::string m_continueLabel;
    std::string m_switchLabel;
public:
    bool programValidate(Parsing::Program& program);

    void visit(Parsing::BreakStmt&) override;
    void visit(Parsing::ContinueStmt&) override;
    void visit(Parsing::DefaultStmt&) override;
    void visit(Parsing::CaseStmt&) override;
    void visit(Parsing::WhileStmt&) override;
    void visit(Parsing::DoWhileStmt&) override;
    void visit(Parsing::ForStmt&) override;
    void visit(Parsing::SwitchStmt&) override;
private:
    static std::string makeTemporary(const std::string& name);
};

inline std::string LoopLabeling::makeTemporary(const std::string& name)
{
    static i32 m_counter = 0;
    return name + '.' + std::to_string(m_counter++);
}

} // Semantics

#endif // CC_SEMANTICS_SWITCH_HPP
