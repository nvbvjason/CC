#pragma once

#ifndef CC_SEMANTICS_LABELS_UNIQUE_HPP
#define CC_SEMANTICS_LABELS_UNIQUE_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ConstASTTraverser.hpp"
#include "ShortTypes.hpp"

namespace Semantics {

class LabelsUnique : public Parsing::ConstASTTraverser {
    bool m_valid = true;
    std::unordered_map<std::string, i32> m_labels;
    std::unordered_set<std::string> m_goto;
public:
    bool programValidate(Parsing::Program& program);

    void visit(const Parsing::FunDecl& funDecl) override;
    void visit(const Parsing::GotoStmt& gotoStmt) override;
    void visit(const Parsing::LabelStmt& labelStmt) override;
};

} // Semantics

#endif // CC_SEMANTICS_LABELS_UNIQUE_HPP
