#include "GenerateAsmTree.hpp"
#include "AsmAST.hpp"
#include "DynCast.hpp"
#include "FixUpInstructions.hpp"
#include "PseudoRegisterReplacer.hpp"
#include "Types/TypeConversion.hpp"
#include "Operators.hpp"

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
    for (const auto& toplevelIr : program.topLevels)
        m_toplevel.emplace_back(std::move(genTopLevel(*toplevelIr)));
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
        const AsmType type = Operators::getAsmType(function.argTypes[i]);
        std::shared_ptr<RegisterOperand> src;
        if (type != AsmType::Double && regIntIndex < intRegs.size())
            src = std::make_shared<RegisterOperand>(intRegs[regIntIndex++], type);
        else if (type == AsmType::Double && regDoubleInex < doubleRegs.size())
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
            RegType::BP, stackAlignment * stackPtr++, Operators::getAsmType(function.argTypes[i]));
        auto arg = std::make_shared<Ir::ValueVar>(function.args[i], function.argTypes[i]);
        std::shared_ptr<Operand> dst = genOperand(arg);
        emplaceMove(stack, dst, Operators::getAsmType(function.argTypes[i]));
    }
}

u64 getSingleInitValue(const Type type, const Ir::ValueConst* const value)
{
    if (type == Type::I32)
        return std::get<i32>(value->value);
    if (type == Type::I64)
        return std::get<i64>(value->value);
    if (type == Type::U32)
        return std::get<u32>(value->value);
    if (type == Type::U64 || type == Type::Pointer)
        return std::get<u64>(value->value);
    if (type == Type::Double) {
        const double init = std::get<double>(value->value);
        return std::bit_cast<i64>(init);
    }
    std::abort();
}

std::unique_ptr<TopLevel> genStaticVariable(const Ir::StaticVariable& staticVariable)
{
    const Type type = staticVariable.type;
    const auto value = dynCast<const Ir::ValueConst>(staticVariable.value.get());
    auto result = std::make_unique<StaticVariable>(
        staticVariable.name, Operators::getAsmType(type), staticVariable.global);
    result->init = getSingleInitValue(type, value);
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
                    getSingleInitValue(constValue->type, constValue)));
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
        staticArray.global, Operators::getAsmType(staticArray.type));
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
    if (jumpIfZero.type != Type::Double) {
        genJumpIfZeroInteger(jumpIfZero);
        return;
    }
    genJumpIfZeroDouble(jumpIfZero);
}

void GenerateAsmTree::genJumpIfZeroDouble(const Ir::JumpIfZeroInst& jumpIfZero)
{
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, AsmType::Double);
    const std::shared_ptr<Operand> condition = genOperand(jumpIfZero.condition);
    const Identifier target(jumpIfZero.target.value);
    const Identifier endLabel(makeTemporaryPseudoName());

    zeroOutReg(xmm0);

    emplaceCmp(condition, xmm0, AsmType::Double);
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
    if (jumpIfNotZero.type != Type::Double) {
        genJumpIfNotZeroInteger(jumpIfNotZero);
        return;
    }
    genJumpIfNotZeroDouble(jumpIfNotZero);
}

void GenerateAsmTree::genJumpIfNotZeroDouble(const Ir::JumpIfNotZeroInst& jumpIfNotZero)
{
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, AsmType::Double);
    const std::shared_ptr<Operand> condition = genOperand(jumpIfNotZero.condition);
    const Identifier target(jumpIfNotZero.target.value);

    zeroOutReg(xmm0);

    emplaceCmp(condition, xmm0, AsmType::Double);
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
    emplaceLea(src, dst, AsmType::QuadWord);
}

void GenerateAsmTree::genLoad(const Ir::LoadInst& load)
{
    const std::shared_ptr<Operand> ptr = genOperand(load.ptr);
    const std::shared_ptr<Operand> dst = genOperand(load.dst);
    const auto rax = std::make_shared<RegisterOperand>(RegType::DX, AsmType::QuadWord);
    const auto memory = std::make_shared<MemoryOperand>(RegType::DX, 0, AsmType::QuadWord);

    emplaceMove(ptr, rax, AsmType::QuadWord);
    emplaceMove(memory, dst, dst->type);
}

