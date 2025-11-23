#include "GenerateAsmTree.hpp"
#include "AsmAST.hpp"
#include "DynCast.hpp"
#include "FixUpInstructions.hpp"
#include "PseudoRegisterReplacer.hpp"
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
        std::shared_ptr<Operand> dst = genOperand(arg);
        emplaceMove(src, dst, type);
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
        std::shared_ptr<Operand> dst = genOperand(arg);
        emplaceMove(stack, dst, getAsmType(function.argTypes[i]));
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
                    getSingleInitValue(constValue->type.kind, constValue)));
                break;
            }
            case Ir::Initializer::Kind::Zero: {
                const auto zero = dynCast<Ir::ZeroInitializer>(init.get());
                initializers.emplace_back(std::make_unique<ZeroInitializer>(zero->size));
                break;
            }
        }
    }
    return std::make_unique<ArrayVariable>(
        Identifier(staticArray.name), 16, std::move(initializers),
        staticArray.global, getAsmType(staticArray.type));
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
    emplaceJmp(iden);
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
    const std::shared_ptr<Operand> condition = genOperand(jumpIfZero.condition);
    const Identifier target(jumpIfZero.target.value);
    const Identifier endLabel(makeTemporaryPseudoName());

    zeroOutReg(xmm0);

    emplaceCmp(condition, xmm0, asmDouble);
    emplaceJmpCC(Inst::CondCode::PF, endLabel);
    emplaceJmpCC(Inst::CondCode::E, target);
    emplaceLabel(endLabel);
}

void GenerateAsmTree::genJumpIfZeroInteger(const Ir::JumpIfZeroInst& jumpIfZero)
{
    const std::shared_ptr<Operand> condition = genOperand(jumpIfZero.condition);
    const std::shared_ptr<Operand> zero = getZeroOperand(condition->type);
    const Identifier target(jumpIfZero.target.value);

    emplaceCmp(zero, condition, condition->type);
    emplaceJmpCC(BinaryInst::CondCode::E, target);
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
    const std::shared_ptr<Operand> condition = genOperand(jumpIfNotZero.condition);
    const Identifier target(jumpIfNotZero.target.value);

    zeroOutReg(xmm0);

    emplaceCmp(condition, xmm0, asmDouble);
    emplaceJmpCC(Inst::CondCode::PF, target);
    emplaceJmpCC(Inst::CondCode::NE, target);
}

void GenerateAsmTree::genJumpIfNotZeroInteger(const Ir::JumpIfNotZeroInst& jumpIfNotZero)
{
    const std::shared_ptr<Operand> condition = genOperand(jumpIfNotZero.condition);
    const std::shared_ptr<Operand> zero = getZeroOperand(condition->type);
    const Identifier target(jumpIfNotZero.target.value);

    emplaceCmp(zero, condition, condition->type);
    emplaceJmpCC(Inst::CondCode::NE, target);
}

void GenerateAsmTree::genCopy(const Ir::CopyInst& copy)
{
    const std::shared_ptr<Operand> src = genOperand(copy.src);
    const std::shared_ptr<Operand> dst = genOperand(copy.dst);
    emplaceMove(src, dst, src->type);
}

void GenerateAsmTree::genGetAddress(const Ir::GetAddressInst& getAddress)
{
    const std::shared_ptr<Operand> src = genOperand(getAddress.src);
    const std::shared_ptr<Operand> dst = genOperand(getAddress.dst);
    emplaceLea(src, dst, asmQuadWord);
}

void GenerateAsmTree::genLoad(const Ir::LoadInst& load)
{
    const std::shared_ptr<Operand> ptr = genOperand(load.ptr);
    const std::shared_ptr<Operand> dst = genOperand(load.dst);
    const auto rax = std::make_shared<RegisterOperand>(RegType::DX, asmQuadWord);
    const auto memory = std::make_shared<MemoryOperand>(RegType::DX, 0, asmQuadWord);

    emplaceMove(ptr, rax, asmQuadWord);
    emplaceMove(memory, dst, dst->type);
}

