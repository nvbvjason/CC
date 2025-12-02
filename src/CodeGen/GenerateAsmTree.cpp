#include "GenerateAsmTree.hpp"
#include "AsmAST.hpp"
#include "DynCast.hpp"
#include "FixUpInstructions.hpp"
#include "Operators.hpp"
#include "Types/TypeConversion.hpp"

#include <array>
#include <cassert>

namespace {
using RegType = CodeGen::Operand::RegKind;
constexpr std::array intRegs = {RegType::DI, RegType::SI, RegType::DX,
                                RegType::CX, RegType::R8, RegType::R9};
constexpr std::array doubleRegs = {RegType::XMM0, RegType::XMM1, RegType::XMM2, RegType::XMM3,
                                   RegType::XMM4, RegType::XMM5, RegType::XMM6, RegType::XMM7};
}

namespace CodeGen {

void GenerateAsmTree::genProgram(const Ir::Program &program, Program &programCodegen)
{
    m_toplevel.clear();
    for (const auto& toplevelIr : program.topLevels) {
        std::unique_ptr<TopLevel> topLevel = genTopLevel(*toplevelIr);
        m_toplevel.emplace_back(std::move(topLevel));
    }
    programCodegen.topLevels = std::move(m_toplevel);
}

std::unique_ptr<TopLevel> GenerateAsmTree::genTopLevel(const Ir::TopLevel& topLevel)
{
    using Type = Ir::TopLevel::Kind;
    switch (topLevel.kind) {
        case Type::Function: {
            const auto function = dynCast<const Ir::Function>(&topLevel);
            return genFunction(*function);
        }
        case Type::StaticVariable: {
            const auto staticVariable = dynCast<const Ir::StaticVariable>(&topLevel);
            return genStaticVariable(*staticVariable);
        }
        case Type::StaticArray: {
            const auto staticArray = dynCast<const Ir::StaticArray>(&topLevel);
            return genStaticArray(*staticArray);
        }
        case Type::StaticConstant: {
            const auto staticConstant = dynCast<const Ir::StaticConstant>(&topLevel);
            return genStaticString(*staticConstant);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<TopLevel> GenerateAsmTree::genFunction(const Ir::Function& function)
{
    auto functionCodeGen = std::make_unique<Function>(function.name, function.isGlobal);
    insts.clear();
    const std::vector<bool> pushedIntoRegs = genFunctionPushIntoRegs(function);
    genFunctionPushOntoStack(function, pushedIntoRegs);
    for (const std::unique_ptr<Ir::Instruction>& inst : function.insts)
        genInst(inst);
    functionCodeGen->instructions = std::move(insts);
    return functionCodeGen;
}

std::vector<bool> GenerateAsmTree::genFunctionPushIntoRegs(const Ir::Function& function)
{
    std::vector pushedIntoRegs(function.args.size(), false);
    i32 regIntIndex = 0;
    i32 regDoubleInex = 0;
    for (size_t i = 0; i < function.args.size(); ++i) {
        const AsmType type = getAsmType(function.argTypes[i]);
        std::shared_ptr<RegisterOperand> src;
        if (type != asmDouble && regIntIndex < intRegs.size())
            src = std::make_shared<RegisterOperand>(intRegs[regIntIndex++], type);
        else if (type == asmDouble && regDoubleInex < doubleRegs.size())
            src = std::make_shared<RegisterOperand>(doubleRegs[regDoubleInex++], type);
        else
            continue;
        auto arg = std::make_shared<Ir::ValueVar>(function.args[i], function.argTypes[i]);
        std::shared_ptr<Operand> dst = genOperand(*arg);
        emitMove(src, dst, type);
        pushedIntoRegs[i] = true;
    }
    return pushedIntoRegs;
}

void GenerateAsmTree::genFunctionPushOntoStack(const Ir::Function& function, std::vector<bool> pushedIntoRegs)
{
    i32 stackPtr = 2;
    for (size_t i = 0; i < function.args.size(); ++i) {
        if (pushedIntoRegs[i])
            continue;
        constexpr i32 stackAlignment = 8;
        auto stack = std::make_shared<MemoryOperand>(
            RegType::BP, stackAlignment * stackPtr++,
            getAsmType(function.argTypes[i]));
        auto arg = std::make_shared<Ir::ValueVar>(function.args[i], function.argTypes[i]);
        std::shared_ptr<Operand> dst = genOperand(*arg);
        emitMove(stack, dst, getAsmType(function.argTypes[i]));
    }
}

u64 getSingleInitValue(const Ir::IrType::Kind type, const Ir::ValueConst* const value)
{
    using IrKind = Ir::IrType::Kind;
    switch (type) {
        case IrKind::Char:    return std::get<char>(value->value);
        case IrKind::I8:      return std::get<i8>(value->value);
        case IrKind::U8:      return std::get<u8>(value->value);
        case IrKind::I32:     return std::get<i32>(value->value);
        case IrKind::U32:     return std::get<u32>(value->value);
        case IrKind::I64:     return std::get<i64>(value->value);
        case IrKind::Pointer: return std::get<u64>(value->value);
        case IrKind::U64:     return std::get<u64>(value->value);
        case IrKind::Double: {
            const double init = std::get<double>(value->value);
            return std::bit_cast<i64>(init);
        }
        default:
            std::abort();
    }
}

std::unique_ptr<TopLevel> genStaticString(const Ir::StaticConstant& staticConstant)
{
    return std::make_unique<StringVariable>(
        staticConstant.identifier.value,
        staticConstant.value,
        staticConstant.global,
        staticConstant.nullTerminated
    );
}

std::unique_ptr<TopLevel> genStaticVariable(const Ir::StaticVariable& staticVariable)
{
    const auto value = dynCast<const Ir::ValueConst>(staticVariable.value.get());
    auto result = std::make_unique<StaticVariable>(
        staticVariable.name, getAsmType(staticVariable.type), staticVariable.global);
    result->init = getSingleInitValue(staticVariable.type.kind, value);
    return result;
}

std::unique_ptr<TopLevel> genStaticArray(const Ir::StaticArray& staticArray)
{
    std::vector<std::unique_ptr<Initializer>> initializers;
    for (const auto& init : staticArray.initializers) {
        switch (init->kind) {
            case Ir::Initializer::Kind::Value: {
                const auto value = dynCast<Ir::ValueInitializer>(init.get());
                const auto constValue = dynCast<Ir::ValueConst>(value->value.get());
                initializers.emplace_back(std::make_unique<ValueInitializer>(
                    getSingleInitValue(constValue->type.kind, constValue), getAsmType(constValue->type)));
                break;
            }
            case Ir::Initializer::Kind::Zero: {
                const auto zero = dynCast<Ir::ZeroInitializer>(init.get());
                initializers.emplace_back(std::make_unique<ZeroInitializer>(zero->size));
                break;
            }
        }
    }
    return std::make_unique<CompoundVariable>(
        Identifier(staticArray.name), 16, std::move(initializers), staticArray.global);
}

void GenerateAsmTree::genInst(const std::unique_ptr<Ir::Instruction>& inst)
{
    using Kind = Ir::Instruction::Kind;
    switch (inst->kind) {
        case Kind::Return: {
            const auto irReturn = dynCast<const Ir::ReturnInst>(inst.get());
            genReturn(*irReturn);
            break;
        }
        case Kind::SignExtend: {
            const auto signExtend = dynCast<const Ir::SignExtendInst>(inst.get());
            genSignExtend(*signExtend);
            break;
        }
        case Kind::Truncate: {
            const auto truncate = dynCast<const Ir::TruncateInst>(inst.get());
            genTruncate(*truncate);
            break;
        }
        case Kind::ZeroExtend: {
            const auto zeroExtend = dynCast<const Ir::ZeroExtendInst>(inst.get());
            genZeroExtend(*zeroExtend);
            break;
        }
        case Kind::DoubleToInt: {
            const auto doubleToInt = dynCast<const Ir::DoubleToIntInst>(inst.get());
            genDoubleToInt(*doubleToInt);
            break;
        }
        case Kind::DoubleToUInt: {
            const auto doubleToUInt = dynCast<const Ir::DoubleToUIntInst>(inst.get());
            genDoubleToUInt(*doubleToUInt);
            break;
        }
        case Kind::IntToDouble: {
            const auto intToDouble = dynCast<const Ir::IntToDoubleInst>(inst.get());
            genIntToDouble(*intToDouble);
            break;
        }
        case Kind::UIntToDouble: {
            const auto uIntToDouble = dynCast<const Ir::UIntToDoubleInst>(inst.get());
            genUIntToDouble(*uIntToDouble);
            break;
        }
        case Kind::Unary: {
            const auto irUnary = dynCast<const Ir::UnaryInst>(inst.get());
            genUnary(*irUnary);
            break;
        }
        case Kind::Binary: {
            const auto irBinary = dynCast<const Ir::BinaryInst>(inst.get());
            genBinary(*irBinary);
            break;
        }
        case Kind::Copy: {
            const auto irCopy = dynCast<const Ir::CopyInst>(inst.get());
            genCopy(*irCopy);
            break;
        }
        case Kind::Jump: {
            const auto irJump = dynCast<const Ir::JumpInst>(inst.get());
            genJump(*irJump);
            break;
        }
        case Kind::JumpIfZero: {
            const auto irJumpIfZero = dynCast<const Ir::JumpIfZeroInst>(inst.get());
            genJumpIfZero(*irJumpIfZero);
            break;
        }
        case Kind::JumpIfNotZero: {
            const auto irJumpIfNotZero = dynCast<const Ir::JumpIfNotZeroInst>(inst.get());
            genJumpIfNotZero(*irJumpIfNotZero);
            break;
        }
        case Kind::Label: {
            const auto irLabel = dynCast<const Ir::LabelInst>(inst.get());
            genLabel(*irLabel);
            break;
        }
        case Kind::FunCall: {
            const auto irFunCall = dynCast<const Ir::FunCallInst>(inst.get());
            genFunCall(*irFunCall);
            break;
        }
        case Kind::Store: {
            const auto irStore = dynCast<const Ir::StoreInst>(inst.get());
            genStore(*irStore);
            break;
        }
        case Kind::Load: {
            const auto irLoad = dynCast<const Ir::LoadInst>(inst.get());
            genLoad(*irLoad);
            break;
        }
        case Kind::GetAddress: {
            const auto irGetAddress = dynCast<const Ir::GetAddressInst>(inst.get());
            genGetAddress(*irGetAddress);
            break;
        }
        case Kind::AddPtr: {
            const auto irAddPtr = dynCast<const Ir::AddPtrInst>(inst.get());
            genAddPtr(*irAddPtr);
            break;
        }
        case Kind::CopyToOffset: {
            const auto irCopyToOffset = dynCast<const Ir::CopyToOffsetInst>(inst.get());
            genCopyToOffSet(*irCopyToOffset);
            break;
        }
        case Kind::CopyFromOffset: {
            const auto irCopyFromOffset = dynCast<const Ir::CopyFromOffsetInst>(inst.get());
            genCopyFromOffset(*irCopyFromOffset);
            break;
        }
        case Kind::Allocate: {
            const auto allocate = dynCast<const Ir::AllocateInst>(inst.get());
            genAllocate(*allocate);
            break;
        }
        default:
            std::abort();
    }
}

void GenerateAsmTree::genJump(const Ir::JumpInst& irJump)
{
    const Identifier iden(irJump.target.value);
    emitJmp(iden);
}

void GenerateAsmTree::genJumpIfZero(const Ir::JumpIfZeroInst& jumpIfZero)
{
    if (jumpIfZero.type != Ir::doubleType) {
        genJumpIfZeroInteger(jumpIfZero);
        return;
    }
    genJumpIfZeroDouble(jumpIfZero);
}

void GenerateAsmTree::genJumpIfZeroDouble(const Ir::JumpIfZeroInst& jumpIfZero)
{
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, asmDouble);
    const std::shared_ptr<Operand> condition = genOperand(*jumpIfZero.condition);
    const Identifier target(jumpIfZero.target.value);
    const Identifier endLabel(makeTemporaryPseudoName());

    zeroOutReg(xmm0);

    emitCmp(condition, xmm0, asmDouble);
    emitJmpCC(Inst::CondCode::PF, endLabel);
    emitJmpCC(Inst::CondCode::E, target);
    emitLabel(endLabel);
}

void GenerateAsmTree::genJumpIfZeroInteger(const Ir::JumpIfZeroInst& jumpIfZero)
{
    const std::shared_ptr<Operand> condition = genOperand(*jumpIfZero.condition);
    const std::shared_ptr<Operand> zero = getZeroOperand(condition->type);
    const Identifier target(jumpIfZero.target.value);

    emitCmp(zero, condition, condition->type);
    emitJmpCC(BinaryInst::CondCode::E, target);
}

void GenerateAsmTree::genJumpIfNotZero(const Ir::JumpIfNotZeroInst& jumpIfNotZero)
{
    if (jumpIfNotZero.type != Ir::doubleType) {
        genJumpIfNotZeroInteger(jumpIfNotZero);
        return;
    }
    genJumpIfNotZeroDouble(jumpIfNotZero);
}

void GenerateAsmTree::genJumpIfNotZeroDouble(const Ir::JumpIfNotZeroInst& jumpIfNotZero)
{
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, asmDouble);
    const std::shared_ptr<Operand> condition = genOperand(*jumpIfNotZero.condition);
    const Identifier target(jumpIfNotZero.target.value);

    zeroOutReg(xmm0);

    emitCmp(condition, xmm0, asmDouble);
    emitJmpCC(Inst::CondCode::PF, target);
    emitJmpCC(Inst::CondCode::NE, target);
}

void GenerateAsmTree::genJumpIfNotZeroInteger(const Ir::JumpIfNotZeroInst& jumpIfNotZero)
{
    const std::shared_ptr<Operand> condition = genOperand(*jumpIfNotZero.condition);
    const std::shared_ptr<Operand> zero = getZeroOperand(condition->type);
    const Identifier target(jumpIfNotZero.target.value);

    emitCmp(zero, condition, condition->type);
    emitJmpCC(Inst::CondCode::NE, target);
}

void GenerateAsmTree::genMove(
    i64 offset,
    i64 size,
    const AsmType type,
    const Ir::ValueVar& srcValue,
    const Ir::ValueVar& dstValue)
{
    const auto src = std::make_shared<PseudoMemOperand>(
        Identifier(srcValue.value.value),
        offset,
        size,
        0,
        srcValue.referingTo == ReferingTo::Local,
        type);
    const auto dst = std::make_shared<PseudoMemOperand>(
        Identifier(dstValue.value.value),
        offset,
        size,
        0,
        dstValue.referingTo == ReferingTo::Local,
        type);
    emitMove(src, dst, src->type);
}

void GenerateAsmTree::genCopyByteArray(const Ir::ValueVar& src, const Ir::ValueVar& dst, const i64 size)
{
    i64 i = 0;
    for (; i + 8 <= size; i += 8)
        genMove(i, 8, asmQuadWord, src, dst);
    for (; i + 4 <= size; i += 4)
        genMove(i, 4, asmLongWord, src, dst);
    for (; i + 2 <= size; i += 2)
        genMove(i, 2, asmWord, src, dst);
    for (; i < size; ++i)
        genMove(i, 1, asmByte, src, dst);
}

void GenerateAsmTree::genCopy(const Ir::CopyInst& copy)
{
    if (copy.type.kind != Ir::IrType::Kind::ByteArray) {
        const std::shared_ptr<Operand> src = genOperand(*copy.src);
        const std::shared_ptr<Operand> dst = genOperand(*copy.dst);
        emitMove(src, dst, src->type);
        return;
    }
    const AsmType type = getAsmType(copy.type);
    const auto src = dynCast<const Ir::ValueVar>(copy.src.get());
    const auto dst = dynCast<const Ir::ValueVar>(copy.dst.get());
    genCopyByteArray(*src, *dst, type.size);
}

void GenerateAsmTree::genGetAddress(const Ir::GetAddressInst& getAddress)
{
    const std::shared_ptr<Operand> src = genOperand(*getAddress.src);
    const std::shared_ptr<Operand> dst = genOperand(*getAddress.dst);
    emitLea(src, dst, asmQuadWord);
}

void GenerateAsmTree::genLoad(const Ir::LoadInst& load)
{
    const std::shared_ptr<Operand> ptr = genOperand(*load.ptr);
    const std::shared_ptr<Operand> dst = genOperand(*load.dst);
    const auto rax = std::make_shared<RegisterOperand>(RegType::DX, asmQuadWord);
    const auto memory = std::make_shared<MemoryOperand>(RegType::DX, 0, asmQuadWord);

    emitMove(ptr, rax, asmQuadWord);
    if (load.type.kind == Ir::IrType::Kind::ByteArray) {
        const auto asmType = getAsmType(load.type);
        const auto src = dynCast<const Ir::ValueVar>(load.ptr.get());
        const auto dstValue = dynCast<const Ir::ValueVar>(load.dst.get());
        genCopyByteArray(*src, *dstValue, asmType.size);
        return;
    }
    emitMove(memory, dst, dst->type);
}

void GenerateAsmTree::genStore(const Ir::StoreInst& store)
{
    const std::shared_ptr<Operand> src = genOperand(*store.src);
    const std::shared_ptr<Operand> ptr = genOperand(*store.ptr);
    const auto rax = std::make_shared<RegisterOperand>(RegType::DX, asmQuadWord);
    const auto memory = std::make_shared<MemoryOperand>(RegType::DX, 0, asmQuadWord);

    emitMove(ptr, rax, asmQuadWord);
    emitMove(src, memory, src->type);
}

void GenerateAsmTree::genLabel(const Ir::LabelInst& irLabel)
{
    const Identifier label(irLabel.target.value);
    emitLabel(label);
}

void GenerateAsmTree::genUnary(const Ir::UnaryInst& irUnary)
{
    if (irUnary.operation == Ir::UnaryInst::Operation::Not) {
        genUnaryNot(irUnary);
        return;
    }
    if (irUnary.operation == Ir::UnaryInst::Operation::Negate &&
        irUnary.type == Ir::doubleType) {
        genNegateDouble(irUnary);
        return;
    }
    genUnaryBasic(irUnary);
}

void GenerateAsmTree::genUnaryBasic(const Ir::UnaryInst& irUnary)
{
    const std::shared_ptr<Operand> src = genOperand(*irUnary.src);
    const std::shared_ptr<Operand> dst = genOperand(*irUnary.dst);
    const UnaryInst::Operator oper = unaryOperator(irUnary.operation);

    emitMove(src, dst, src->type);
    emitUnary(dst, oper, src->type);
}

void GenerateAsmTree::genNegateDouble(const Ir::UnaryInst& irUnary)
{
    using Operator = BinaryInst::Operator;
    const std::shared_ptr<Operand> src = genOperand(*irUnary.src);
    const std::shared_ptr<Operand> rhs = genOperand(*irUnary.dst);
    const std::shared_ptr<Operand> lhs = genDoubleLocalConst(-0.0, 16);

    emitMove(src, rhs, src->type);
    emitBinary(lhs, rhs, Operator::BitwiseXor, asmDouble);
}

void GenerateAsmTree::genUnaryNot(const Ir::UnaryInst& irUnary)
{
    if (irUnary.src->type != Ir::doubleType)
        genUnaryNotInteger(irUnary);
    else
        genUnaryNotDouble(irUnary);
}

void GenerateAsmTree::genUnaryNotDouble(const Ir::UnaryInst& irUnary)
{
    const std::shared_ptr<Operand> src = genOperand(*irUnary.src);
    const std::shared_ptr<Operand> dst = genOperand(*irUnary.dst);
    const auto zero = getZeroOperand(dst->type);
    const auto one = std::make_shared<ImmOperand>(1, asmLongWord);
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, asmDouble);
    const Identifier nanLabel(makeTemporaryPseudoName() + "nanUnaryNot");
    const Identifier endLabel(makeTemporaryPseudoName());

    zeroOutReg(xmm0);

    emitCmp(src, xmm0, asmDouble);
    emitJmpCC(BinaryInst::CondCode::PF, nanLabel);
    emitMove(zero, dst, dst->type);
    emitSetCC(BinaryInst::CondCode::E, dst);
    emitJmp(endLabel);
    emitLabel(nanLabel);
    emitMove(zero, dst, dst->type);
    emitLabel(endLabel);
}

void GenerateAsmTree::genUnaryNotInteger(const Ir::UnaryInst& irUnary)
{
    const std::shared_ptr<Operand> src = genOperand(*irUnary.src);
    const std::shared_ptr<Operand> dst = genOperand(*irUnary.dst);
    const std::shared_ptr<Operand> zero = getZeroOperand(src->type);

    emitCmp(zero, src, src->type);
    emitMove(zero, dst, dst->type);
    emitSetCC(BinaryInst::CondCode::E, dst);
}

void GenerateAsmTree::genZeroExtend(const Ir::ZeroExtendInst& zeroExtend)
{
    const std::shared_ptr<Operand> src = genOperand(*zeroExtend.src);
    const std::shared_ptr<Operand> dst = genOperand(*zeroExtend.dst);
    emitMoveZeroExtend(src, dst, src->type, dst->type);
}

void GenerateAsmTree::genDoubleToInt(const Ir::DoubleToIntInst& doubleToInt)
{
    const std::shared_ptr<Operand> src = genOperand(*doubleToInt.src);
    const std::shared_ptr<Operand> dst = genOperand(*doubleToInt.dst);

    if (dst->type.size < 4) {
        const auto raxLongWord = std::make_shared<RegisterOperand>(RegType::AX, asmLongWord);
        emitCvttsd2si(src, raxLongWord, asmLongWord);
        const auto raxByte = std::make_shared<RegisterOperand>(RegType::AX, asmByte);
        emitMove(raxByte, dst, dst->type);
    }
    else
        emitCvttsd2si(src, dst, dst->type);
}

void GenerateAsmTree::genDoubleToUInt(const Ir::DoubleToUIntInst& doubleToUInt)
{
    switch (doubleToUInt.type.kind) {
        case Ir::IrType::Kind::U8:
            genDoubleToUIntByte(doubleToUInt);
            break;
        case Ir::IrType::Kind::U32:
            genDoubleToUIntLong(doubleToUInt);
            break;
        case Ir::IrType::Kind::U64:
            genDoubleToUIntQuad(doubleToUInt);
            break;
        default:
            std::abort();
    }
}
void GenerateAsmTree::genDoubleToUIntByte(const Ir::DoubleToUIntInst& doubleToUInt)
{
    const std::shared_ptr<Operand> src = genOperand(*doubleToUInt.src);
    const std::shared_ptr<Operand> dst = genOperand(*doubleToUInt.dst);
    const auto rax = std::make_shared<RegisterOperand>(RegType::AX, asmLongWord);
    const auto eax = std::make_shared<RegisterOperand>(RegType::AX, asmByte);

    emitCvttsd2si(src, rax, asmLongWord);
    emitMove(eax, dst, dst->type);
}

void GenerateAsmTree::genDoubleToUIntLong(const Ir::DoubleToUIntInst& doubleToUInt)
{
    const std::shared_ptr<Operand> src = genOperand(*doubleToUInt.src);
    const std::shared_ptr<Operand> dst = genOperand(*doubleToUInt.dst);
    const auto rax = std::make_shared<RegisterOperand>(RegType::AX, asmQuadWord);
    const auto eax = std::make_shared<RegisterOperand>(RegType::AX, asmLongWord);

    emitCvttsd2si(src, rax, asmQuadWord);
    emitMove(eax, dst, dst->type);
}

void GenerateAsmTree::genDoubleToUIntQuad(const Ir::DoubleToUIntInst& doubleToUInt)
{
    constexpr double upperBoundConst = 9223372036854775808.0;
    const std::shared_ptr<Operand> upperBound = genDoubleLocalConst(upperBoundConst, 8);
    const std::shared_ptr<Operand> src = genOperand(*doubleToUInt.src);
    const std::shared_ptr<Operand> dst = genOperand(*doubleToUInt.dst);
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, asmDouble);
    const auto xmm1 = std::make_shared<RegisterOperand>(RegType::XMM1, asmDouble);
    const Identifier labelOne(makeTemporaryPseudoName());
    const Identifier labelTwo(makeTemporaryPseudoName());

