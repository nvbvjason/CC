#pragma once

#ifndef CC_CODEGEN_PSEUDO_REGISTER_REPLACER_HPP
#define CC_CODEGEN_PSEUDO_REGISTER_REPLACER_HPP

#include "AbstractTree.hpp"

#include <unordered_map>

namespace CodeGen {

class PseudoRegisterReplacer final : public InstVisitor {
    std::unordered_map<std::string, i32> pseudoMap;
    i32 stackPtr = 0;
public:

    [[nodiscard]] i32 stackPointer() const { return stackPtr; }

    void visit(MoveInst& move) override;
    void visit(UnaryInst& unary) override;
    void visit(BinaryInst& binary) override;
    void visit(IdivInst& idiv) override;

    void visit(CdqInst&) override {}
    void visit(AllocStackInst&) override {}
    void visit(ReturnInst&) override {}
private:
    void replaceIfPseudo(std::shared_ptr<Operand>& operand);
};

} // CodeGen

#endif // CC_CODEGEN_PSEUDO_REGISTER_REPLACER_HPP