void GenerateAsmTree::genStore(const Ir::StoreInst& store)
{
    const std::shared_ptr<Operand> src = genOperand(store.src);
    const std::shared_ptr<Operand> ptr = genOperand(store.ptr);
    const auto rax = std::make_shared<RegisterOperand>(RegType::DX, asmQuadWord);
    const auto memory = std::make_shared<MemoryOperand>(RegType::DX, 0, asmQuadWord);

    emplaceMove(ptr, rax, asmQuadWord);
    emplaceMove(src, memory, src->type);
}

void GenerateAsmTree::genLabel(const Ir::LabelInst& irLabel)
{
    const Identifier label(irLabel.target.value);
    emplaceLabel(label);
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
    const UnaryInst::Operator oper = unaryOperator(irUnary.operation);
    const std::shared_ptr<Operand> src = genOperand(irUnary.src);
    const std::shared_ptr<Operand> dst = genOperand(irUnary.dst);

    emplaceMove(src, dst, src->type);
    emplaceUnary(dst, oper, src->type);
}

void GenerateAsmTree::genNegateDouble(const Ir::UnaryInst& irUnary)
{
    using Operator = BinaryInst::Operator;
    const std::shared_ptr<Operand> src = genOperand(irUnary.src);
    const std::shared_ptr<Operand> rhs = genOperand(irUnary.dst);
    const std::shared_ptr<Operand> lhs = genDoubleLocalConst(-0.0, 16);

    emplaceMove(src, rhs, src->type);
    emplaceBinary(lhs, rhs, Operator::BitwiseXor, asmDouble);
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
    const std::shared_ptr<Operand> src = genOperand(irUnary.src);
    const std::shared_ptr<Operand> dst = genOperand(irUnary.dst);
    const auto zero = getZeroOperand(dst->type);
    const auto one = std::make_shared<ImmOperand>(1, asmLongWord);
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, asmDouble);
    const Identifier nanLabel(makeTemporaryPseudoName() + "nanUnaryNot");
    const Identifier endLabel(makeTemporaryPseudoName());

    zeroOutReg(xmm0);

    emplaceCmp(src, xmm0, asmDouble);
    emplaceJmpCC(BinaryInst::CondCode::PF, nanLabel);
    emplaceMove(zero, dst, dst->type);
    emplaceSetCC(BinaryInst::CondCode::E, dst);
    emplaceJmp(endLabel);
    emplaceLabel(nanLabel);
    emplaceMove(zero, dst, dst->type);
    emplaceLabel(endLabel);
}

void GenerateAsmTree::genUnaryNotInteger(const Ir::UnaryInst& irUnary)
{
    const std::shared_ptr<Operand> src = genOperand(irUnary.src);
    const std::shared_ptr<Operand> zero = getZeroOperand(src->type);
    const std::shared_ptr<Operand> dst = genOperand(irUnary.dst);

    emplaceCmp(zero, src, src->type);
    emplaceMove(zero, dst, dst->type);
    emplaceSetCC(BinaryInst::CondCode::E, dst);
}

void GenerateAsmTree::genZeroExtend(const Ir::ZeroExtendInst& zeroExtend)
{
    const std::shared_ptr<Operand> src = genOperand(zeroExtend.src);
    const std::shared_ptr<Operand> dst = genOperand(zeroExtend.dst);
    emplaceMoveZeroExtend(src, dst, src->type, dst->type);
}

void GenerateAsmTree::genDoubleToInt(const Ir::DoubleToIntInst& doubleToInt)
{
    const std::shared_ptr<Operand> src = genOperand(doubleToInt.src);
    const std::shared_ptr<Operand> dst = genOperand(doubleToInt.dst);

    if (dst->type.size < 4) {
        const auto raxLongWord = std::make_shared<RegisterOperand>(RegType::AX, asmLongWord);
        emplaceCvttsd2si(src, raxLongWord, asmLongWord);
        const auto raxByte = std::make_shared<RegisterOperand>(RegType::AX, asmByte);
        emplaceMove(raxByte, dst, dst->type);
    }
    else
        emplaceCvttsd2si(src, dst, dst->type);
}

void GenerateAsmTree::genDoubleToUInt(const Ir::DoubleToUIntInst& doubleToUInt)
{
    if (doubleToUInt.type == Ir::i8Type)
        genDoubleToUIntByte(doubleToUInt);
    if (doubleToUInt.type == Ir::u32Type)
        genDoubleToUIntLong(doubleToUInt);
    if (doubleToUInt.type == Ir::u64Type)
        genDoubleToUIntQuad(doubleToUInt);
}