    emitCmp(upperBound, src, asmDouble);
    emitJmpCC(Inst::CondCode::AE, labelOne);
    emitCvttsd2si(src, dst, asmQuadWord);
    emitJmp(labelTwo);
    emitLabel(labelOne);
    emitMove(src, xmm1, asmDouble);
    emitBinary(upperBound, xmm1, BinaryInst::Operator::Sub, asmDouble);
    emitCvttsd2si(xmm1, dst, asmQuadWord);
    emitMove(upperBound, xmm0, asmQuadWord);
    emitLabel(labelTwo);
}

void GenerateAsmTree::genIntToDouble(const Ir::IntToDoubleInst& intToDouble)
{
    const std::shared_ptr<Operand> src = genOperand(*intToDouble.src);
    const std::shared_ptr<Operand> dst = genOperand(*intToDouble.dst);

    if (src->type.size < 4) {
        const auto rax = std::make_shared<RegisterOperand>(RegType::AX, asmLongWord);
        emitMoveSX(src, rax, asmByte, asmLongWord);
        emitCvtsi2sd(rax, dst, asmLongWord);
    }
    else
        emitCvtsi2sd(src, dst, src->type);
}

void GenerateAsmTree::genUIntToDouble(const Ir::UIntToDoubleInst& uintToDouble)
{
    switch (uintToDouble.src->type.kind) {
        case Ir::IrType::Kind::U8:
            genUIntToDoubleByte(uintToDouble);
            break;
        case Ir::IrType::Kind::U32:
            genUIntToDoubleLong(uintToDouble);
            break;
        case Ir::IrType::Kind::U64:
            genUIntToDoubleQuad(uintToDouble);
            break;
        default:
            std::abort();
    }
}

