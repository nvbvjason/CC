#include "PseudoRegisterReplacer.hpp"

namespace CodeGen {
void PseudoRegisterReplacer::replaceIfPseudo(std::shared_ptr<Operand>& operand)
{
    if (operand->kind == Operand::Kind::Pseudo && operand) {
        const auto pseudo = dynamic_cast<PseudoOperand*>(operand.get());
        if (pseudo->referingTo == ReferingTo::Extern ||
            pseudo->referingTo == ReferingTo::Static) {
            operand = std::make_shared<DataOperand>(pseudo->identifier, pseudo->type, pseudo->isConst);
            return;
        }
        if (!m_pseudoMap.contains(pseudo->identifier.value)) {
            if (pseudo->type == AsmType::LongWord)
                m_stackPtr -= 4;
            if (pseudo->type == AsmType::QuadWord ||
                pseudo->type == AsmType::Double) {
                m_stackPtr -= 8;
            }
            constexpr i32 requiredAlignment = 8;
            if (m_stackPtr % requiredAlignment != 0)
                m_stackPtr += -requiredAlignment - m_stackPtr % requiredAlignment;
            m_pseudoMap[pseudo->identifier.value] = m_stackPtr;
        }
        operand = std::make_shared<StackOperand>(m_pseudoMap.at(pseudo->identifier.value), operand->type);
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

void PseudoRegisterReplacer::visit(MoveZeroExtendInst& moveZero)
{
    replaceIfPseudo(moveZero.src);
    replaceIfPseudo(moveZero.dst);
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

void PseudoRegisterReplacer::visit(DivInst& div)
{
    replaceIfPseudo(div.operand);
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

void PseudoRegisterReplacer::visit(Cvttsd2siInst& cvttsd2siInst)
{
    replaceIfPseudo(cvttsd2siInst.src);
    replaceIfPseudo(cvttsd2siInst.dst);
}

void PseudoRegisterReplacer::visit(Cvtsi2sdInst& cvtsi2sdInst)
{
    replaceIfPseudo(cvtsi2sdInst.src);
    replaceIfPseudo(cvtsi2sdInst.dst);
}
} // namespace CodeGen