void GenerateAsmTree::genDoubleToUIntByte(const Ir::DoubleToUIntInst& doubleToUInt)
{
    const std::shared_ptr<Operand> src = genOperand(doubleToUInt.src);
    const std::shared_ptr<Operand> dst = genOperand(doubleToUInt.dst);
    const auto rax = std::make_shared<RegisterOperand>(RegType::AX, asmLongWord);
    const auto eax = std::make_shared<RegisterOperand>(RegType::AX, asmByte);

    emplaceCvttsd2si(src, rax, asmLongWord);
    emplaceMove(eax, dst, dst->type);
}

void GenerateAsmTree::genDoubleToUIntLong(const Ir::DoubleToUIntInst& doubleToUInt)
{
    const std::shared_ptr<Operand> src = genOperand(doubleToUInt.src);
    const std::shared_ptr<Operand> dst = genOperand(doubleToUInt.dst);
    const auto rax = std::make_shared<RegisterOperand>(RegType::AX, asmQuadWord);
    const auto eax = std::make_shared<RegisterOperand>(RegType::AX, asmLongWord);

    emplaceCvttsd2si(src, rax, asmQuadWord);
    emplaceMove(eax, dst, dst->type);
}

void GenerateAsmTree::genDoubleToUIntQuad(const Ir::DoubleToUIntInst& doubleToUInt)
{
    constexpr double upperBoundConst = 9223372036854775808.0;
    const std::shared_ptr<Operand> upperBound = genDoubleLocalConst(upperBoundConst, 8);
    const std::shared_ptr<Operand> src = genOperand(doubleToUInt.src);
    const std::shared_ptr<Operand> dst = genOperand(doubleToUInt.dst);
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, asmDouble);
    const auto xmm1 = std::make_shared<RegisterOperand>(RegType::XMM1, asmDouble);
    const Identifier labelOne(makeTemporaryPseudoName());
    const Identifier labelTwo(makeTemporaryPseudoName());

    emplaceCmp(upperBound, src, asmDouble);
    emplaceJmpCC(Inst::CondCode::AE, labelOne);
    emplaceCvttsd2si(src, dst, asmQuadWord);
    emplaceJmp(labelTwo);
    emplaceLabel(labelOne);
    emplaceMove(src, xmm1, asmDouble);
    emplaceBinary(upperBound, xmm1, BinaryInst::Operator::Sub, asmDouble);
    emplaceCvttsd2si(xmm1, dst, asmQuadWord);
    emplaceMove(upperBound, xmm0, asmQuadWord);
    emplaceLabel(labelTwo);
}

void GenerateAsmTree::genIntToDouble(const Ir::IntToDoubleInst& intToDouble)
{
    const std::shared_ptr<Operand> src = genOperand(intToDouble.src);
    const std::shared_ptr<Operand> dst = genOperand(intToDouble.dst);

    if (src->type.size < 4) {
        const auto rax = std::make_shared<RegisterOperand>(RegType::AX, asmLongWord);
        emplaceMoveSX(src, rax, asmByte, asmLongWord);
        emplaceCvtsi2sd(rax, dst, asmLongWord);
    }
    else
        emplaceCvtsi2sd(src, dst, src->type);
}

void GenerateAsmTree::genUIntToDouble(const Ir::UIntToDoubleInst& uintToDouble)
{
    if (uintToDouble.src->type == Ir::u8Type)
        genUIntToDoubleByte(uintToDouble);
    if (uintToDouble.src->type == Ir::u32Type)
        genUIntToDoubleLong(uintToDouble);
    if (uintToDouble.src->type == Ir::u64Type)
        genUIntToDoubleQuad(uintToDouble);
}

void GenerateAsmTree::genUIntToDoubleByte(const Ir::UIntToDoubleInst& uintToDouble)
{
    const std::shared_ptr<Operand> src = genOperand(uintToDouble.src);
    const std::shared_ptr<Operand> rax = std::make_unique<RegisterOperand>(RegType::AX, asmLongWord);
    const std::shared_ptr<Operand> dst = genOperand(uintToDouble.dst);

    emplaceMoveZeroExtend(src, rax, asmByte, asmLongWord);
    emplaceCvtsi2sd(rax, dst, asmLongWord);
}

