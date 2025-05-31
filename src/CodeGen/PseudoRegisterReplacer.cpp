#include "PseudoRegisterReplacer.hpp"

namespace CodeGen {
void PseudoRegisterReplacer::replaceIfPseudo(std::shared_ptr<Operand>& operand)
{
    if (operand->kind == Operand::Kind::Pseudo && operand) {
        const auto pseudo = dynamic_cast<PseudoOperand*>(operand.get());
        const auto entry = c_symbolTable.lookupVar(pseudo->identifier);
        if (pseudo->identifier.ends_with(".external")) {
            operand = std::make_shared<DataOperand>(pseudo->identifier.substr(0, pseudo->identifier.size() - 9));
            return;
        }
        if (entry.isSet(SymbolTable::State::CorrectType)) {
            operand = std::make_shared<DataOperand>(pseudo->identifier);
            return;
        }
        if (!m_pseudoMap.contains(pseudo->identifier)) {
            m_stackPtr -= 4;
            m_pseudoMap[pseudo->identifier] = m_stackPtr;
        }
        operand = std::make_shared<StackOperand>(m_pseudoMap.at(pseudo->identifier));
    }
}

void PseudoRegisterReplacer::visit(MoveInst& move)
{
    replaceIfPseudo(move.src);
    replaceIfPseudo(move.dst);
}

void PseudoRegisterReplacer::visit(UnaryInst& unary)
{
    replaceIfPseudo(unary.destination);
}

void PseudoRegisterReplacer::visit(BinaryInst& binary)
{
    replaceIfPseudo(binary.lhs);
    replaceIfPseudo(binary.rhs);
}

void PseudoRegisterReplacer::visit(IdivInst& idiv)
{
    replaceIfPseudo(idiv.operand);
}

void PseudoRegisterReplacer::visit(CmpInst& cmpInst)
{
    replaceIfPseudo(cmpInst.lhs);
    replaceIfPseudo(cmpInst.rhs);
}

void PseudoRegisterReplacer::visit(SetCCInst& setCCInst)
{
    replaceIfPseudo(setCCInst.operand);
}

void PseudoRegisterReplacer::visit(PushInst& pushInst)
{
    replaceIfPseudo(pushInst.operand);
}
} // namespace CodeGen