void GenerateAsmTree::genUIntToDoubleByte(const Ir::UIntToDoubleInst& uintToDouble)
{
    const std::shared_ptr<Operand> src = genOperand(*uintToDouble.src);
    const std::shared_ptr<Operand> rax = std::make_unique<RegisterOperand>(RegType::AX, asmLongWord);
    const std::shared_ptr<Operand> dst = genOperand(*uintToDouble.dst);

    emitMoveZeroExtend(src, rax, asmByte, asmLongWord);
    emitCvtsi2sd(rax, dst, asmLongWord);
}

void GenerateAsmTree::genUIntToDoubleLong(const Ir::UIntToDoubleInst& uintToDouble)
{
    const std::shared_ptr<Operand> src = genOperand(*uintToDouble.src);
    const std::shared_ptr<Operand> rax = std::make_unique<RegisterOperand>(RegType::AX, asmQuadWord);
    const std::shared_ptr<Operand> dst = genOperand(*uintToDouble.dst);

    emitMoveZeroExtend(src, rax, asmLongWord, asmQuadWord);
    emitCvtsi2sd(rax, dst, asmQuadWord);
}

void GenerateAsmTree::genUIntToDoubleQuad(const Ir::UIntToDoubleInst& uintToDouble)
{
    using UnaryOper = UnaryInst::Operator;
    using BinaryOper = BinaryInst::Operator;

    const std::shared_ptr<Operand> zero = getZeroOperand(asmQuadWord);
    const std::shared_ptr<Operand> src = genOperand(*uintToDouble.src);
    const Identifier labelOutOfRange(makeTemporaryPseudoName());
    const std::shared_ptr<Operand> dst = genOperand(*uintToDouble.dst);
    const Identifier labelEnd(makeTemporaryPseudoName());
    const auto rax = std::make_shared<RegisterOperand>(RegType::AX, asmQuadWord);
    const auto rdx = std::make_shared<RegisterOperand>(RegType::DX, asmQuadWord);
    const auto one = std::make_shared<ImmOperand>(1l, asmQuadWord);

    emitCmp(zero, src, asmQuadWord);
    emitJmpCC(Inst::CondCode::L, labelOutOfRange);
    emitCvtsi2sd(src, dst, asmQuadWord);
    emitJmp(labelEnd);
    emitLabel(labelOutOfRange);
    emitMove(src, rdx, asmQuadWord);
    emitMove(rdx, rax, asmQuadWord);
    emitUnary(rdx, UnaryOper::Shr, asmQuadWord);
    emitBinary(one, rax, BinaryOper::BitwiseAnd, asmQuadWord);
    emitBinary(rax, rdx, BinaryOper::BitwiseOr, asmQuadWord);
    emitCvtsi2sd(rdx, dst, asmQuadWord);
    emitBinary(dst, dst, BinaryOper::Add, asmDouble);
    emitLabel(labelEnd);
}

