#pragma once

#include "ASTParser.hpp"
#include "SymbolTable.hpp"
#include "ExprResult.hpp"

#include <unordered_set>

namespace Ir {
class GenerateIr {
    using Storage = Parsing::Declaration::StorageClass;

    bool m_global = true;
    std::vector<std::unique_ptr<Instruction>> m_insts;
    SymbolTable& m_symbolTable;
    std::unordered_set<std::string> m_writtenGlobals;
    std::vector<std::unique_ptr<TopLevel>> m_topLevels;
public:
    explicit GenerateIr(SymbolTable& symbolTable)
        : m_symbolTable(symbolTable) {}
    void program(const Parsing::Program& parsingProgram, Program& tackyProgram);
    std::unique_ptr<TopLevel> topLevelIr(const Parsing::Declaration& decl);
    std::unique_ptr<TopLevel> functionIr(const Parsing::FuncDecl& parsingFunction);

    std::unique_ptr<TopLevel> staticVariableIr(const Parsing::VarDecl& varDecl);
    std::unique_ptr<TopLevel> genStaticArray(const Parsing::VarDecl& varDecl, bool defined);
    std::unique_ptr<TopLevel> genStaticInit(const Parsing::VarDecl& varDecl, bool defined);
    std::vector<std::unique_ptr<Initializer>> genStaticArrayInit(const Parsing::VarDecl& varDecl, bool defined);
    std::shared_ptr<Value> genStaticVariableInit(const Parsing::VarDecl& varDecl, bool defined);

    void genBlock(const Parsing::Block& block);
    void genBlockItem(const Parsing::BlockItem& blockItem);
    void genSingleDeclaration(const Parsing::VarDecl& varDecl);
    void genZeroLocalInit(const std::string& name,
                          Type type,
                          i64 arraySize,
                          i64 alignment,
                          i64 lengthZeroInit,
                          i64& offset,
                          const std::shared_ptr<Value>& zeroConst);
    void genSingleLocalInit(const std::string& name,
                            Type type,
                            i64 arraySize,
                            i64 alignment,
                            i64& offset,
                            const Parsing::SingleInitializer& singleInit);

    void genDeclaration(const Parsing::Declaration& decl);
    void genStaticLocal(const Parsing::VarDecl& varDecl);
    void genCompoundLocalInit(const Parsing::VarDecl& varDecl);

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
    std::shared_ptr<ValueVar> castValue(const std::shared_ptr<Value>& result, Type towards, Type from);

    static std::unique_ptr<ExprResult> genConstPlainOperand(const Parsing::ConstExpr& constExpr);
    std::unique_ptr<ExprResult> genStringPlainOperand(const Parsing::StringExpr& stringExpr);
    std::unique_ptr<ExprResult> genCastInst(const Parsing::CastExpr& castExpr);
    std::unique_ptr<ExprResult> genUnaryInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genUnaryBasicInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genUnaryPostfixInst(const Parsing::UnaryExpr& unaryExpr);
    std::unique_ptr<ExprResult> genUnaryPrefixInst(const Parsing::UnaryExpr& unaryExpr);

    std::unique_ptr<ExprResult> genBinaryInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genBinarySimpleInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genBinaryAndInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genBinaryOrInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genBinaryPtrInst(const Parsing::BinaryExpr& binaryExpr);
    std::unique_ptr<ExprResult> genBinaryPtrAddInst(const Parsing::BinaryExpr& binaryExpr);
    void binaryPtrSubInst(const std::shared_ptr<Value>& lhs,
                          const std::shared_ptr<Value>& rhs,
                          const std::shared_ptr<Value>& dst,
                          i64 scale);
    std::unique_ptr<ExprResult> genBinaryPtrSubInst(const Parsing::BinaryExpr& binaryExpr);

    std::unique_ptr<ExprResult> genAssignInst(const Parsing::AssignmentExpr& assignmentExpr);
    void genCompoundAssignWithoutDeref(const Parsing::AssignmentExpr& assignmentExpr,
                                       std::shared_ptr<Value>& rhs,
                                       const std::shared_ptr<Value>& lhs);
    std::unique_ptr<ExprResult> genTernaryInst(const Parsing::TernaryExpr& ternaryExpr);
    std::unique_ptr<ExprResult> genFuncCallInst(const Parsing::FuncCallExpr& funcCallExpr);
    std::unique_ptr<ExprResult> genAddrOfInst(const Parsing::AddrOffExpr& addrOffExpr);
    std::unique_ptr<ExprResult> genSubscriptInst(const Parsing::SubscriptExpr& subscriptExpr);
    std::unique_ptr<ExprResult> genDereferenceInst(const Parsing::DereferenceExpr& dereferenceExpr);
    static std::unique_ptr<ExprResult> genSizeOfExprInst(const Parsing::SizeOfExprExpr& sizeOfExprExpr);
    static std::unique_ptr<ExprResult> genSizeOfTypeInst(const Parsing::SizeOfTypeExpr& sizeOfTypeExpr);
    static std::unique_ptr<ExprResult> genVarInst(const Parsing::VarExpr& varExpr);

private:
    void allocateLocalArrayWithoutInitializer(const Parsing::VarDecl& varDecl);
    void directlyPushConstant32Bit(const Parsing::VarDecl& varDecl, const std::shared_ptr<Value>& value);