void GenerateAsmTree::genUIntToDoubleLong(const Ir::UIntToDoubleInst& uintToDouble)
{
    const std::shared_ptr<Operand> src = genOperand(uintToDouble.src);
    const std::shared_ptr<Operand> rax = std::make_unique<RegisterOperand>(RegType::AX, asmQuadWord);
    const std::shared_ptr<Operand> dst = genOperand(uintToDouble.dst);

    emplaceMoveZeroExtend(src, rax, asmLongWord, asmQuadWord);
    emplaceCvtsi2sd(rax, dst, asmQuadWord);
}

void GenerateAsmTree::genUIntToDoubleQuad(const Ir::UIntToDoubleInst& uintToDouble)
{
    using UnaryOper = UnaryInst::Operator;
    using BinaryOper = BinaryInst::Operator;

    const std::shared_ptr<Operand> zero = getZeroOperand(asmQuadWord);
    const std::shared_ptr<Operand> src = genOperand(uintToDouble.src);
    const Identifier labelOutOfRange(makeTemporaryPseudoName());
    const std::shared_ptr<Operand> dst = genOperand(uintToDouble.dst);
    const Identifier labelEnd(makeTemporaryPseudoName());
    const auto rax = std::make_shared<RegisterOperand>(RegType::AX, asmQuadWord);
    const auto rdx = std::make_shared<RegisterOperand>(RegType::DX, asmQuadWord);
    const auto one = std::make_shared<ImmOperand>(1l, asmQuadWord);

    emplaceCmp(zero, src, asmQuadWord);
    emplaceJmpCC(Inst::CondCode::L, labelOutOfRange);
    emplaceCvtsi2sd(src, dst, asmQuadWord);
    emplaceJmp(labelEnd);
    emplaceLabel(labelOutOfRange);
    emplaceMove(src, rdx, asmQuadWord);
    emplaceMove(rdx, rax, asmQuadWord);
    emplaceUnary(rdx, UnaryOper::Shr, asmQuadWord);
    emplaceBinary(one, rax, BinaryOper::BitwiseAnd, asmQuadWord);
    emplaceBinary(rax, rdx, BinaryOper::BitwiseOr, asmQuadWord);
    emplaceCvtsi2sd(rdx, dst, asmQuadWord);
    emplaceBinary(dst, dst, BinaryOper::Add, asmDouble);
    emplaceLabel(labelEnd);
}

void GenerateAsmTree::genSignExtend(const Ir::SignExtendInst& signExtend)
{
    const std::shared_ptr<Operand> src = genOperand(signExtend.src);
    const std::shared_ptr<Operand> dst = genOperand(signExtend.dst);
    emplaceMoveSX(src, dst, src->type, dst->type);
}

void GenerateAsmTree::genTruncate(const Ir::TruncateInst& truncate)
{
    const std::shared_ptr<Operand> src = genOperand(truncate.src);
    const std::shared_ptr<Operand> dst = genOperand(truncate.dst);
    emplaceMove(src, dst, dst->type);
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
            assert("Unsupported binary operation");
            std::unreachable();
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
    const std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    const std::shared_ptr<Operand> rhs = genOperand(irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const std::shared_ptr<Operand> zero = getZeroOperand(asmLongWord);
    const BinaryInst::CondCode cc = condCode(irBinary.operation, Ir::isSigned(irBinary.lhs->type));

    emplaceCmp(rhs, lhs, lhs->type);
    emplaceMove(zero, dst, dst->type);
    emplaceSetCC(cc, dst);
}

void GenerateAsmTree::genBinaryCondDouble(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    const std::shared_ptr<Operand> rhs = genOperand(irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const std::shared_ptr<Operand> zero = getZeroOperand(asmLongWord);
    const BinaryInst::CondCode cc = condCode(irBinary.operation, false);
    const Identifier nanLabel(makeTemporaryPseudoName());
    const Identifier endLabel(makeTemporaryPseudoName());

    emplaceCmp(rhs, lhs, lhs->type);
    emplaceMove(zero, dst, dst->type);
    emplaceJmpCC(Inst::CondCode::PF, nanLabel);
    emplaceSetCC(cc, dst);
    emplaceJmp(endLabel);
    emplaceLabel(nanLabel);
    if (cc == Inst::CondCode::NE) {
        const auto one = std::make_shared<ImmOperand>(1, asmLongWord);
        emplaceMove(one, dst, dst->type);
    }
    emplaceLabel(endLabel);
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
    const std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const std::shared_ptr<Operand> rhs = genOperand(irBinary.rhs);

    emplaceMove(lhs, dst, asmDouble);
    emplaceBinary(rhs, dst, BinaryInst::Operator::DivDouble, asmDouble);
}

void GenerateAsmTree::genBinaryDivideSigned(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(irBinary.type));
    const std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);

    emplaceMove(src1, regAX, src1->type);
    emplaceCdq(src1->type);
    emplaceIdiv(src2, src1->type);
    emplaceMove(regAX, dst, src1->type);
}