void GenerateAsmTree::genSignExtend(const Ir::SignExtendInst& signExtend)
{
    const std::shared_ptr<Operand> src = genOperand(*signExtend.src);
    const std::shared_ptr<Operand> dst = genOperand(*signExtend.dst);
    emitMoveSX(src, dst, src->type, dst->type);
}

void GenerateAsmTree::genTruncate(const Ir::TruncateInst& truncate)
{
    const std::shared_ptr<Operand> src = genOperand(*truncate.src);
    const std::shared_ptr<Operand> dst = genOperand(*truncate.dst);
    emitMove(src, dst, dst->type);
}

void GenerateAsmTree::genBinary(const Ir::BinaryInst& irBinary)
{
    using IrOper = Ir::BinaryInst::Operation;
    switch (irBinary.operation) {
        case IrOper::Add:
        case IrOper::Subtract:
        case IrOper::Multiply:
        case IrOper::BitwiseAnd:
        case IrOper::BitwiseOr:
        case IrOper::BitwiseXor:
            genBinaryBasic(irBinary);
            break;
        case IrOper::LeftShift:
        case IrOper::RightShift:
            genBinaryShift(irBinary);
            break;
        case IrOper::Divide:
            genBinaryDivide(irBinary);
            break;
        case IrOper::Remainder:
            genBinaryRemainder(irBinary);
            break;
        case IrOper::Equal:
        case IrOper::NotEqual:
        case IrOper::LessThan:
        case IrOper::LessOrEqual:
        case IrOper::GreaterThan:
        case IrOper::GreaterOrEqual:
            genBinaryCond(irBinary);
            break;
        default:
            std::abort();
    }
}

