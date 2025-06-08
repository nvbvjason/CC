#include "PseudoRegisterReplacer.hpp"

namespace CodeGen {
void PseudoRegisterReplacer::replaceIfPseudo(std::shared_ptr<Operand>& operand)
{
    if (operand->kind == Operand::Kind::Pseudo && operand) {
        const auto pseudo = dynamic_cast<PseudoOperand*>(operand.get());
        if (pseudo->referingTo == ReferingTo::Extern ||
            pseudo->referingTo == ReferingTo::Static) {
            operand = std::make_shared<DataOperand>(pseudo->identifier, pseudo->type);
            return;
        }
        if (!m_pseudoMap.contains(pseudo->identifier)) {
            if (pseudo->type == AssemblyType::LongWord)
                m_stackPtr -= 4;
            if (pseudo->type == AssemblyType::QuadWord) {
                if (m_stackPtr % 8 == 0)
                    m_stackPtr -= 8;
                else
                    m_stackPtr -= 12;
            }
            m_pseudoMap[pseudo->identifier] = m_stackPtr;
        }
        operand = std::make_shared<StackOperand>(m_pseudoMap.at(pseudo->identifier), operand->type);
    }
}

void PseudoRegisterReplacer::visit(MoveInst& move)
{
    replaceIfPseudo(move.src);
    replaceIfPseudo(move.dst);
}

void PseudoRegisterReplacer::visit(MoveSXInst& moveSX)
{
    replaceIfPseudo(moveSX.src);
    replaceIfPseudo(moveSX.dst);
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