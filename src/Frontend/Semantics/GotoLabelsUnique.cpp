#include "GotoLabelsUnique.hpp"
#include "ASTParser.hpp"

namespace Semantics {
bool GotoLabelsUnique::programValidate(Parsing::Program& program)
{
    program.accept(*this);
    return m_valid;
}

void GotoLabelsUnique::visit(Parsing::FunDecl& funDecl)
{
    if (funDecl.body == nullptr)
        return;
    m_labels.clear();
    m_goto.clear();
    m_funName = funDecl.name;
    ASTTraverser::visit(funDecl);
    for (auto& label : m_labels)
        if (1 < label.second)
            m_valid = false;
    for (auto& gotoStmt : m_goto)
        if (!m_labels.contains(gotoStmt))
            m_valid = false;
}

void GotoLabelsUnique::visit(Parsing::GotoStmt& gotoStmt)
{
    gotoStmt.identifier += '.' + m_funName;
    m_goto.insert(gotoStmt.identifier);
    ASTTraverser::visit(gotoStmt);
}

void GotoLabelsUnique::visit(Parsing::LabelStmt& labelStmt)
{
    labelStmt.identifier += '.' + m_funName;
    ++m_labels[labelStmt.identifier];
    ASTTraverser::visit(labelStmt);
}
} // Semantics