void GenerateAsmTree::genBinaryCond(const Ir::BinaryInst& irBinary)
{
    if (irBinary.lhs->type == Ir::doubleType) {
        genBinaryCondDouble(irBinary);
        return;
    }
    genBinaryCondInteger(irBinary);
}

void GenerateAsmTree::genBinaryCondInteger(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(*irBinary.lhs);
    const std::shared_ptr<Operand> rhs = genOperand(*irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(*irBinary.dst);
    const std::shared_ptr<Operand> zero = getZeroOperand(asmLongWord);
    const BinaryInst::CondCode cc = condCode(irBinary.operation, Ir::isSigned(irBinary.lhs->type));

    emitCmp(rhs, lhs, lhs->type);
    emitMove(zero, dst, dst->type);
    emitSetCC(cc, dst);
}

void GenerateAsmTree::genBinaryCondDouble(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(*irBinary.lhs);
    const std::shared_ptr<Operand> rhs = genOperand(*irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(*irBinary.dst);
    const std::shared_ptr<Operand> zero = getZeroOperand(asmLongWord);
    const BinaryInst::CondCode cc = condCode(irBinary.operation, false);
    const Identifier nanLabel(makeTemporaryPseudoName());
    const Identifier endLabel(makeTemporaryPseudoName());

    emitCmp(rhs, lhs, lhs->type);
    emitMove(zero, dst, dst->type);
    emitJmpCC(Inst::CondCode::PF, nanLabel);
    emitSetCC(cc, dst);
    emitJmp(endLabel);
    emitLabel(nanLabel);
    if (cc == Inst::CondCode::NE) {
        const auto one = std::make_shared<ImmOperand>(1, asmLongWord);
        emitMove(one, dst, dst->type);
    }
    emitLabel(endLabel);
}

void GenerateAsmTree::genBinaryDivide(const Ir::BinaryInst& irBinary)
{
    if (irBinary.type == Ir::doubleType) {
        genBinaryDivideDouble(irBinary);
        return;
    }
    if (isSigned(irBinary.type)) {
        genBinaryDivideSigned(irBinary);
        return;
    }
    genUnsignedBinaryDivide(irBinary);
}

void GenerateAsmTree::genBinaryDivideDouble(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(*irBinary.lhs);
    const std::shared_ptr<Operand> dst = genOperand(*irBinary.dst);
    const std::shared_ptr<Operand> rhs = genOperand(*irBinary.rhs);

    emitMove(lhs, dst, asmDouble);
    emitBinary(rhs, dst, BinaryInst::Operator::DivDouble, asmDouble);
}

void GenerateAsmTree::genBinaryDivideSigned(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> src1 = genOperand(*irBinary.lhs);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(irBinary.type));
    const std::shared_ptr<Operand> src2 = genOperand(*irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(*irBinary.dst);

    emitMove(src1, regAX, src1->type);
    emitCdq(src1->type);
    emitIdiv(src2, src1->type);
    emitMove(regAX, dst, src1->type);
}

void GenerateAsmTree::genUnsignedBinaryDivide(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> src1 = genOperand(*irBinary.lhs);
    const auto zero = getZeroOperand(src1->type);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(irBinary.type));
    const auto regDX = std::make_shared<RegisterOperand>(
        RegType::DX, getAsmType(irBinary.type));
    const std::shared_ptr<Operand> src2 = genOperand(*irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(*irBinary.dst);

    emitMove(src1, regAX, src1->type);
    emitMove(zero, regDX, src1->type);
    emitDiv(src2, src1->type);
    emitMove(regAX, dst, src1->type);
}

void GenerateAsmTree::genBinaryRemainder(const Ir::BinaryInst& irBinary)
{
    if (irBinary.type == Ir::i32Type|| irBinary.type == Ir::i64Type) {
        genSignedBinaryRemainder(irBinary);
        return;
    }
    genUnsignedBinaryRemainder(irBinary);
}

void GenerateAsmTree::genSignedBinaryRemainder(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> src1 = genOperand(*irBinary.lhs);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(irBinary.type));
    const std::shared_ptr<Operand> src2 = genOperand(*irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(*irBinary.dst);
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(irBinary.type));

    emitMove(src1, regAX, src1->type);
    emitCdq(src1->type);
    emitIdiv(src2, src1->type);
    emitMove(regDX, dst, src1->type);
}

void GenerateAsmTree::genUnsignedBinaryRemainder(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(*irBinary.lhs);
    const auto zero = getZeroOperand(lhs->type);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(irBinary.type));
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(irBinary.type));
    const std::shared_ptr<Operand> rhs = genOperand(*irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(*irBinary.dst);

    emitMove(lhs, regAX, lhs->type);
    emitMove(zero, regDX, lhs->type);
    emitDiv(rhs, lhs->type);
    emitMove(regDX, dst, lhs->type);
}

