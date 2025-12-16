#pragma once

#include "ASTParser.hpp"
#include "SymbolTable.hpp"
#include "ExprResult.hpp"
#include "IrType.hpp"
#include "TypeTable.hpp"

#include <unordered_set>

namespace Ir {

class GenerateIr {
    using Storage = Parsing::Declaration::StorageClass;

    bool m_global = true;
    std::vector<std::unique_ptr<Instruction>> m_insts;
    SymbolTable& m_symbolTable;
    std::unordered_set<std::string> m_writtenGlobals;
    std::vector<std::unique_ptr<TopLevel>> m_topLevels;
    std::unordered_map<std::string, IrStruct> m_irStructs;
    std::vector<std::unique_ptr<Value>> m_values;

    std::unordered_map<std::string, std::string> m_constStrings;

    const TypeTable& typeTable;
public:
    explicit GenerateIr(SymbolTable& symbolTable, const TypeTable& varTable)
        : m_symbolTable(symbolTable), typeTable(varTable) {}
    void program(const Parsing::Program& parsingProgram, Program& tackyProgram);
    std::unique_ptr<TopLevel> topLevelIr(const Parsing::Declaration& decl);
    std::unique_ptr<TopLevel> structuredDecl(const Parsing::StructuredDecl& structuredDecl);
    std::unique_ptr<TopLevel> functionIr(const Parsing::FuncDecl& parsingFunction);

    std::unique_ptr<TopLevel> staticVariableIr(const Parsing::VarDecl& varDecl);
    std::unique_ptr<TopLevel> genStaticWithoutInit(const Parsing::VarDecl& varDecl);
    std::unique_ptr<TopLevel> genCompoundInit(const Parsing::VarDecl& varDecl);
    std::unique_ptr<TopLevel> genStaticInit(const Parsing::VarDecl& varDecl, bool defined);
    std::vector<std::unique_ptr<Initializer>> genStaticCompoundInit(const Parsing::VarDecl& varDecl);
    const Value* genStaticVariableInit(const Parsing::VarDecl& varDecl);