void GenerateAsmTree::genStore(const Ir::StoreInst& store)
{
    const std::shared_ptr<Operand> src = genOperand(store.src);
    const std::shared_ptr<Operand> ptr = genOperand(store.ptr);
    const auto rax = std::make_shared<RegisterOperand>(RegType::DX, AsmType::QuadWord);
    const auto memory = std::make_shared<MemoryOperand>(RegType::DX, 0, AsmType::QuadWord);

    emplaceMove(ptr, rax, AsmType::QuadWord);
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
        irUnary.type == Type::Double) {
        genNegateDouble(irUnary);
        return;
    }
    genUnaryBasic(irUnary);
}

void GenerateAsmTree::genUnaryBasic(const Ir::UnaryInst& irUnary)
{
    const UnaryInst::Operator oper = Operators::unaryOperator(irUnary.operation);
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
    emplaceBinary(lhs, rhs, Operator::BitwiseXor, AsmType::Double);
}

void GenerateAsmTree::genUnaryNot(const Ir::UnaryInst& irUnary)
{
    if (irUnary.src->type != Type::Double)
        genUnaryNotInteger(irUnary);
    else
        genUnaryNotDouble(irUnary);
}

void GenerateAsmTree::genUnaryNotDouble(const Ir::UnaryInst& irUnary)
{
    const std::shared_ptr<Operand> src = genOperand(irUnary.src);
    const std::shared_ptr<Operand> dst = genOperand(irUnary.dst);
    const auto zero = getZeroOperand(dst->type);
    const auto one = std::make_shared<ImmOperand>(1, AsmType::LongWord);
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, AsmType::Double);
    const Identifier nanLabel(makeTemporaryPseudoName() + "nanUnaryNot");
    const Identifier endLabel(makeTemporaryPseudoName());

    zeroOutReg(xmm0);

    emplaceCmp(src, xmm0, AsmType::Double);
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
    emplaceMoveZeroExtend(src, dst, src->type);
}

void GenerateAsmTree::genDoubleToInt(const Ir::DoubleToIntInst& doubleToInt)
{
    const std::shared_ptr<Operand> src = genOperand(doubleToInt.src);
    const std::shared_ptr<Operand> dst = genOperand(doubleToInt.dst);
    emplaceCvttsd2si(src, dst, dst->type);
}

void GenerateAsmTree::genDoubleToUInt(const Ir::DoubleToUIntInst& doubleToUInt)
{
    if (doubleToUInt.type == Type::U32)
        genDoubleToUIntLong(doubleToUInt);
    if (doubleToUInt.type == Type::U64)
        genDoubleToUIntQuad(doubleToUInt);
}

void GenerateAsmTree::genDoubleToUIntLong(const Ir::DoubleToUIntInst& doubleToUInt)
{
    const std::shared_ptr<Operand> src = genOperand(doubleToUInt.src);
    const std::shared_ptr<Operand> dst = genOperand(doubleToUInt.dst);
    const auto rax = std::make_shared<RegisterOperand>(RegType::AX, AsmType::QuadWord);
    const auto eax = std::make_shared<RegisterOperand>(RegType::AX, AsmType::LongWord);

    emplaceCvttsd2si(src, rax, AsmType::QuadWord);
    emplaceMove(eax, dst, AsmType::LongWord);
}

void GenerateAsmTree::genDoubleToUIntQuad(const Ir::DoubleToUIntInst& doubleToUInt)
{
    constexpr double upperBoundConst = 9223372036854775808.0;
    const std::shared_ptr<Operand> upperBound = genDoubleLocalConst(upperBoundConst, 8);
    const std::shared_ptr<Operand> src = genOperand(doubleToUInt.src);
    const std::shared_ptr<Operand> dst = genOperand(doubleToUInt.dst);
    const auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, AsmType::Double);
    const auto xmm1 = std::make_shared<RegisterOperand>(RegType::XMM1, AsmType::Double);
    const Identifier labelOne(makeTemporaryPseudoName());
    const Identifier labelTwo(makeTemporaryPseudoName());

    emplaceCmp(upperBound, src, AsmType::Double);
    emplaceJmpCC(Inst::CondCode::AE, labelOne);
    emplaceCvttsd2si(src, dst, AsmType::QuadWord);
    emplaceJmp(labelTwo);
    emplaceLabel(labelOne);
    emplaceMove(src, xmm1, AsmType::Double);
    emplaceBinary(upperBound, xmm1, BinaryInst::Operator::Sub, AsmType::Double);
    emplaceCvttsd2si(xmm1, dst, AsmType::QuadWord);
    emplaceMove(upperBound, xmm0, AsmType::QuadWord);
    emplaceLabel(labelTwo);
}