void GenerateAsmTree::genBinaryBasic(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(*irBinary.lhs);
    const std::shared_ptr<Operand> dst = genOperand(*irBinary.dst);
    const BinaryInst::Operator oper = binaryOperator(irBinary.operation);
    const std::shared_ptr<Operand> rhs = genOperand(*irBinary.rhs);

    emitMove(lhs, dst, lhs->type);
    emitBinary(rhs, dst, oper, lhs->type);
}

void GenerateAsmTree::genBinaryShift(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(*irBinary.lhs);
    const std::shared_ptr<Operand> dst = genOperand(*irBinary.dst);
    const bool isSigned = irBinary.type == Ir::i32Type || irBinary.type == Ir::i64Type;
    const BinaryInst::Operator oper = getShiftOperator(irBinary.operation, isSigned);
    const std::shared_ptr<Operand> rhs = genOperand(*irBinary.rhs);

    emitMove(lhs, dst, lhs->type);
    emitBinary(rhs, dst, oper, lhs->type);
}

void GenerateAsmTree::genAddPtr(const Ir::AddPtrInst& addPtrInst)
{
    if (addPtrInst.index->kind == Ir::Value::Kind::Constant)
        genAddPtrConstIndex(addPtrInst);
    else if (addPtrInst.scale == 1 || addPtrInst.scale == 4 ||
             addPtrInst.scale == 2 || addPtrInst.scale == 8) {
        genAddPtrVariableIndex1_2_4_8(addPtrInst);
    }
    else {
        genAddPtrVariableIndexAndOtherScale(addPtrInst);
    }
}

void GenerateAsmTree::genAddPtrConstIndex(const Ir::AddPtrInst& addPtrInst)
{
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(addPtrInst.type));
    const auto ptr = genOperand(*addPtrInst.ptr);
    const auto constValue = dynCast<Ir::ValueConst>(addPtrInst.index.get());
    const i64 index = std::get<i64>(constValue->value) * addPtrInst.scale;
    const auto memoryOp = std::make_shared<MemoryOperand>(
        RegType::AX, index, getAsmType(addPtrInst.ptr->type));
    const std::shared_ptr<Operand> dst = genOperand(*addPtrInst.dst);

    emitMove(ptr, regAX, getAsmType(addPtrInst.ptr->type));
    emitLea(memoryOp, dst, getAsmType(addPtrInst.ptr->type));
}

