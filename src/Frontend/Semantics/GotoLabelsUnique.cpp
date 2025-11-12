#include "GotoLabelsUnique.hpp"
#include "ASTParser.hpp"

#include <ranges>

namespace Semantics {
std::vector<Error> GotoLabelsUnique::programValidate(Parsing::Program& program)
{
    program.accept(*this);
    return std::move(m_errors);
}

void GotoLabelsUnique::visit(Parsing::FuncDeclaration& funDecl)
{
    if (funDecl.body == nullptr)
        return;
    m_labels.clear();
    m_goto.clear();
    m_funName = funDecl.name;
    ASTTraverser::visit(funDecl);
    for (const std::vector<i64>& locations: m_labels | std::views::values)
        if (1 < locations.size())
            for (const auto& location: locations)
                m_errors.emplace_back("Duplicate labels at ", location);
    for (auto& gotoStmt : m_goto)
        if (!m_labels.contains(gotoStmt->identifier))
            m_errors.emplace_back("Did not find goto label ", gotoStmt->location);
}

void GotoLabelsUnique::visit(Parsing::GotoStmt& gotoStmt)
{
    gotoStmt.identifier += '.' + m_funName;
    m_goto.insert(&gotoStmt);
    ASTTraverser::visit(gotoStmt);
}

void GotoLabelsUnique::visit(Parsing::LabelStmt& labelStmt)
{
    labelStmt.identifier += '.' + m_funName;
    m_labels[labelStmt.identifier].emplace_back(labelStmt.location);
    ASTTraverser::visit(labelStmt);
}
} // Semantics