    void genBlock(const Parsing::Block& block);
    void genBlockItem(const Parsing::BlockItem& blockItem);
    void genSingleDeclaration(const Parsing::VarDecl& varDecl);
    void genZeroLocalInit(const std::string& name,
                          i64 arraySize,
                          i64 alignment,
                          i64 lengthZeroInit,
                          i64& offset);
    void genSingleLocalInit(const std::string& name,
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
    const Value* genInstAndConvert(const Parsing::Expr& parsingExpr);
    const Value* castValue(const Value* result, IrType towards, IrType from);

    std::unique_ptr<ExprResult> genConstPlainOperand(const Parsing::ConstExpr& constExpr);
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
    void binaryPtrSubInst(const Value* lhs,
                          const Value* rhs,
                          const Value* dst,
                          i64 scale);
    std::unique_ptr<ExprResult> genBinaryPtrSubInst(const Parsing::BinaryExpr& binaryExpr);

    std::unique_ptr<ExprResult> genAssignInst(const Parsing::AssignmentExpr& assignmentExpr);
    void genCompoundAssignWithoutDeref(const Parsing::AssignmentExpr& assignmentExpr,
                                       const Value* rhs,
                                       const Value* lhs);
    std::unique_ptr<ExprResult> genTernaryInst(const Parsing::TernaryExpr& ternaryExpr);
    std::unique_ptr<ExprResult> genFuncCallInst(const Parsing::FuncCallExpr& funcCallExpr);
    std::unique_ptr<ExprResult> genAddrOfInst(const Parsing::AddrOffExpr& addrOffExpr);
    std::unique_ptr<ExprResult> genSubscriptInst(const Parsing::SubscriptExpr& subscriptExpr);
    std::unique_ptr<ExprResult> genDereferenceInst(const Parsing::DereferenceExpr& dereferenceExpr);
    std::unique_ptr<ExprResult> genSizeOfExprInst(const Parsing::SizeOfExprExpr& sizeOfExprExpr);
    std::unique_ptr<ExprResult> genSizeOfTypeInst(const Parsing::SizeOfTypeExpr& sizeOfTypeExpr);
    std::unique_ptr<ExprResult> genDotExprInst(const Parsing::DotExpr& dotExpr);
    std::unique_ptr<ExprResult> genArrowExprInst(const Parsing::ArrowExpr& arrowExpr);
    std::unique_ptr<ExprResult> genVarInst(const Parsing::VarExpr& varExpr);

    const Value* genConstValue(const Parsing::ConstExpr& constExpr);
    const Value* genZeroValueForType(Type type);
    const Value* genValueVar(const Identifier& iden, const IrType& type);
    const Value* genValueVar(const Identifier& iden, const IrType& type, ReferingTo referingTo);
    const Value* genConstValue(i8 constValue);
    const Value* genConstValue(u8 constValue);
    const Value* genConstValue(char constValue);
    const Value* genConstValue(i32 constValue);
    const Value* genConstValue(u32 constValue);
    const Value* genConstValue(i64 constValue);
    const Value* genConstValue(u64 constValue);
    const Value* genConstValue(double constValue);
private:
    void allocateLocal(const Parsing::VarDecl& varDecl);
    void directlyPushConstant32Bit(const Parsing::VarDecl& varDecl, const Value* value);

    const Value* zeroConst1 = genZeroValueForType(Type::U8);
    const Value* zeroConst4 = genZeroValueForType(Type::U32);
    const Value* zeroConst8 = genZeroValueForType(Type::U64);

    [[nodiscard]] i64 getPointerReferenceTypeSize(const Parsing::TypeBase* typeBase) const;
    const Value* getInrDecScale(const Parsing::UnaryExpr& unaryExpr, Type type);

    [[nodiscard]] IrType convert(const Parsing::TypeBase& typeBase) const
    {
        switch (typeBase.type) {
            case Type::Char:     return charType;
            case Type::I8:       return i8Type;
            case Type::U8:       return u8Type;
            case Type::I32:      return i32Type;
            case Type::U32:      return u32Type;
            case Type::I64:      return i64Type;
            case Type::U64:      return u64Type;
            case Type::Pointer:  return pointerType;
            case Type::Array:    return pointerType;
            case Type::Double:   return doubleType;
            case Type::Void:     return voidType;
            default:
                return IrType(typeTable.getSize(&typeBase));
        }
    }

    void emitReturn()
    {
        m_insts.emplace_back(std::make_unique<ReturnInst>(voidType));
    }
    void emitReturn(const Value* src, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<ReturnInst>(src, type));
    }
    void emitSignExtend(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<SignExtendInst>(src, dst, type));
    }
    void emitTruncate(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<TruncateInst>(src, dst, type));
    }
    void emitZeroExtend(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<ZeroExtendInst>(src, dst, type));
    }
    void emitDoubleToInt(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<DoubleToIntInst>(src, dst, type));
    }
    void emitDoubleToUInt(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<DoubleToUIntInst>(src, dst, type));
    }
    void emitIntToDouble(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<IntToDoubleInst>(src, dst, type));
    }
    void emitUIntToDouble(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<UIntToDoubleInst>(src, dst, type));
    }
    void emitUnary(const UnaryInst::Operation oper, const IrType type,
                   const Value* src, const Value* dst)
    {
        m_insts.emplace_back(std::make_unique<UnaryInst>(oper, src, dst, type));
    }
    void emitBinary(const BinaryInst::Operation oper, const IrType type,
                    const Value* lhs, const Value* rhs, const Value* dst)
    {
        m_insts.emplace_back(std::make_unique<BinaryInst>(oper, lhs, rhs, dst, type));
    }
    void emitCopy(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<CopyInst>(src, dst, type));
    }
    void emitGetAddress(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<GetAddressInst>(src, dst, type));
    }
    void emitLoad(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<LoadInst>(src, dst, type));
    }
    void emitStore(const Value* src, const Value* dst, const IrType type)
    {
        m_insts.emplace_back(std::make_unique<StoreInst>(src, dst, type));
    }
    void emitAddPtr(const Value* ptr, const Value* index, const Value* dst, const i64 scale)
    {
        m_insts.emplace_back(std::make_unique<AddPtrInst>(ptr, index, dst, scale));
    }
    void emitCopyToOffset(const Value* src,
                          const Identifier& iden,
                          const ReferingTo referingTo,
                          const i64 offset,
                          const i64 arraySize,
                          const i64 alignment,
                          const IrType type)
    {
        m_insts.emplace_back(std::make_unique<CopyToOffsetInst>(
            src, iden, referingTo, offset, arraySize, alignment, type));
    }
    void emitCopyFromOffset(const Identifier& iden,
                            const ReferingTo referingTo,
                            const Value* dst,
                            const i64 offset,
                            const IrType type)
    {
        m_insts.emplace_back(std::make_unique<CopyFromOffsetInst>(iden, referingTo, dst, offset, type));
    }
    void emitJump(const Identifier& iden)
    {
        m_insts.emplace_back(std::make_unique<JumpInst>(iden));
    }
    void emitJumpIfZero(const Value* src, const Identifier& iden)
    {
        m_insts.emplace_back(std::make_unique<JumpIfZeroInst>(src, iden));
    }
    void emitJumpIfNotZero(const Value* src, const Identifier& iden)
    {
        m_insts.emplace_back(std::make_unique<JumpIfNotZeroInst>(src, iden));
    }
    void emitLabel(const Identifier& iden)
    {
        m_insts.emplace_back(std::make_unique<LabelInst>(iden));
    }
    void emitFunCall(const Identifier& iden,
                     std::vector<const Value*>&& src,
                     const IrType type)
    {
        m_insts.emplace_back(std::make_unique<FunCallInst>(iden, std::move(src), type));
    }
    void emitFunCall(const Identifier& iden,
                     std::vector<const Value*>&& src,
                     const Value* dst,
                     const IrType type)
    {
        m_insts.emplace_back(std::make_unique<FunCallInst>(iden, std::move(src), dst, type));
    }
    void emitAllocate(const i64 size, const std::string& iden)
    {
        m_insts.emplace_back(std::make_unique<AllocateInst>(size, Identifier(iden)));
    }
};
} // IR