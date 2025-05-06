#pragma once

#ifndef CC_SEMANTICS_SWITCH_HPP
#define CC_SEMANTICS_SWITCH_HPP

#include "ConstASTTraverser.hpp"
#include "ShortTypes.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Semantics {

class Switch : public Parsing::ConstASTTraverser {
    bool m_valid = true;
    std::unordered_set<std::string> m_default;
    std::unordered_map<std::string, std::vector<i32>> m_case;
public:
    bool programValidate(const Parsing::Program& program);

    void visit(const Parsing::SwitchStmt&) override;
    void visit(const Parsing::CaseStmt&) override;
    void visit(const Parsing::DefaultStmt&) override;
};

} // Semantics

#endif // CC_SEMANTICS_SWITCH_HPP
