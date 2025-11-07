#include "PseudoRegisterReplacer.hpp"
#include "DynCast.hpp"

namespace CodeGen {
    std::tuple<ReferingTo, AsmType const *, bool, std::string> getPseudoValues(
        const std::shared_ptr<Operand>& operand)
    {
        if (operand->kind == Operand::Kind::PseudoMem) {
            const auto pseudoMem = dynamic_cast<PseudoMemOperand*>(operand.get());
            return {pseudoMem->referingTo, &pseudoMem->type, pseudoMem->local, pseudoMem->identifier.value};
        }
        if (operand->kind == Operand::Kind::Pseudo) {
            const auto pseudo = dynamic_cast<PseudoOperand*>(operand.get());
            return {pseudo->referingTo, &pseudo->type, pseudo->local, pseudo->identifier.value};
        }
        std::abort();
    }

void PseudoRegisterReplacer::replaceIfPseudo(std::shared_ptr<Operand>& operand)
{
    if (operand && (operand->kind == Operand::Kind::PseudoMem || operand->kind == Operand::Kind::Pseudo)) {
        const auto [referingTo, asmType, isLocal, identifier] = getPseudoValues(operand);
        const auto pseudoMem = dynamic_cast<PseudoMemOperand*>(operand.get());
        if (referingTo == ReferingTo::Extern || referingTo == ReferingTo::Static) {
            operand = std::make_shared<DataOperand>(Identifier(identifier), *asmType, !isLocal);
            return;
        }
        if (!m_pseudoMap.contains(identifier)) {
            if (asmType->kind == AsmType::Kind::ByteArray) {
                const auto byteArray = dynCast<const ByteArray>(&pseudoMem->type);
                m_stackPtr -= byteArray->size;
            }
            if (asmType->kind == AsmType::Kind::LongWord)
                m_stackPtr -= 4;
            if (asmType->kind == AsmType::Kind::QuadWord ||
                asmType->kind == AsmType::Kind::Double) {
                m_stackPtr -= 8;
            }
            constexpr i64 requiredAlignment = 8;
            if (m_stackPtr % requiredAlignment != 0)
                m_stackPtr += -requiredAlignment - m_stackPtr % requiredAlignment;
            m_pseudoMap[identifier] = m_stackPtr;
        }
        operand = std::make_shared<MemoryOperand>(
            Operand::RegKind::BP, m_pseudoMap.at(identifier), operand->type);
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

void PseudoRegisterReplacer::visit(LeaInst& lea)
{
    replaceIfPseudo(lea.src);
    replaceIfPseudo(lea.dst);
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