void GenerateAsmTree::genAddPtrVariableIndex1_2_4_8(const Ir::AddPtrInst& addPtrInst)
{
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(addPtrInst.type));
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(addPtrInst.type));
    const auto ptr = genOperand(*addPtrInst.ptr);
    const std::shared_ptr<Operand> index = genOperand(*addPtrInst.index);
    const AsmType type = getAsmType(addPtrInst.ptr->type);
    const auto indexed = std::make_shared<IndexedOperand>(RegType::AX, RegType::DX, addPtrInst.scale, type);
    const std::shared_ptr<Operand> dst = genOperand(*addPtrInst.dst);

    emitMove(ptr, regAX, asmQuadWord);
    emitMove(index, regDX, asmQuadWord);
    emitLea(indexed, dst, type);
}

void GenerateAsmTree::genAddPtrVariableIndexAndOtherScale(const Ir::AddPtrInst& addPtrInst)
{
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(addPtrInst.type));
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(addPtrInst.type));
    const auto ptr = genOperand(*addPtrInst.ptr);
    const std::shared_ptr<Operand> index = genOperand(*addPtrInst.index);
    const auto immScale = std::make_shared<ImmOperand>(addPtrInst.scale, asmQuadWord);
    const AsmType type = getAsmType(addPtrInst.ptr->type);
    constexpr i64 byteSize = 1;
    const auto indexed = std::make_shared<IndexedOperand>(RegType::AX, RegType::DX, byteSize, type);
    const std::shared_ptr<Operand> dst = genOperand(*addPtrInst.dst);

    emitMove(ptr, regAX, asmQuadWord);
    emitMove(index, regDX, asmQuadWord);
    emitBinary(immScale, regDX, BinaryInst::Operator::Mul, asmQuadWord);
    emitLea(indexed, dst, type);
}

void GenerateAsmTree::genCopyToOffSet(const Ir::CopyToOffsetInst& copyToOffset)
{
    const auto src = genOperand(*copyToOffset.src);
    const AsmType srcType = src->type;
    bool referingToLocal;
    if (copyToOffset.src->kind == Ir::Value::Kind::Variable) {
        const auto copyToOffSetSrcVar = dynCast<Ir::ValueVar>(copyToOffset.src.get());
        referingToLocal = copyToOffSetSrcVar->referingTo == ReferingTo::Local;
    }
    if (copyToOffset.src->kind == Ir::Value::Kind::Constant)
        referingToLocal = true;
    if (copyToOffset.referingTo != ReferingTo::Local)
        referingToLocal = false;
    const auto pseudoMem = std::make_shared<PseudoMemOperand>(
            Identifier(copyToOffset.iden.value),
            copyToOffset.offset,
            copyToOffset.size,
            copyToOffset.alignment,
            referingToLocal,
            srcType);

    pseudoMem->referingTo = copyToOffset.referingTo;
    emitMove(src, pseudoMem, srcType);
}

void GenerateAsmTree::genCopyFromOffset(const Ir::CopyFromOffsetInst& copyFromOffset)
{
    if (copyFromOffset.type.kind != Ir::IrType::Kind::ByteArray) {
        const auto dst = genOperand(*copyFromOffset.dst);
        const auto src = std::make_shared<PseudoMemOperand>(
                Identifier(copyFromOffset.src.value),
                copyFromOffset.offset,
                dst->type.size,
                1,
                copyFromOffset.referingTo == ReferingTo::Local,
                dst->type);
        src->referingTo = copyFromOffset.referingTo;
        emitMove(src, dst, dst->type);
        return;
    }
}

void GenerateAsmTree::genAllocate(const Ir::AllocateInst& allocate)
{
    emitPushPseudo(allocate.size, getAsmType(allocate.type), allocate.iden.value);
}

std::shared_ptr<Operand> GenerateAsmTree::getReturnRegister(const Ir::ReturnInst& returnInst)
{
    if (getAsmType(returnInst.type) == asmDouble)
        return std::make_shared<RegisterOperand>(RegType::XMM0, getAsmType(returnInst.type));
    return std::make_shared<RegisterOperand>(RegType::AX, getAsmType(returnInst.type));
}

void GenerateAsmTree::genReturn(const Ir::ReturnInst& returnInst)
{
    if (!returnInst.returnValue) {
        emitReturn();
        return;
    }
    const std::shared_ptr<Operand> val = genOperand(*returnInst.returnValue);
    const std::shared_ptr<Operand> regReturn = getReturnRegister(returnInst);

    emitMove(val, regReturn, getAsmType(returnInst.type));
    emitReturn();
}

void GenerateAsmTree::deAllocateStack(const Ir::FunCallInst& funcCall, const i64 stackPadding)
{
    const i64 bytesToRemove = 8l * (funcCall.args.size() - 6l) + stackPadding;
    if (0 < bytesToRemove) {
        const auto bytesToRemoveOperand = std::make_shared<ImmOperand>(bytesToRemove, asmLongWord);
        const auto sp = std::make_shared<RegisterOperand>(RegType::SP, asmQuadWord);
        emitBinary(bytesToRemoveOperand, sp, BinaryInst::Operator::Add, asmQuadWord);
    }
}

void GenerateAsmTree::genFunCall(const Ir::FunCallInst& funcCall)
{
    const i64 stackPadding = getStackPadding(funcCall.args.size());
    if (0 < stackPadding) {
        const auto left = std::make_shared<ImmOperand>(8, asmLongWord);
        const auto right = std::make_shared<RegisterOperand>(RegType::SP, asmQuadWord);
        emitBinary(left, right, BinaryInst::Operator::Sub, asmQuadWord);
    }
    genFunCallPushArgs(funcCall);
    emitCall(Identifier(funcCall.funName.value));
    deAllocateStack(funcCall, stackPadding);
    if (!funcCall.destination)
        return;
    const std::shared_ptr<Operand> dst = genOperand(*funcCall.destination);
    std::shared_ptr<Operand> src;
    if (getAsmType(funcCall.type) != asmDouble)
        src = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(funcCall.type));
    else
        src = std::make_shared<RegisterOperand>(RegType::XMM0, getAsmType(funcCall.type));
    emitMove(src, dst, getAsmType(funcCall.type));
}

