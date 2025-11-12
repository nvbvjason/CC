#include "PseudoRegisterReplacer.hpp"
#include "DynCast.hpp"
#include "Operators.hpp"

namespace CodeGen {

std::tuple<ReferingTo, AsmType, bool, std::string, i64> getPseudoValues(const std::shared_ptr<Operand>& operand)
{
    if (operand->kind == Operand::Kind::PseudoMem) {
        const auto pseudoMem = dynamic_cast<PseudoMemOperand*>(operand.get());
        return {pseudoMem->referingTo, pseudoMem->type, pseudoMem->local, pseudoMem->identifier.value, pseudoMem->offset};
    }
    if (operand->kind == Operand::Kind::Pseudo) {
        const auto pseudo = dynamic_cast<PseudoOperand*>(operand.get());
        return {pseudo->referingTo, pseudo->type, pseudo->local, pseudo->identifier.value, 0};
    }
    std::abort();
}

void PseudoRegisterReplacer::replaceIfPseudo(std::shared_ptr<Operand>& operand)
{
    if (operand && (operand->kind == Operand::Kind::PseudoMem || operand->kind == Operand::Kind::Pseudo)) {
        const auto [referingTo, asmType, isLocal, identifier, offset] = getPseudoValues(operand);
        if (referingTo == ReferingTo::Extern || referingTo == ReferingTo::Static) {
            operand = std::make_shared<DataOperand>(Identifier(identifier), asmType, !isLocal);
            return;
        }
        if (!m_pseudoMap.contains(identifier)) {
            if (operand->kind == Operand::Kind::PseudoMem) {
                const auto pseudoMem = dynCast<PseudoMemOperand>(operand.get());
                i64 arraySize = pseudoMem->size * Operators::getSizeAsmType(asmType);
                if (arraySize % pseudoMem->alignment != 0) {
                    arraySize -= arraySize % pseudoMem->alignment;
                    arraySize += pseudoMem->alignment;
                }
                m_stackPtr -= arraySize;
                fitToAlignment();
                m_pseudoMap[identifier] = m_stackPtr;
                operand = std::make_shared<MemoryOperand>(
                    Operand::RegKind::BP, m_stackPtr, operand->type);
                return;
            }
            m_stackPtr -= 1 * Operators::getSizeAsmType(asmType);
            fitToAlignment();
            m_pseudoMap[identifier] = m_stackPtr;
        }
        operand = std::make_shared<MemoryOperand>(
            Operand::RegKind::BP, m_pseudoMap.at(identifier) + offset, operand->type);
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

void PseudoRegisterReplacer::visit(PushPseudoInst& pushPseudoInst)
{
    m_stackPtr -= pushPseudoInst.size * Operators::getSizeAsmType(pushPseudoInst.type);
    fitToAlignment();
    m_pseudoMap[pushPseudoInst.identifier.value] = m_stackPtr;
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

void PseudoRegisterReplacer::fitToAlignment()
{
    constexpr i64 requiredAlignment = 8;
    if (m_stackPtr % requiredAlignment != 0)
        m_stackPtr += -requiredAlignment - m_stackPtr % requiredAlignment;
}
} // namespace CodeGen