void GenerateAsmTree::genIntToDouble(const Ir::IntToDoubleInst& intToDouble)
{
    const std::shared_ptr<Operand> src = genOperand(intToDouble.src);
    const std::shared_ptr<Operand> dst = genOperand(intToDouble.dst);

    emplaceCvtsi2sd(src, dst, src->type);
}

void GenerateAsmTree::genUIntToDouble(const Ir::UIntToDoubleInst& uintToDouble)
{
    if (uintToDouble.src->type == Type::U32)
        genUIntToDoubleLong(uintToDouble);
    if (uintToDouble.src->type == Type::U64)
        genUIntToDoubleQuad(uintToDouble);
}

void GenerateAsmTree::genUIntToDoubleLong(const Ir::UIntToDoubleInst& uintToDouble)
{
    const std::shared_ptr<Operand> src = genOperand(uintToDouble.src);
    const std::shared_ptr<Operand> rax = std::make_unique<RegisterOperand>(RegType::AX, AsmType::QuadWord);
    const std::shared_ptr<Operand> dst = genOperand(uintToDouble.dst);

    emplaceMoveZeroExtend(src, rax, Operators::getAsmType(uintToDouble.type));
    emplaceCvtsi2sd(rax, dst, AsmType::QuadWord);
}

void GenerateAsmTree::genUIntToDoubleQuad(const Ir::UIntToDoubleInst& uintToDouble)
{
    using UnaryOper = UnaryInst::Operator;
    using BinaryOper = BinaryInst::Operator;

    const std::shared_ptr<Operand> zero = getZeroOperand(AsmType::QuadWord);
    const std::shared_ptr<Operand> src = genOperand(uintToDouble.src);
    const Identifier labelOutOfRange(makeTemporaryPseudoName());
    const std::shared_ptr<Operand> dst = genOperand(uintToDouble.dst);
    const Identifier labelEnd(makeTemporaryPseudoName());
    const auto rax = std::make_shared<RegisterOperand>(RegType::AX, AsmType::QuadWord);
    const auto rdx = std::make_shared<RegisterOperand>(RegType::DX, AsmType::QuadWord);
    const auto one = std::make_shared<ImmOperand>(1l, AsmType::QuadWord);

    emplaceCmp(zero, src, AsmType::QuadWord);
    emplaceJmpCC(Inst::CondCode::L, labelOutOfRange);
    emplaceCvtsi2sd(src, dst, AsmType::QuadWord);
    emplaceJmp(labelEnd);
    emplaceLabel(labelOutOfRange);
    emplaceMove(src, rdx, AsmType::QuadWord);
    emplaceMove(rdx, rax, AsmType::QuadWord);
    emplaceUnary(rdx, UnaryOper::Shr, AsmType::QuadWord);
    emplaceBinary(one, rax, BinaryOper::BitwiseAnd, AsmType::QuadWord);
    emplaceBinary(rax, rdx, BinaryOper::BitwiseOr, AsmType::QuadWord);
    emplaceCvtsi2sd(rdx, dst, AsmType::QuadWord);
    emplaceBinary(dst, dst, BinaryOper::Add, AsmType::Double);
    emplaceLabel(labelEnd);
}

void GenerateAsmTree::genSignExtend(const Ir::SignExtendInst& signExtend)
{
    const std::shared_ptr<Operand> src1 = genOperand(signExtend.src);
    const std::shared_ptr<Operand> src2 = genOperand(signExtend.dst);
    emplaceMoveSX(src1, src2);
}

