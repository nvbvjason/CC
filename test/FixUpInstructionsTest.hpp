#pragma once

#include "AsmAST.hpp"
#include "CodeGenInstructionFactory.hpp"

#include <memory>
#include <vector>
#include <gtest/gtest.h>

using RegType = CodeGen::Operand::RegKind;
using AsmType = CodeGen::AsmType;
using RegisterOperand = CodeGen::RegisterOperand;
using OperKind = CodeGen::Operand::Kind;
using InstKind = CodeGen::Inst::Kind;
using AsmType = CodeGen::AsmType;
using BinaryOper = CodeGen::BinaryInst::Operator;
using std::make_shared;

class FixUpInstructionsTest : public testing::Test {
    CodeGen::CodeGenInstructionFactory factory;
public:
    std::vector<std::unique_ptr<CodeGen::Inst>> insts;
    void addMove(OperKind srcKind, OperKind dstKind, AsmType asmType);
    void addMoveZero(OperKind srcKind, OperKind dstKind, AsmType asmType);
    void addMoveSX(OperKind srcKind, OperKind dstKind);
    void addLea(OperKind srcKind, OperKind dstKind);
    void addDiv(OperKind srcKind, OperKind dstKind);
    void addIdiv(OperKind srcKind, OperKind dstKind);
    void addCvttsd2si(OperKind srcKind, OperKind dstKind);
    void addCvtsi2sd(OperKind srcKind, OperKind dstKind);
    void addCmp(OperKind srcKind, OperKind dstKind, AsmType asmType);
    void addBinary(CodeGen::BinaryInst::Operator oper, AsmType asmType, OperKind srcKind, OperKind dstKind);
    void run();
    void run(i32 stackAlloc);
protected:
    void TearDown() override
    {
        insts.clear();
    }
};