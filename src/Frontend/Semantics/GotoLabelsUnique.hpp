#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ASTTraverser.hpp"
#include "ShortTypes.hpp"

namespace Semantics {

class GotoLabelsUnique : public Parsing::ASTTraverser {
    bool m_valid = true;
    std::unordered_map<std::string, i32> m_labels;
    std::unordered_set<std::string> m_goto;
    std::string m_funName;
public:
    bool programValidate(Parsing::Program& program);

    void visit(Parsing::FunDecl& funDecl) override;
    void visit(Parsing::GotoStmt& gotoStmt) override;
    void visit(Parsing::LabelStmt& labelStmt) override;
};

} // Semantics