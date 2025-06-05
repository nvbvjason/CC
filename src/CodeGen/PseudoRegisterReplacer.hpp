#pragma once

#ifndef CC_CODEGEN_PSEUDO_REGISTER_REPLACER_HPP
#define CC_CODEGEN_PSEUDO_REGISTER_REPLACER_HPP

#include "AsmAST.hpp"
#include "Frontend/SymbolTable.hpp"

#include <unordered_map>

namespace CodeGen {

class PseudoRegisterReplacer final : public InstVisitor {
    std::unordered_map<std::string, i32> m_pseudoMap;
    i32 m_stackPtr = 0;
    const SymbolTable &c_symbolTable;
public:
    explicit PseudoRegisterReplacer(const SymbolTable &symbolTable)
        : c_symbolTable(symbolTable) {}
    [[nodiscard]] i32 stackPointer() const { return m_stackPtr; }

    void visit(MoveInst& move) override;
    void visit(UnaryInst& unary) override;
    void visit(BinaryInst& binary) override;
    void visit(IdivInst& idiv) override;
    void visit(CmpInst& cmpInst) override;
    void visit(SetCCInst& setCCInst) override;
    void visit(PushInst& pushInst) override;

    void visit(DeallocStackInst&) override {}
    void visit(CallInst&) override {}
    void visit(CdqInst&) override {}
    void visit(AllocStackInst&) override {}
    void visit(ReturnInst&) override {}
    void visit(JmpInst&) override {}
    void visit(JmpCCInst&) override {}
    void visit(LabelInst&) override {}
private:
    void replaceIfPseudo(std::shared_ptr<Operand>& operand);
    bool isStatic(const std::string& iden);
};

inline bool PseudoRegisterReplacer::isStatic(const std::string& iden)
{
    return iden.contains(".s.");
}

} // CodeGen

#endif // CC_CODEGEN_PSEUDO_REGISTER_REPLACER_HPP