    void emplaceReturn()
    {
        m_insts.emplace_back(std::make_unique<ReturnInst>(Type::Void));
    }
    void emplaceReturn(const std::shared_ptr<Value>& src, const Type type)
    {
        m_insts.emplace_back(std::make_unique<ReturnInst>(src, type));
    }
    void emplaceSignExtend(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<SignExtendInst>(src, dst, type));
    }
    void emplaceTruncate(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<TruncateInst>(src, dst, type));
    }
    void emplaceZeroExtend(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<ZeroExtendInst>(src, dst, type));
    }
    void emplaceDoubleToInt(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<DoubleToIntInst>(src, dst, type));
    }
    void emplaceDoubleToUInt(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<DoubleToUIntInst>(src, dst, type));
    }
    void emplaceIntToDouble(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<IntToDoubleInst>(src, dst, type));
    }
    void emplaceUIntToDouble(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<UIntToDoubleInst>(src, dst, type));
    }
    void emplaceUnary(const UnaryInst::Operation oper, const std::shared_ptr<Value>& src,
                      const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<UnaryInst>(oper, src, dst, type));
    }
    void emplaceBinary(const BinaryInst::Operation oper,
                       const std::shared_ptr<Value>& lhs,
                       const std::shared_ptr<Value>& rhs,
                       const std::shared_ptr<Value>& dst,
                       const Type type)
    {
        m_insts.emplace_back(std::make_unique<BinaryInst>(oper, lhs, rhs, dst, type));
    }
    void emplaceCopy(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<CopyInst>(src, dst, type));
    }
    void emplaceGetAddress(const std::shared_ptr<Value>& src,
                           const std::shared_ptr<Value>& dst,
                           const Type type)
    {
        m_insts.emplace_back(std::make_unique<GetAddressInst>(src, dst, type));
    }
    void emplaceLoad(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<LoadInst>(src, dst, type));
    }
    void emplaceStore(const std::shared_ptr<Value>& src, const std::shared_ptr<Value>& dst, const Type type)
    {
        m_insts.emplace_back(std::make_unique<StoreInst>(src, dst, type));
    }
    void emplaceAddPtr(const std::shared_ptr<Value>& ptr,
                       const std::shared_ptr<Value>& index,
                       const std::shared_ptr<Value>& dst,
                       const i64 scale)
    {
        m_insts.emplace_back(std::make_unique<AddPtrInst>(ptr, index, dst, scale));
    }
    void emplaceCopyToOffset(const std::shared_ptr<Value>& src,
                             const Identifier& iden,
                             const i64 offset,
                             const i64 arraySize,
                             const i64 alignment,
                             const Type type)
    {
        m_insts.emplace_back(std::make_unique<CopyToOffsetInst>(src, iden, offset, arraySize, alignment, type));
    }
    void emplaceJump(const Identifier& iden)
    {
        m_insts.emplace_back(std::make_unique<JumpInst>(iden));
    }
    void emplaceJumpIfZero(const std::shared_ptr<Value>& src, const Identifier& iden)
    {
        m_insts.emplace_back(std::make_unique<JumpIfZeroInst>(src, iden));
    }
    void emplaceJumpIfNotZero(const std::shared_ptr<Value>& src, const Identifier& iden)
    {
        m_insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(src, iden));
    }
    void emplaceLabel(const Identifier& iden)
    {
        m_insts.emplace_back(std::make_unique<LabelInst>(iden));
    }
    void emplaceFunCall(const Identifier& iden,
                    std::vector<std::shared_ptr<Value>>&& src,
                    const Type type)
    {
        m_insts.emplace_back(std::make_unique<FunCallInst>(iden, src, type));
    }
    void emplaceFunCall(const Identifier& iden,
                        std::vector<std::shared_ptr<Value>>&& src,
                        const std::shared_ptr<Value>& dst,
                        const Type type)
    {
        m_insts.emplace_back(std::make_unique<FunCallInst>(iden, src, dst, type));
    }
    void emplaceAllocate(const i64 size, const std::string& iden, const Type type)
    {
        m_insts.emplace_back(std::make_unique<AllocateInst>(size, Identifier(iden), type));
    }
};

i64 getReferencedTypeSize(Parsing::TypeBase* typeBase);
std::shared_ptr<ValueConst> getInrDecScale(const Parsing::UnaryExpr& unaryExpr, Type type);
} // IR