void GenerateAsmTree::genTruncate(const Ir::TruncateInst& truncate)
{
    const std::shared_ptr<Operand> src1 = genOperand(truncate.src);
    const std::shared_ptr<Operand> src2 = genOperand(truncate.dst);
    emplaceMove(src1, src2, AsmType::LongWord);
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
    if (irBinary.lhs->type == Type::Double) {
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
    const std::shared_ptr<Operand> zero = getZeroOperand(AsmType::LongWord);
    const bool isSigned = irBinary.lhs->type == Type::I32 || irBinary.lhs->type == Type::I64;
    const BinaryInst::CondCode cc = Operators::condCode(irBinary.operation, isSigned);

    emplaceCmp(rhs, lhs, lhs->type);
    emplaceMove(zero, dst, dst->type);
    emplaceSetCC(cc, dst);
}

void GenerateAsmTree::genBinaryCondDouble(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    const std::shared_ptr<Operand> rhs = genOperand(irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const std::shared_ptr<Operand> zero = getZeroOperand(AsmType::LongWord);
    const BinaryInst::CondCode cc = Operators::condCode(irBinary.operation, false);
    const Identifier nanLabel(makeTemporaryPseudoName());
    const Identifier endLabel(makeTemporaryPseudoName());

    emplaceCmp(rhs, lhs, lhs->type);
    emplaceMove(zero, dst, dst->type);
    emplaceJmpCC(Inst::CondCode::PF, nanLabel);
    emplaceSetCC(cc, dst);
    emplaceJmp(endLabel);
    emplaceLabel(nanLabel);
    if (cc == Inst::CondCode::NE) {
        const auto one = std::make_shared<ImmOperand>(1, AsmType::LongWord);
        emplaceMove(one, dst, dst->type);
    }
    emplaceLabel(endLabel);
}

void GenerateAsmTree::genBinaryDivide(const Ir::BinaryInst& irBinary)
{
    if (irBinary.type == Type::Double) {
        genBinaryDivideDouble(irBinary);
        return;
    }
    if (irBinary.type == Type::I64 || irBinary.type == Type::I32) {
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

    emplaceMove(lhs, dst, AsmType::Double);
    emplaceBinary(rhs, dst, BinaryInst::Operator::DivDouble, AsmType::Double);
}

void GenerateAsmTree::genBinaryDivideSigned(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, Operators::getAsmType(irBinary.type));
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
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, Operators::getAsmType(irBinary.type));
    const auto regDX = std::make_shared<RegisterOperand>(
        RegType::DX, Operators::getAsmType(irBinary.type)
        );
    const std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);

    emplaceMove(src1, regAX, src1->type);
    emplaceMove(zero, regDX, src1->type);
    emplaceDiv(src2, src1->type);
    emplaceMove(regAX, dst, src1->type);
}

void GenerateAsmTree::genBinaryRemainder(const Ir::BinaryInst& irBinary)
{
    if (irBinary.type == Type::I64 || irBinary.type == Type::I32) {
        genSignedBinaryRemainder(irBinary);
        return;
    }
    genUnsignedBinaryRemainder(irBinary);
}

void GenerateAsmTree::genSignedBinaryRemainder(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, Operators::getAsmType(irBinary.type));
    const std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, Operators::getAsmType(irBinary.type));

    emplaceMove(src1, regAX, src1->type);
    emplaceCdq(src1->type);
    emplaceIdiv(src2, src1->type);
    emplaceMove(regDX, dst, src1->type);
}

void GenerateAsmTree::genUnsignedBinaryRemainder(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    const auto zero = getZeroOperand(lhs->type);
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, Operators::getAsmType(irBinary.type));
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, Operators::getAsmType(irBinary.type));
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
    const BinaryInst::Operator oper = Operators::binaryOperator(irBinary.operation);
    const std::shared_ptr<Operand> rhs = genOperand(irBinary.rhs);

    emplaceMove(lhs, dst, lhs->type);
    emplaceBinary(rhs, dst, oper, lhs->type);
}

void GenerateAsmTree::genBinaryShift(const Ir::BinaryInst& irBinary)
{
    const std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    const std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const bool isSigned = irBinary.type == Type::I32 || irBinary.type == Type::I64;
    const BinaryInst::Operator oper = Operators::getShiftOperator(irBinary.operation, isSigned);
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
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, Operators::getAsmType(addPtrInst.type));
    const auto ptr = genOperand(addPtrInst.ptr);
    const auto constValue = dynCast<Ir::ValueConst>(addPtrInst.index.get());
    const i64 index = std::get<i64>(constValue->value) * addPtrInst.scale;
    const auto memoryOp = std::make_shared<MemoryOperand>(
        RegType::AX, index, Operators::getAsmType(addPtrInst.ptr->type));
    const std::shared_ptr<Operand> dst = genOperand(addPtrInst.dst);

    emplaceMove(ptr, regAX, Operators::getAsmType(addPtrInst.ptr->type));
    emplaceLea(memoryOp, dst, Operators::getAsmType(addPtrInst.ptr->type));
}

