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
public:
    bool programValidate(Parsing::Program& program);

    void visit(Parsing::SwitchStmt&) override;
    void visit(Parsing::CaseStmt&) override;
    void visit(Parsing::DefaultStmt&) override;
    void visit(Parsing::ContinueStmt&) override;
    void visit(Parsing::BreakStmt&) override;
};

} // Semantics

#endif // CC_SEMANTICS_SWITCH_HPP