std::vector<bool> GenerateAsmTree::genFuncCallPushArgsRegs(const Ir::FunCallInst& funcCall)
{
    i32 regIntIndex = 0;
    i32 regDoubleIndex = 0;
    std::vector pushedIntoRegs(funcCall.args.size(), false);
    for (size_t i = 0; i < funcCall.args.size(); ++i) {
        std::shared_ptr<Operand> src = genOperand(*funcCall.args[i]);
        const AsmType type = getAsmType(funcCall.args[i]->type);
        std::shared_ptr<RegisterOperand> reg;
        if (type != asmDouble && regIntIndex < intRegs.size())
            reg = std::make_shared<RegisterOperand>(intRegs[regIntIndex++], type);
        else if (type == asmDouble && regDoubleIndex < doubleRegs.size())
            reg = std::make_shared<RegisterOperand>(doubleRegs[regDoubleIndex++], type);
        else
            continue;
        emitMove(src, reg, type);
        pushedIntoRegs[i] = true;
    }
    return pushedIntoRegs;
}

void GenerateAsmTree::genFunCallPushArgs(const Ir::FunCallInst& funcCall)
{
    std::vector<bool> pushedIntoRegs = genFuncCallPushArgsRegs(funcCall);
    for (i64 i = funcCall.args.size() - 1; 0 <= i; --i) {
        if (pushedIntoRegs[i])
            continue;
        std::shared_ptr<Operand> src = genOperand(*funcCall.args[i]);
        if (src->kind == Operand::Kind::Imm ||
            src->kind == Operand::Kind::Register ||
            src->type.size == 8) {
            emitPush(src);
        }
        else {
            const AsmType type = getAsmType(funcCall.args[i]->type);
            emitMove(src, std::make_shared<RegisterOperand>(RegType::AX, type), type);
            emitPush(std::make_shared<RegisterOperand>(RegType::AX, asmQuadWord));
        }
    }
}

i64 getStackPadding(const size_t numArgs)
{
    if (numArgs <= 6)
        return 0;
    i32 stackPadding = 0;
    if (numArgs % 2 == 1)
        stackPadding += 8;
    return stackPadding;
}

std::shared_ptr<Operand> GenerateAsmTree::genOperand(const Ir::Value& value)
{
    switch (value.kind) {
        case Ir::Value::Kind::Constant:
            return getOperandFromConstant(value);
        case Ir::Value::Kind::Variable: {
            const auto valueVar = dynCast<const Ir::ValueVar>(&value);
            const bool isConst = valueVar->type == Ir::doubleType;
            return std::make_shared<PseudoOperand>(
                Identifier(valueVar->value.value),
                valueVar->referingTo,
                getAsmType(valueVar->type),
                isConst
            );
        }
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

std::shared_ptr<Operand> GenerateAsmTree::getOperandFromConstant(const Ir::Value& value)
{
    const auto valueConst = dynCast<const Ir::ValueConst>(&value);
    if (valueConst->type == Ir::doubleType)
        return genDoubleLocalConst(std::get<double>(valueConst->value), 8);
    std::shared_ptr<ImmOperand> imm = getImmOperandFromValue(*valueConst);
    if (INT_MAX < imm->value) {
        Identifier pseudoName(makeTemporaryPseudoName());
        const auto reg10 = std::make_shared<RegisterOperand>(RegType::R10, asmQuadWord);
        const auto pseudo = std::make_shared<PseudoOperand>(
            pseudoName, ReferingTo::Local, asmQuadWord, false);

        emitMove(imm, reg10, asmQuadWord);
        emitMove(reg10, pseudo, asmQuadWord);
        return pseudo;
    }
    return imm;
}

std::shared_ptr<Operand> GenerateAsmTree::genDoubleLocalConst(double value, i32 alignment)
{
    const auto it = m_constantDoubles.find(value);
    if (it != m_constantDoubles.end())
        return std::make_shared<DataOperand>(
            asmDouble, 0, Identifier(it->second), true, true);
    Identifier constLabel(makeTemporaryPseudoName());
    m_toplevel.emplace_back(std::make_unique<ConstVariable>(
        Identifier(constLabel), alignment, value, true));
    m_constantDoubles.emplace_hint(it, value, constLabel.value);
    return std::make_shared<DataOperand>(asmDouble, 0, constLabel, true, true);
}

std::shared_ptr<Operand> GenerateAsmTree::getZeroOperand(const AsmType type)
{
    using AsmKind = AsmType::Kind;

    switch (type.kind) {
        case AsmKind::Byte:       return std::make_shared<ImmOperand>(0, asmByte);
        case AsmKind::LongWord:   return std::make_shared<ImmOperand>(0, asmLongWord);
        case AsmKind::QuadWord:   return std::make_shared<ImmOperand>(0, asmQuadWord);
        case AsmKind::Double:     return genDoubleLocalConst(0.0, 8);
        default:
            std::abort();
    }
}

std::shared_ptr<ImmOperand> GenerateAsmTree::getImmOperandFromValue(const Ir::ValueConst& valueConst)
{
    using IrKind = Ir::IrType::Kind;

    switch (valueConst.type.kind) {
        case IrKind::Char: {
            const u64 value = std::get<char>(valueConst.value) & 0xFF;
            return std::make_shared<ImmOperand>(value, asmByte);
        }
        case IrKind::I8: {
            const u64 value = std::get<i8>(valueConst.value) & 0xFF;
            return std::make_shared<ImmOperand>(value, asmByte);
        }
        case IrKind::U8:
            return std::make_shared<ImmOperand>(std::get<u8>(valueConst.value), asmByte);
        case IrKind::I32: {
            const u64 value = std::get<i32>(valueConst.value) & 0xFFFFFFFF;
            return std::make_shared<ImmOperand>(value, asmLongWord);
        }
        case IrKind::U32:
            return std::make_shared<ImmOperand>(std::get<u32>(valueConst.value), asmLongWord);
        case IrKind::U64:
            return std::make_shared<ImmOperand>(std::get<u64>(valueConst.value), asmQuadWord);
        case IrKind::I64:
            return std::make_shared<ImmOperand>(
                std::bit_cast<u64>(std::get<i64>(valueConst.value)), asmQuadWord);
        default:
            std::abort();
    }
}

void GenerateAsmTree::zeroOutReg(const std::shared_ptr<RegisterOperand>& reg)
{
    emitBinary(reg, reg, BinaryInst::Operator::BitwiseXor, reg->type);
}

std::string makeTemporaryPseudoName()
{
    static i32 i = 0;
    return std::to_string(i++) + "..";
}

}// namespace CodeGen