void GenerateAsmTree::genAddPtrVariableIndex1_2_4_8(const Ir::AddPtrInst& addPtrInst)
{
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, Operators::getAsmType(addPtrInst.type));
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, Operators::getAsmType(addPtrInst.type));
    const auto ptr = genOperand(addPtrInst.ptr);
    const std::shared_ptr<Operand> index = genOperand(addPtrInst.index);
    const AsmType type = Operators::getAsmType(addPtrInst.ptr->type);
    const auto indexed = std::make_shared<IndexedOperand>(RegType::AX, RegType::DX, addPtrInst.scale, type);
    const std::shared_ptr<Operand> dst = genOperand(addPtrInst.dst);

    emplaceMove(ptr, regAX, AsmType::QuadWord);
    emplaceMove(index, regDX, AsmType::QuadWord);
    emplaceLea(indexed, dst, type);
}

void GenerateAsmTree::genAddPtrVariableIndexAndOtherScale(const Ir::AddPtrInst& addPtrInst)
{
    const auto regAX = std::make_shared<RegisterOperand>(RegType::AX, Operators::getAsmType(addPtrInst.type));
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, Operators::getAsmType(addPtrInst.type));
    const auto ptr = genOperand(addPtrInst.ptr);
    const std::shared_ptr<Operand> index = genOperand(addPtrInst.index);
    const auto immScale = std::make_shared<ImmOperand>(addPtrInst.scale, AsmType::QuadWord);
    const AsmType type = Operators::getAsmType(addPtrInst.ptr->type);
    constexpr i64 byteSize = 1;
    const auto indexed = std::make_shared<IndexedOperand>(RegType::AX, RegType::DX, byteSize, type);
    const std::shared_ptr<Operand> dst = genOperand(addPtrInst.dst);

    emplaceMove(ptr, regAX, AsmType::QuadWord);
    emplaceMove(index, regDX, AsmType::QuadWord);
    emplaceBinary(immScale, regDX, BinaryInst::Operator::Mul, AsmType::QuadWord);
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
    emplacePushPseudo(allocate.size, Operators::getAsmType(allocate.type), allocate.iden.value);
}

void GenerateAsmTree::genReturn(const Ir::ReturnInst& returnInst)
{
    const std::shared_ptr<Operand> val = genOperand(returnInst.returnValue);
    std::shared_ptr<Operand> regReturn;
    if (Operators::getAsmType(returnInst.type) == AsmType::Double)
        regReturn = std::make_shared<RegisterOperand>(RegType::XMM0, Operators::getAsmType(returnInst.type));
    else
        regReturn = std::make_shared<RegisterOperand>(RegType::AX, Operators::getAsmType(returnInst.type));

    emplaceMove(val, regReturn, Operators::getAsmType(returnInst.type));
    emplaceReturn();
}

void GenerateAsmTree::genFunCall(const Ir::FunCallInst& funcCall)
{
    const i32 stackPadding = getStackPadding(funcCall.args.size());
    if (0 < stackPadding)
        emplaceBinary(
            std::make_shared<ImmOperand>(8, AsmType::LongWord),
            std::make_shared<RegisterOperand>(RegType::SP, AsmType::QuadWord),
            BinaryInst::Operator::Sub, AsmType::QuadWord);
    genFunCallPushArgs(funcCall);
    emplaceCall(Identifier(funcCall.funName.value));
    const i32 bytesToRemove = 8 * (funcCall.args.size() - 6) + stackPadding;
    if (0 < bytesToRemove) {
        const auto bytesToRemoveOperand = std::make_shared<ImmOperand>(bytesToRemove, AsmType::LongWord);
        const auto sp = std::make_shared<RegisterOperand>(RegType::SP, AsmType::QuadWord);
        emplaceBinary(bytesToRemoveOperand, sp, BinaryInst::Operator::Add, AsmType::QuadWord);
    }
    std::shared_ptr<Operand> dst = genOperand(funcCall.destination);
    std::shared_ptr<Operand> src;
    if (Operators::getAsmType(funcCall.type) != AsmType::Double)
        src = std::make_shared<RegisterOperand>(RegType::AX, Operators::getAsmType(funcCall.type));
    else
        src = std::make_shared<RegisterOperand>(RegType::XMM0, Operators::getAsmType(funcCall.type));
    emplaceMove(src, dst, Operators::getAsmType(funcCall.type));
}

