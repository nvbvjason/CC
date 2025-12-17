#pragma once

#include "AsmAST.hpp"
#include "CodeGenInstructionFactory.hpp"

#include <memory>
#include <vector>
#include <gtest/gtest.h>

namespace CodeGen {

using RegType = Operand::RegKind;
using OperKind = Operand::Kind;
using InstKind = Inst::Kind;
using AsmType = AsmType;
using BinaryOper = BinaryInst::Operator;

class FixUpInstructionsTest : public testing::Test {
    CodeGenInstructionFactory factory;
    Program program;
public:
    std::vector<std::unique_ptr<Inst>> insts;
    void addMove(OperKind srcKind, OperKind dstKind, AsmType asmType);
    void addMoveZero(OperKind srcKind, OperKind dstKind, AsmType asmType);
    void addMoveSX(OperKind srcKind, OperKind dstKind);
    void addLea(OperKind srcKind, OperKind dstKind);
    void addDiv(OperKind srcKind, OperKind dstKind);
    void addIdiv(OperKind srcKind, OperKind dstKind);
    void addCvttsd2si(OperKind srcKind, OperKind dstKind);
    void addCvtsi2sd(OperKind srcKind, OperKind dstKind);
    void addCmp(OperKind srcKind, OperKind dstKind, AsmType asmType);
    void addBinary(BinaryInst::Operator oper, AsmType asmType, OperKind srcKind, OperKind dstKind);
    void run();
    void run(i32 stackAlloc);
protected:
    void TearDown() override
    {
        insts.clear();
    }
};

} // CodeGen