void GenerateAsmTree::genUnsignedBinaryDivide(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    const auto zero = getZeroOperand(src1->type);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(irBinary.type));
    const auto regDX = std::make_shared<RegisterOperand>(
        RegType::DX, getAsmType(irBinary.type));
    const std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);

    emplaceMove(src1, regAX, src1->type);
    emplaceMove(zero, regDX, src1->type);
    emplaceDiv(src2, src1->type);
    emplaceMove(regAX, dst, src1->type);
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
    const std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(irBinary.type));
    const std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(irBinary.type));

    emplaceMove(src1, regAX, src1->type);
    emplaceCdq(src1->type);
    emplaceIdiv(src2, src1->type);
    emplaceMove(regDX, dst, src1->type);
}

void GenerateAsmTree::genUnsignedBinaryRemainder(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    const auto zero = getZeroOperand(lhs->type);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(irBinary.type));
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(irBinary.type));
    const std::shared_ptr<Operand> rhs = genOperand(irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);

    emplaceMove(lhs, regAX, lhs->type);
    emplaceMove(zero, regDX, lhs->type);
    emplaceDiv(rhs, lhs->type);
    emplaceMove(regDX, dst, lhs->type);
}

void GenerateAsmTree::genBinaryBasic(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const BinaryInst::Operator oper = binaryOperator(irBinary.operation);
    const std::shared_ptr<Operand> rhs = genOperand(irBinary.rhs);

    emplaceMove(lhs, dst, lhs->type);
    emplaceBinary(rhs, dst, oper, lhs->type);
}

void GenerateAsmTree::genBinaryShift(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const bool isSigned = irBinary.type == Ir::i32Type || irBinary.type == Ir::i64Type;
    const BinaryInst::Operator oper = getShiftOperator(irBinary.operation, isSigned);
    const std::shared_ptr<Operand> rhs = genOperand(irBinary.rhs);

    emplaceMove(lhs, dst, lhs->type);
    emplaceBinary(rhs, dst, oper, lhs->type);
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
    const auto ptr = genOperand(addPtrInst.ptr);
    const auto constValue = dynCast<Ir::ValueConst>(addPtrInst.index.get());
    const i64 index = std::get<i64>(constValue->value) * addPtrInst.scale;
    const auto memoryOp = std::make_shared<MemoryOperand>(
        RegType::AX, index, getAsmType(addPtrInst.ptr->type));
    const std::shared_ptr<Operand> dst = genOperand(addPtrInst.dst);

    emplaceMove(ptr, regAX, getAsmType(addPtrInst.ptr->type));
    emplaceLea(memoryOp, dst, getAsmType(addPtrInst.ptr->type));
}

void GenerateAsmTree::genAddPtrVariableIndex1_2_4_8(const Ir::AddPtrInst& addPtrInst)
{
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(addPtrInst.type));
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(addPtrInst.type));
    const auto ptr = genOperand(addPtrInst.ptr);
    const std::shared_ptr<Operand> index = genOperand(addPtrInst.index);
    const AsmType type = getAsmType(addPtrInst.ptr->type);
    const auto indexed = std::make_shared<IndexedOperand>(RegType::AX, RegType::DX, addPtrInst.scale, type);
    const std::shared_ptr<Operand> dst = genOperand(addPtrInst.dst);

    emplaceMove(ptr, regAX, asmQuadWord);
    emplaceMove(index, regDX, asmQuadWord);
    emplaceLea(indexed, dst, type);
}

