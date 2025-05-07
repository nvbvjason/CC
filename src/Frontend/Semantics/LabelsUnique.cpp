#include "LabelsUnique.hpp"
#include "ASTParser.hpp"

namespace Semantics {
bool LabelsUnique::programValidate(Parsing::Program& program)
{
    program.accept(*this);
    return m_valid;
}

void LabelsUnique::visit(const Parsing::FunDecl& funDecl)
{
    m_labels.clear();
    m_goto.clear();
    ConstASTTraverser::visit(funDecl);
    for (auto& label : m_labels)
        if (1 < label.second)
            m_valid = false;
    for (auto& gotoStmt : m_goto)
        if (!m_labels.contains(gotoStmt))
            m_valid = false;
}

void LabelsUnique::visit(const Parsing::GotoStmt& gotoStmt)
{
    m_goto.insert(gotoStmt.identifier);
}

void LabelsUnique::visit(const Parsing::LabelStmt& labelStmt)
{
    ++m_labels[labelStmt.identifier];
    ConstASTTraverser::visit(labelStmt);
}
} // Semantics