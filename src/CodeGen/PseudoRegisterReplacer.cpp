#include "PseudoRegisterReplacer.hpp"
#include "DynCast.hpp"
#include "Operators.hpp"

namespace CodeGen {

std::tuple<ReferringTo, AsmType, bool, std::string, i64> getPseudoValues(const Operand* operand)
{
    if (operand->kind == Operand::Kind::PseudoMem) {
        const auto pseudoMem = dynCast<const PseudoMemOperand>(operand);
        return {pseudoMem->referringTo, pseudoMem->type, pseudoMem->local, pseudoMem->identifier.value, pseudoMem->offset};
    }
    if (operand->kind == Operand::Kind::Pseudo) {
        const auto pseudo = dynCast<const PseudoOperand>(operand);
        return {pseudo->referringTo, pseudo->type, pseudo->local, pseudo->identifier.value, 0};
    }
    std::abort();
}

const Operand* PseudoRegisterReplacer::replaceIfPseudo(const Operand* operand)
{
    if (operand && (operand->kind == Operand::Kind::PseudoMem || operand->kind == Operand::Kind::Pseudo)) {
        const auto [referringTo, asmType, isLocal, identifier, offset] = getPseudoValues(operand);
        if (referringTo == ReferringTo::Extern || referringTo == ReferringTo::Static)
            return program.getDataOperand(asmType, offset, Identifier(identifier), !isLocal);
        if (!pseudoMap.contains(identifier)) {
            if (operand->kind == Operand::Kind::PseudoMem) {
                const auto pseudoMem = dynCast<const PseudoMemOperand>(operand);
                i64 arraySize = pseudoMem->size;
                if (arraySize != 0
                      && pseudoMem->alignment
                      && arraySize % pseudoMem->alignment != 0) {
                    arraySize -= arraySize % pseudoMem->alignment;
                    arraySize += pseudoMem->alignment;
                }
                stackPtr -= arraySize;
                fitTo8Alignment();
                pseudoMap[identifier] = stackPtr;
                return program.getMemoryOperand(Operand::RegKind::BP, stackPtr, operand->type);
            }
            stackPtr -= 1 * asmType.size;
            fitTo8Alignment();
            pseudoMap[identifier] = stackPtr;
        }
        return program.getMemoryOperand(
            Operand::RegKind::BP, pseudoMap.at(identifier) + offset, operand->type);
    }
    return operand;
}

void PseudoRegisterReplacer::visit(MoveInst& move)
{
    move.src = replaceIfPseudo(move.src);
    move.dst = replaceIfPseudo(move.dst);
}

void PseudoRegisterReplacer::visit(MoveSXInst& moveSX)
{
    moveSX.src = replaceIfPseudo(moveSX.src);
    moveSX.dst = replaceIfPseudo(moveSX.dst);
}

void PseudoRegisterReplacer::visit(MoveZeroExtendInst& moveZero)
{
    moveZero.src = replaceIfPseudo(moveZero.src);
    moveZero.dst = replaceIfPseudo(moveZero.dst);
}

void PseudoRegisterReplacer::visit(LeaInst& lea)
{
    lea.src = replaceIfPseudo(lea.src);
    lea.dst = replaceIfPseudo(lea.dst);
}

void PseudoRegisterReplacer::visit(UnaryInst& unary)
{
    unary.dst = replaceIfPseudo(unary.dst);
}

void PseudoRegisterReplacer::visit(BinaryInst& binary)
{
    binary.lhs = replaceIfPseudo(binary.lhs);
    binary.rhs = replaceIfPseudo(binary.rhs);
}

void PseudoRegisterReplacer::visit(IdivInst& idiv)
{
    idiv.operand = replaceIfPseudo(idiv.operand);
}

void PseudoRegisterReplacer::visit(DivInst& div)
{
    div.operand= replaceIfPseudo(div.operand);
}

void PseudoRegisterReplacer::visit(CmpInst& cmpInst)
{
    cmpInst.lhs = replaceIfPseudo(cmpInst.lhs);
    cmpInst.rhs = replaceIfPseudo(cmpInst.rhs);
}

void PseudoRegisterReplacer::visit(SetCCInst& setCCInst)
{
    setCCInst.operand = replaceIfPseudo(setCCInst.operand);
}

void PseudoRegisterReplacer::visit(PushPseudoInst& pushPseudoInst)
{
    stackPtr -= pushPseudoInst.size;
    if (pushPseudoInst.size < 16)
        fitTo8Alignment();
    else
        fitTo16Alignment();
    pseudoMap[pushPseudoInst.identifier.value] = stackPtr;
}

void PseudoRegisterReplacer::visit(PushInst& pushInst)
{
    pushInst.operand = replaceIfPseudo(pushInst.operand);
}

void PseudoRegisterReplacer::visit(Cvttsd2siInst& cvttsd2siInst)
{
    cvttsd2siInst.src = replaceIfPseudo(cvttsd2siInst.src);
    cvttsd2siInst.dst = replaceIfPseudo(cvttsd2siInst.dst);
}

void PseudoRegisterReplacer::visit(Cvtsi2sdInst& cvtsi2sdInst)
{
    cvtsi2sdInst.src = replaceIfPseudo(cvtsi2sdInst.src);
    cvtsi2sdInst.dst = replaceIfPseudo(cvtsi2sdInst.dst);
}

void PseudoRegisterReplacer::fitTo8Alignment()
{
    constexpr i64 requiredAlignment = 8;
    if (stackPtr % requiredAlignment != 0)
        stackPtr += -requiredAlignment - stackPtr % requiredAlignment;
}

void PseudoRegisterReplacer::fitTo16Alignment()
{
    constexpr i64 requiredAlignment = 16;
    if (stackPtr % requiredAlignment != 0)
        stackPtr += -requiredAlignment - stackPtr % requiredAlignment;
}
} // namespace CodeGen