#include "PseudoRegisterReplacer.hpp"

namespace CodeGen {
void PseudoRegisterReplacer::replaceIfPseudo(std::shared_ptr<Operand>& operand)
{
    if (operand->kind == Operand::Kind::Pseudo) {
        const auto pseudo = dynamic_cast<PseudoOperand*>(operand.get());
        if (!pseudoMap.contains(pseudo->identifier)) {
            stackPtr -= 4;
            pseudoMap[pseudo->identifier] = stackPtr;
        }
        operand = std::make_shared<StackOperand>(pseudoMap.at(pseudo->identifier));
    }
}

void PseudoRegisterReplacer::visit(MoveInst& move)
{
    replaceIfPseudo(move.source);
    replaceIfPseudo(move.destination);
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
} // namespace CodeGen