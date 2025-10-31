#pragma once

#include "ASTIr.hpp"
#include "ASTParser.hpp"
#include "SymbolTable.hpp"
#include "ExprResult.hpp"

#include <unordered_set>

namespace Ir {
class GenerateIr {
    using Storage = Parsing::Declaration::StorageClass;

    bool m_global = true;
    std::vector<std::unique_ptr<Instruction>> insts;
    SymbolTable& m_symbolTable;
    std::unordered_set<std::string> m_writtenGlobals;
    std::vector<std::unique_ptr<TopLevel>> m_topLevels;
public:
    explicit GenerateIr(SymbolTable& symbolTable)
        : m_symbolTable(symbolTable) {}
    void program(const Parsing::Program& parsingProgram, Program& tackyProgram);
    std::unique_ptr<TopLevel> topLevelIr(const Parsing::Declaration& decl);
    std::unique_ptr<TopLevel> functionIr(const Parsing::FunDeclaration& parsingFunction);
    std::unique_ptr<TopLevel> staticVariableIr(const Parsing::VarDecl& varDecl);
    std::shared_ptr<Value> genStaticVariableInit(const Parsing::VarDecl& varDecl, bool defined);
    void genBlock(const Parsing::Block& block);
    void genBlockItem(const Parsing::BlockItem& blockItem);
    void genDeclaration(const Parsing::Declaration& decl);
    void genStaticLocal(const Parsing::VarDecl& varDecl);

    void genStmt(const Parsing::Stmt& stmt);
    void genReturnStmt(const Parsing::ReturnStmt& returnStmt);
    void genIfStmt(const Parsing::IfStmt& ifStmt);
    void genForInit(const Parsing::ForInit& forInit);
    void genIfBasicStmt(const Parsing::IfStmt& ifStmt);
    void genIfElseStmt(const Parsing::IfStmt& ifStmt);
    void genGotoStmt(const Parsing::GotoStmt& gotoStmt);
    void genCompoundStmt(const Parsing::CompoundStmt& compoundStmt);
    void genBreakStmt(const Parsing::BreakStmt& breakStmt);
    void genContinueStmt(const Parsing::ContinueStmt& continueStmt);
    void genLabelStmt(const Parsing::LabelStmt& labelStmt);
    void genCaseStmt(const Parsing::CaseStmt& caseStmt);
    void genDefaultStmt(const Parsing::DefaultStmt& defaultStmt);
    void genDoWhileStmt(const Parsing::DoWhileStmt& doWhileStmt);
    void genWhileStmt(const Parsing::WhileStmt& whileStmt);
    void genForStmt(const Parsing::ForStmt& forStmt);
    void genSwitchStmt(const Parsing::SwitchStmt& stmt);

    std::unique_ptr<ExprResult> genInst(const Parsing::Expr& parsingExpr);
    std::shared_ptr<Value> genInstAndConvert(const Parsing::Expr& parsingExpr);
    std::shared_ptr<ValueVar> castValue(std::shared_ptr<Value> result, Type towards, Type from);

    std::unique_ptr<ExprResult> genCastInst(const Parsing::CastExpr& castExpr);
    std::unique_ptr<ExprResult> genUnaryInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genUnaryBasicInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genBinaryInst(const Parsing::BinaryExpr& binaryExpr);
    void genCompoundAssignWithoutDeref(
        const Parsing::AssignmentExpr& assignmentExpr,
        std::shared_ptr<Value>& rhs,
        std::shared_ptr<Value> lhs);
    std::unique_ptr<ExprResult> genBinaryAndInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genBinaryOrInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genAssignInst(const Parsing::AssignmentExpr& assignmentExpr);
    std::unique_ptr<ExprResult> genTernaryInst(const Parsing::TernaryExpr& ternaryExpr);
    std::unique_ptr<ExprResult> genFuncCallInst(const Parsing::FuncCallExpr& funcCallExpr);
    std::unique_ptr<ExprResult> genAddrOfInst(const Parsing::AddrOffExpr& addrOffExpr);
    std::unique_ptr<ExprResult> genDereferenceInst(const Parsing::DereferenceExpr& dereferenceExpr);
    static std::unique_ptr<ExprResult> genConstInst(const Parsing::ConstExpr& constExpr);
    static std::unique_ptr<ExprResult> genVarInst(const Parsing::VarExpr& varExpr);
private:
    void emplaceBinary(const BinaryInst::Operation oper,
                       std::shared_ptr<Value> lhs,
                       std::shared_ptr<Value> rhs,
                       std::shared_ptr<Value> dst,
                       const Type type)
    {
        insts.emplace_back(std::make_unique<BinaryInst>(oper, lhs, rhs, dst, type));
    }
    void emplaceLabel(const Identifier& iden)
    {
        insts.emplace_back(std::make_unique<LabelInst>(iden));
    }
    void emplaceCopy(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<CopyInst>(src, dst, type));
    }
    void emplaceLoad(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<LoadInst>(src, dst, type));
    }
    void emplaceJump(const Identifier& iden)
    {
        insts.emplace_back(std::make_unique<JumpInst>(iden));
    }
    void emplaceGetAddress(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<GetAddressInst>(src, dst, type));
    }
    void emplaceJumpIfZero(std::shared_ptr<Value> src, const Identifier& iden)
    {
        insts.emplace_back(std::make_unique<JumpIfZeroInst>(src, iden));
    }
    void emplaceJumpIfNotZero(std::shared_ptr<Value> src, const Identifier& iden)
    {
        insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(src, iden));
    }
    void emplaceStore(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<StoreInst>(src, dst, type));
    }
    void emplaceDoubleToInt(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<DoubleToIntInst>(src, dst, type));
    }
    void emplaceDoubleToUInt(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<DoubleToUIntInst>(src, dst, type));
    }
    void emplaceIntToDouble(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<IntToDoubleInst>(src, dst, type));
    }
    void emplaceUIntToDouble(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<UIntToDoubleInst>(src, dst, type));
    }
    void emplaceTruncate(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<TruncateInst>(src, dst, type));
    }
    void emplaceSignExtend(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<SignExtendInst>(src, dst, type));
    }
    void emplaceZeroExtend(std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<ZeroExtendInst>(src, dst, type));
    }
    void emplaceUnary(const UnaryInst::Operation oper, std::shared_ptr<Value> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<UnaryInst>(oper, src, dst, type));
    }
    void emplaceFunCall(const Identifier& iden, std::vector<std::shared_ptr<Value>> src, std::shared_ptr<Value> dst, const Type type)
    {
        insts.emplace_back(std::make_unique<FunCallInst>(iden, src, dst, type));
    }
    void emplaceReturn(std::shared_ptr<Value> src, const Type type)
    {
        insts.emplace_back(std::make_unique<ReturnInst>(src, type));
    }
};
} // IR