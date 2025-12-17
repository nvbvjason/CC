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

void PseudoRegisterReplacer::replace(Inst &inst)
{
    using Kind = Inst::Kind;
    switch (inst.kind) {
        case Kind::Move: {
            const auto move = dynCast<MoveInst>(&inst);
            return replace(*move);
        }
        case Kind::MoveSX: {
            const auto move = dynCast<MoveSXInst>(&inst);
            return replace(*move);
        }
        case Kind::MoveZeroExtend: {
            const auto moveZeroExtend = dynCast<MoveZeroExtendInst>(&inst);
            return replace(*moveZeroExtend);
        }
        case Kind::Lea: {
            const auto lea = dynCast<LeaInst>(&inst);
            return replace(*lea);
        }
        case Kind::Unary: {
            const auto Unary = dynCast<UnaryInst>(&inst);
            return replace(*Unary);
        }
        case Kind::Binary: {
            const auto Binary = dynCast<BinaryInst>(&inst);
            return replace(*Binary);
        }
        case Kind::Idiv: {
            const auto idiv = dynCast<IdivInst>(&inst);
            return replace(*idiv);
        }
        case Kind::Div: {
            const auto div = dynCast<DivInst>(&inst);
            return replace(*div);
        }
        case Kind::Cmp: {
            const auto cmp = dynCast<CmpInst>(&inst);
            return replace(*cmp);
        }
        case Kind::SetCC: {
            const auto setCC = dynCast<SetCCInst>(&inst);
            return replace(*setCC);
        }
        case Kind::PushPseudo: {
            const auto pushPseudo = dynCast<PushPseudoInst>(&inst);
            return replace(*pushPseudo);
        }
        case Kind::Push: {
            const auto push = dynCast<PushInst>(&inst);
            return replace(*push);
        }
        case Kind::Cvttsd2si: {
            const auto Cvttsd2si = dynCast<Cvttsd2siInst>(&inst);
            return replace(*Cvttsd2si);
        }
        case Kind::Cvtsi2sd: {
            const auto cvtsi2sd = dynCast<Cvtsi2sdInst>(&inst);
            return replace(*cvtsi2sd);
        }
    }
}

void PseudoRegisterReplacer::replace(MoveInst& move)
{
    move.src = replaceIfPseudo(move.src);
    move.dst = replaceIfPseudo(move.dst);
}

void PseudoRegisterReplacer::replace(MoveSXInst& moveSX)
{
    moveSX.src = replaceIfPseudo(moveSX.src);
    moveSX.dst = replaceIfPseudo(moveSX.dst);
}

void PseudoRegisterReplacer::replace(MoveZeroExtendInst& moveZero)
{
    moveZero.src = replaceIfPseudo(moveZero.src);
    moveZero.dst = replaceIfPseudo(moveZero.dst);
}

void PseudoRegisterReplacer::replace(LeaInst& lea)
{
    lea.src = replaceIfPseudo(lea.src);
    lea.dst = replaceIfPseudo(lea.dst);
}

void PseudoRegisterReplacer::replace(UnaryInst& unary)
{
    unary.dst = replaceIfPseudo(unary.dst);
}

void PseudoRegisterReplacer::replace(BinaryInst& binary)
{
    binary.lhs = replaceIfPseudo(binary.lhs);
    binary.rhs = replaceIfPseudo(binary.rhs);
}

void PseudoRegisterReplacer::replace(IdivInst& idiv)
{
    idiv.operand = replaceIfPseudo(idiv.operand);
}

void PseudoRegisterReplacer::replace(DivInst& div)
{
    div.operand= replaceIfPseudo(div.operand);
}

void PseudoRegisterReplacer::replace(CmpInst& cmpInst)
{
    cmpInst.lhs = replaceIfPseudo(cmpInst.lhs);
    cmpInst.rhs = replaceIfPseudo(cmpInst.rhs);
}

void PseudoRegisterReplacer::replace(SetCCInst& setCCInst)
{
    setCCInst.operand = replaceIfPseudo(setCCInst.operand);
}

void PseudoRegisterReplacer::replace(const PushPseudoInst& pushPseudoInst)
{
    stackPtr -= pushPseudoInst.size;
    if (pushPseudoInst.size < 16)
        fitTo8Alignment();
    else
        fitTo16Alignment();
    pseudoMap[pushPseudoInst.identifier.value] = stackPtr;
}

void PseudoRegisterReplacer::replace(PushInst& pushInst)
{
    pushInst.operand = replaceIfPseudo(pushInst.operand);
}

void PseudoRegisterReplacer::replace(Cvttsd2siInst& cvttsd2siInst)
{
    cvttsd2siInst.src = replaceIfPseudo(cvttsd2siInst.src);
    cvttsd2siInst.dst = replaceIfPseudo(cvttsd2siInst.dst);
}

void PseudoRegisterReplacer::replace(Cvtsi2sdInst& cvtsi2sdInst)
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