std::vector<bool> GenerateAsmTree::genFuncCallPushArgsRegs(const Ir::FunCallInst& funcCall)
{
    i32 regIntIndex = 0;
    i32 regDoubleIndex = 0;
    std::vector pushedIntoRegs(funcCall.args.size(), false);
    for (size_t i = 0; i < funcCall.args.size(); ++i) {
        std::shared_ptr<Operand> src = genOperand(funcCall.args[i]);
        const AsmType type = Operators::getAsmType(funcCall.args[i]->type);
        std::shared_ptr<RegisterOperand> reg;
        if (type != AsmType::Double && regIntIndex < intRegs.size())
            reg = std::make_shared<RegisterOperand>(intRegs[regIntIndex++], type);
        else if (type == AsmType::Double && regDoubleIndex < doubleRegs.size())
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
            getSize(funcCall.args[i]->type) == 8 ) {
            emplacePush(src);
        }
        else {
            const AsmType type = Operators::getAsmType(funcCall.args[i]->type);
            emplaceMove(src, std::make_shared<RegisterOperand>(RegType::AX, type), type);
            emplacePush(std::make_shared<RegisterOperand>(RegType::AX, AsmType::QuadWord));
        }
    }
}

i32 getStackPadding(const size_t numArgs)
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
            const bool isConst = valueVar->type == Type::Double;
            return std::make_shared<PseudoOperand>(
                Identifier(valueVar->value.value),
                valueVar->referingTo,
                Operators::getAsmType(valueVar->type),
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
    if (valueConst->type == Type::Double)
        return genDoubleLocalConst(std::get<double>(valueConst->value), 8);
    std::shared_ptr<ImmOperand> imm = getImmOperandFromValue(*valueConst);
    if (INT_MAX < imm->value) {
        Identifier pseudoName(makeTemporaryPseudoName());
        const auto reg10 = std::make_shared<RegisterOperand>(RegType::R10, AsmType::QuadWord);
        const auto pseudo = std::make_shared<PseudoOperand>(
            pseudoName, ReferingTo::Local, AsmType::QuadWord, false);

        emplaceMove(imm, reg10, AsmType::QuadWord);
        emplaceMove(reg10, pseudo, AsmType::QuadWord);
        return pseudo;
    }
    return imm;
}

std::shared_ptr<Operand> GenerateAsmTree::genDoubleLocalConst(double value, i32 alignment)
{
    const auto it = m_constantDoubles.find(value);
    if (it != m_constantDoubles.end())
        return std::make_shared<DataOperand>(Identifier(it->second), AsmType::Double, true);
    Identifier constLabel(makeTemporaryPseudoName());
    m_toplevel.emplace_back(std::make_unique<ConstVariable>(
        Identifier(constLabel), alignment, value, true));
    m_constantDoubles.emplace_hint(it, value, constLabel.value);
    return std::make_shared<DataOperand>(constLabel, AsmType::Double, true);
}

std::shared_ptr<Operand> GenerateAsmTree::getZeroOperand(const AsmType type)
{
    if (type == AsmType::LongWord)
        return std::make_shared<ImmOperand>(0, AsmType::LongWord);
    if (type == AsmType::QuadWord)
        return std::make_shared<ImmOperand>(0l, AsmType::QuadWord);
    if (type == AsmType::Double)
        return genDoubleLocalConst(0.0, 8);
    std::abort();
}

std::shared_ptr<ImmOperand> GenerateAsmTree::getImmOperandFromValue(const Ir::ValueConst& valueConst)
{
    if (valueConst.type == Type::U32)
        return std::make_shared<ImmOperand>(std::get<u32>(valueConst.value), AsmType::LongWord);
    if (valueConst.type == Type::U64)
        return std::make_shared<ImmOperand>(std::get<u64>(valueConst.value), AsmType::QuadWord);
    if (valueConst.type == Type::I32)
        return std::make_shared<ImmOperand>(
            std::bit_cast<u32>(std::get<i32>(valueConst.value)), AsmType::LongWord);
    if (valueConst.type == Type::I64)
        return std::make_shared<ImmOperand>(
            std::bit_cast<u64>(std::get<i64>(valueConst.value)), AsmType::QuadWord);
    std::abort();
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