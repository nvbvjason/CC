#pragma once

#include "ASTTraverser.hpp"
#include "ShortTypes.hpp"
#include "Error.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Semantics {

class GotoLabelsUnique : public Parsing::ASTTraverser {
    std::unordered_map<std::string, std::vector<i64>> m_labels;
    std::unordered_set<Parsing::GotoStmt*> m_goto;
    std::string m_funName;
    std::vector<Error> m_errors;
public:
    std::vector<Error> programValidate(Parsing::Program& program);

    void visit(Parsing::FunDecl& funDecl) override;
    void visit(Parsing::GotoStmt& gotoStmt) override;
    void visit(Parsing::LabelStmt& labelStmt) override;
};

} // Semantics