void GenerateAsmTree::genAddPtrVariableIndexAndOtherScale(const Ir::AddPtrInst& addPtrInst)
{
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(addPtrInst.type));
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(addPtrInst.type));
    const auto ptr = genOperand(addPtrInst.ptr);
    const std::shared_ptr<Operand> index = genOperand(addPtrInst.index);
    const auto immScale = std::make_shared<ImmOperand>(addPtrInst.scale, asmQuadWord);
    const AsmType type = getAsmType(addPtrInst.ptr->type);
    constexpr i64 byteSize = 1;
    const auto indexed = std::make_shared<IndexedOperand>(RegType::AX, RegType::DX, byteSize, type);
    const std::shared_ptr<Operand> dst = genOperand(addPtrInst.dst);

    emplaceMove(ptr, regAX, asmQuadWord);
    emplaceMove(index, regDX, asmQuadWord);
    emplaceBinary(immScale, regDX, BinaryInst::Operator::Mul, asmQuadWord);
    emplaceLea(indexed, dst, type);
}

void GenerateAsmTree::genCopyToOffSet(const Ir::CopyToOffsetInst& copyToOffset)
{
    const auto src = genOperand(copyToOffset.src);
    const AsmType srcType = src->type;
    bool referingToLocal;
    if (copyToOffset.src->kind == Ir::Value::Kind::Variable) {
        const auto copyToOffSetSrcVar = dynCast<Ir::ValueVar>(copyToOffset.src.get());
        referingToLocal = copyToOffSetSrcVar->referingTo == ReferingTo::Local;
    }
    if (copyToOffset.src->kind == Ir::Value::Kind::Constant)
        referingToLocal = true;
    const auto pseudoMem = std::make_shared<PseudoMemOperand>(
            Identifier(copyToOffset.iden.value),
            copyToOffset.offset,
            copyToOffset.size,
            copyToOffset.alignment,
            referingToLocal,
            srcType);

    emplaceMove(src, pseudoMem, srcType);
}

void GenerateAsmTree::genAllocate(const Ir::AllocateInst& allocate)
{
    emplacePushPseudo(allocate.size, getAsmType(allocate.type), allocate.iden.value);
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
        emplaceReturn();
        return;
    }
    const std::shared_ptr<Operand> val = genOperand(returnInst.returnValue);
    const std::shared_ptr<Operand> regReturn = getReturnRegister(returnInst);

    emplaceMove(val, regReturn, getAsmType(returnInst.type));
    emplaceReturn();
}

void GenerateAsmTree::deAllocateStack(const Ir::FunCallInst& funcCall, const i64 stackPadding)
{
    const i64 bytesToRemove = 8l * (funcCall.args.size() - 6l) + stackPadding;
    if (0 < bytesToRemove) {
        const auto bytesToRemoveOperand = std::make_shared<ImmOperand>(bytesToRemove, asmLongWord);
        const auto sp = std::make_shared<RegisterOperand>(RegType::SP, asmQuadWord);
        emplaceBinary(bytesToRemoveOperand, sp, BinaryInst::Operator::Add, asmQuadWord);
    }
}

void GenerateAsmTree::genFunCall(const Ir::FunCallInst& funcCall)
{
    const i32 stackPadding = getStackPadding(funcCall.args.size());
    if (0 < stackPadding)
        emplaceBinary(
            std::make_shared<ImmOperand>(8, asmLongWord),
            std::make_shared<RegisterOperand>(RegType::SP, asmQuadWord),
            BinaryInst::Operator::Sub, asmQuadWord);
    genFunCallPushArgs(funcCall);
    emplaceCall(Identifier(funcCall.funName.value));
    deAllocateStack(funcCall, stackPadding);
    if (!funcCall.destination)
        return;
    const std::shared_ptr<Operand> dst = genOperand(funcCall.destination);
    std::shared_ptr<Operand> src;
    if (getAsmType(funcCall.type) != asmDouble)
        src = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(funcCall.type));
    else
        src = std::make_shared<RegisterOperand>(RegType::XMM0, getAsmType(funcCall.type));
    emplaceMove(src, dst, getAsmType(funcCall.type));
}

std::vector<bool> GenerateAsmTree::genFuncCallPushArgsRegs(const Ir::FunCallInst& funcCall)
{
    i32 regIntIndex = 0;
    i32 regDoubleIndex = 0;
    std::vector pushedIntoRegs(funcCall.args.size(), false);
    for (size_t i = 0; i < funcCall.args.size(); ++i) {
        std::shared_ptr<Operand> src = genOperand(funcCall.args[i]);
        const AsmType type = getAsmType(funcCall.args[i]->type);
        std::shared_ptr<RegisterOperand> reg;
        if (type != asmDouble && regIntIndex < intRegs.size())
            reg = std::make_shared<RegisterOperand>(intRegs[regIntIndex++], type);
        else if (type == asmDouble && regDoubleIndex < doubleRegs.size())
            reg = std::make_shared<RegisterOperand>(doubleRegs[regDoubleIndex++], type);
        else
            continue;
        emplaceMove(src, reg, type);
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
        std::shared_ptr<Operand> src = genOperand(funcCall.args[i]);
        if (src->kind == Operand::Kind::Imm ||
            src->kind == Operand::Kind::Register ||
            src->type.size == 8) {
            emplacePush(src);
        }
        else {
            const AsmType type = getAsmType(funcCall.args[i]->type);
            emplaceMove(src, std::make_shared<RegisterOperand>(RegType::AX, type), type);
            emplacePush(std::make_shared<RegisterOperand>(RegType::AX, asmQuadWord));
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

std::shared_ptr<Operand> GenerateAsmTree::genOperand(const std::shared_ptr<Ir::Value>& value)
{
    switch (value->kind) {
        case Ir::Value::Kind::Constant:
            return getOperandFromConstant(value);
        case Ir::Value::Kind::Variable: {
            const auto valueVar = dynCast<Ir::ValueVar>(value.get());
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

std::shared_ptr<Operand> GenerateAsmTree::getOperandFromConstant(const std::shared_ptr<Ir::Value>& value)
{
    const auto valueConst = dynCast<Ir::ValueConst>(value.get());
    if (valueConst->type == Ir::doubleType)
        return genDoubleLocalConst(std::get<double>(valueConst->value), 8);
    std::shared_ptr<ImmOperand> imm = getImmOperandFromValue(*valueConst);
    if (INT_MAX < imm->value) {
        Identifier pseudoName(makeTemporaryPseudoName());
        const auto reg10 = std::make_shared<RegisterOperand>(RegType::R10, asmQuadWord);
        const auto pseudo = std::make_shared<PseudoOperand>(
            pseudoName, ReferingTo::Local, asmQuadWord, false);

        emplaceMove(imm, reg10, asmQuadWord);
        emplaceMove(reg10, pseudo, asmQuadWord);
        return pseudo;
    }
    return imm;
}

std::shared_ptr<Operand> GenerateAsmTree::genDoubleLocalConst(double value, i32 alignment)
{
    const auto it = m_constantDoubles.find(value);
    if (it != m_constantDoubles.end())
        return std::make_shared<DataOperand>(Identifier(it->second), asmDouble, true);
    Identifier constLabel(makeTemporaryPseudoName());
    m_toplevel.emplace_back(std::make_unique<ConstVariable>(
        Identifier(constLabel), alignment, value, true));
    m_constantDoubles.emplace_hint(it, value, constLabel.value);
    return std::make_shared<DataOperand>(constLabel, asmDouble, true);
}

std::shared_ptr<Operand> GenerateAsmTree::getZeroOperand(const AsmType type)
{
    using AsmKind = AsmType::Kind;

    switch (type.kind) {
        case AsmKind::Byte:       return std::make_shared<ImmOperand>(0, asmByte);
        case AsmKind::LongWord:       return std::make_shared<ImmOperand>(0, asmLongWord);
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
    emplaceBinary(reg, reg, BinaryInst::Operator::BitwiseXor, reg->type);
}

std::string makeTemporaryPseudoName()
{
    static i32 i = 0;
    return std::to_string(i++) + "..";
}

}// namespace CodeGen