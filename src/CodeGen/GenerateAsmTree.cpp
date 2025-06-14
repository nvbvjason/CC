#include "GenerateAsmTree.hpp"
#include "PseudoRegisterReplacer.hpp"
#include "AsmAST.hpp"
#include "FixUpInstructions.hpp"

#include <cassert>
#include <random>

namespace {
using RegType = CodeGen::RegisterOperand::Kind;
const std::vector intRegs = {RegType::DI, RegType::SI, RegType::DX,
                             RegType::CX, RegType::R8, RegType::R9};
const std::vector doubleRegs = {RegType::XMM0, RegType::XMM1, RegType::XMM2, RegType::XMM4,
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
    switch (topLevel.type) {
        case Type::Function: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto function = static_cast<const Ir::Function*>(&topLevel);
            return genFunction(*function);
        }
        case Type::StaticVariable: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto staticVariable = static_cast<const Ir::StaticVariable*>(&topLevel);
            return genStaticVariable(*staticVariable);
        }
        assert("generateTopLevel idk type");
    }
    std::unreachable();
}

std::unique_ptr<TopLevel> GenerateAsmTree::genFunction(const Ir::Function& function)
{
    auto functionCodeGen = std::make_unique<Function>(function.name, function.isGlobal);
    insts.clear();
    i32 regIndex = 0;
    for (; regIndex < function.args.size() && regIndex < intRegs.size(); ++regIndex) {
        auto src = std::make_shared<RegisterOperand>(intRegs[regIndex], getAsmType(function.argTypes[regIndex]));
        auto arg = std::make_shared<Ir::ValueVar>(function.args[regIndex], function.argTypes[regIndex]);
        std::shared_ptr<Operand> dst = genOperand(arg);
        insts.emplace_back(std::make_unique<MoveInst>(
            src, dst, getAsmType(function.argTypes[regIndex])));
    }
    i32 stackPtr = 2;
    for (; regIndex < function.args.size(); ++regIndex, ++stackPtr) {
        auto stack = std::make_shared<StackOperand>(8 * stackPtr, getAsmType(function.argTypes[regIndex]));
        auto arg = std::make_shared<Ir::ValueVar>(function.args[regIndex], function.argTypes[regIndex]);
        std::shared_ptr<Operand> dst = genOperand(arg);
        insts.emplace_back(std::make_unique<MoveInst>(
            stack, dst, getAsmType(function.argTypes[regIndex])));
    }
    for (const std::unique_ptr<Ir::Instruction>& inst : function.insts)
        genInst(inst);
    functionCodeGen->instructions = std::move(insts);
    return functionCodeGen;
}

std::unique_ptr<TopLevel> genStaticVariable(const Ir::StaticVariable& staticVariable)
{
    const Type type = staticVariable.type;
    const auto value = static_cast<const Ir::ValueConst*>(staticVariable.value.get());
    auto result = std::make_unique<StaticVariable>(
        staticVariable.name, getAsmType(staticVariable.type), staticVariable.global);
    if (type == Type::I32)
        result->init = std::get<i32>(value->value);
    if (type == Type::I64)
        result->init = std::get<i64>(value->value);
    if (type == Type::U32)
        result->init = std::get<u32>(value->value);
    if (type == Type::U64)
        result->init = std::get<u64>(value->value);
    return result;
}

void GenerateAsmTree::genInst(const std::unique_ptr<Ir::Instruction>& inst)
{
    using Kind = Ir::Instruction::Kind;
    switch (inst->kind) {
        case Kind::Return: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irReturn = static_cast<Ir::ReturnInst*>(inst.get());
            genReturn(*irReturn);
            break;
        }
        case Kind::SignExtend: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto signExtend = static_cast<Ir::SignExtendInst*>(inst.get());
            genSignExtend(*signExtend);
            break;
        }
        case Kind::Truncate: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto truncate = static_cast<Ir::TruncateInst*>(inst.get());
            genTruncate(*truncate);
            break;
        }
        case Kind::ZeroExtend: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto zeroExtend = static_cast<Ir::ZeroExtendInst*>(inst.get());
            genZeroExtend(*zeroExtend);
            break;
        }
        case Kind::DoubleToInt: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto doubleToInt = static_cast<Ir::DoubleToIntInst*>(inst.get());
            genDoubleToInt(*doubleToInt);
            break;
        }
        case Kind::DoubleToUInt: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto doubleToUInt = static_cast<Ir::DoubleToUIntInst*>(inst.get());
            genDoubleToUInt(*doubleToUInt);
            break;
        }
        case Kind::IntToDouble: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto intToDouble = static_cast<Ir::IntToDoubleInst*>(inst.get());
            genIntToDouble(*intToDouble);
            break;
        }
        case Kind::UIntToDouble: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto uIntToDouble = static_cast<Ir::UIntToDoubleInst*>(inst.get());
            genUIntToDouble(*uIntToDouble);
            break;
        }
        case Kind::Unary: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irUnary = static_cast<Ir::UnaryInst*>(inst.get());
            genUnary(*irUnary);
            break;
        }
        case Kind::Binary: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irBinary = static_cast<Ir::BinaryInst*>(inst.get());
            genBinary(*irBinary);
            break;
        }
        case Kind::Copy: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irCopy = static_cast<Ir::CopyInst*>(inst.get());
            genCopy(*irCopy);
            break;
        }
        case Kind::Jump: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irJump = static_cast<Ir::JumpInst*>(inst.get());
            genJump(*irJump);
            break;
        }
        case Kind::JumpIfZero: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irJumpIfZero = static_cast<Ir::JumpIfZeroInst*>(inst.get());
            genJumpIfZero(*irJumpIfZero);
            break;
        }
        case Kind::JumpIfNotZero: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irJumpIfNotZero = static_cast<Ir::JumpIfNotZeroInst*>(inst.get());
            genJumpIfNotZero(*irJumpIfNotZero);
            break;
        }
        case Kind::Label: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irLabel = static_cast<Ir::LabelInst*>(inst.get());
            genLabel(*irLabel);
            break;
        }
        case Kind::FunCall: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irFunCall = static_cast<Ir::FunCallInst*>(inst.get());
            genFunCall(*irFunCall);
            break;
        }
        default:
            throw std::runtime_error("Unsupported instruction type");
    }
}

void GenerateAsmTree::genJump(const Ir::JumpInst& irJump)
{
    Identifier iden(irJump.target.value);
    insts.emplace_back(std::make_unique<JmpInst>(iden));
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
    auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, AsmType::Double);
    insts.emplace_back(std::make_unique<BinaryInst>(
        xmm0, xmm0, BinaryInst::Operator::BitwiseXor, AsmType::Double));
    std::shared_ptr<Operand> condition = genOperand(jumpIfZero.condition);
    insts.emplace_back(std::make_unique<CmpInst>(condition, xmm0, AsmType::Double));
    Identifier target(jumpIfZero.target.value);
    insts.emplace_back(std::make_unique<JmpCCInst>(Inst::CondCode::E, target));
}

void GenerateAsmTree::genJumpIfZeroInteger(const Ir::JumpIfZeroInst& jumpIfZero)
{
    std::shared_ptr<Operand> condition = genOperand(jumpIfZero.condition);
    std::shared_ptr<ImmOperand> zero = getZeroImmOfType(condition->type);
    insts.emplace_back(std::make_unique<CmpInst>(
        zero, condition, condition->type));

    Identifier target(jumpIfZero.target.value);
    insts.emplace_back(std::make_unique<JmpCCInst>(BinaryInst::CondCode::E, target));
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
    auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, AsmType::Double);
    insts.emplace_back(std::make_unique<BinaryInst>(
        xmm0, xmm0, BinaryInst::Operator::BitwiseXor, AsmType::Double));
    std::shared_ptr<Operand> condition = genOperand(jumpIfNotZero.condition);
    insts.emplace_back(std::make_unique<CmpInst>(condition, xmm0, AsmType::Double));
    Identifier target(jumpIfNotZero.target.value);
    insts.emplace_back(std::make_unique<JmpCCInst>(Inst::CondCode::E, target));
}

void GenerateAsmTree::genJumpIfNotZeroInteger(const Ir::JumpIfNotZeroInst& jumpIfNotZero)
{
    std::shared_ptr<Operand> condition = genOperand(jumpIfNotZero.condition);
    std::shared_ptr<ImmOperand> zero = getZeroImmOfType(condition->type);
    insts.emplace_back(std::make_unique<CmpInst>(zero, condition, condition->type));
    Identifier target(jumpIfNotZero.target.value);
    insts.emplace_back(std::make_unique<JmpCCInst>(Inst::CondCode::NE, target));
}

void GenerateAsmTree::genCopy(const Ir::CopyInst& type)
{
    std::shared_ptr<Operand> src = genOperand(type.src);
    std::shared_ptr<Operand> dst = genOperand(type.dst);
    insts.emplace_back(std::make_unique<MoveInst>(src, dst, src->type));
}

void GenerateAsmTree::genLabel(const Ir::LabelInst& irLabel)
{
    Identifier label(irLabel.target.value);
    insts.emplace_back(std::make_unique<LabelInst>(label));
}

void GenerateAsmTree::genUnary(const Ir::UnaryInst& irUnary)
{
    if (irUnary.operation == Ir::UnaryInst::Operation::Not) {
        genUnaryNot(irUnary);
        return;
    }
    UnaryInst::Operator oper = unaryOperator(irUnary.operation);
    std::shared_ptr<Operand> src = genOperand(irUnary.source);
    std::shared_ptr<Operand> dst = genOperand(irUnary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(src, dst, src->type));
    insts.emplace_back(std::make_unique<UnaryInst>(dst, oper, src->type));
}

void GenerateAsmTree::genUnaryNot(const Ir::UnaryInst& irUnary)
{
    std::shared_ptr<Operand> src = genOperand(irUnary.source);
    std::shared_ptr<ImmOperand> immOperand = getZeroImmOfType(src->type);
    insts.emplace_back(std::make_unique<CmpInst>(immOperand, src, src->type));

    std::shared_ptr<Operand> dst = genOperand(irUnary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(immOperand, dst, dst->type));

    insts.emplace_back(std::make_unique<SetCCInst>(BinaryInst::CondCode::E, dst));
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

void GenerateAsmTree::genZeroExtend(const Ir::ZeroExtendInst& zeroExtend)
{
    std::shared_ptr<Operand> src = genOperand(zeroExtend.src);
    std::shared_ptr<Operand> dst = genOperand(zeroExtend.dst);
    insts.emplace_back(std::make_unique<MoveZeroExtendInst>(src, dst, src->type));
}

void GenerateAsmTree::genDoubleToInt(const Ir::DoubleToIntInst& doubleToInt)
{
    std::shared_ptr<Operand> src = genOperand(doubleToInt.src);
    std::shared_ptr<Operand> dst = genOperand(doubleToInt.dst);
    insts.emplace_back(std::make_unique<Cvttsd2siInst>(src, dst, dst->type));
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
    std::shared_ptr<Operand> src = genOperand(doubleToUInt.src);
    std::shared_ptr<Operand> dst = genOperand(doubleToUInt.dst);
    insts.emplace_back(std::make_unique<MoveZeroExtendInst>(src, dst, getAsmType(doubleToUInt.type)));
    insts.emplace_back(std::make_unique<Cvtsi2sdInst>(src, dst, getAsmType(doubleToUInt.type)));
}

void GenerateAsmTree::genDoubleToUIntQuad(const Ir::DoubleToUIntInst& doubleToUInt)
{
    constexpr double upperBoundConst = 9223372036854775808.0;
    Identifier constLabel(makeTemporaryPseudoName());
    auto upperBound = std::make_shared<DataOperand>(constLabel, AsmType::Double);
    auto src = genOperand(doubleToUInt.src);
    auto dst = genOperand(doubleToUInt.dst);
    auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, AsmType::Double);
    auto ax = std::make_shared<RegisterOperand>(RegType::AX, AsmType::QuadWord);
    Identifier labelOne(makeTemporaryPseudoName());
    Identifier labelTwo(makeTemporaryPseudoName());
    m_programCodegen.topLevels.emplace_back(std::make_unique<ConstVariable>(constLabel, 8, upperBoundConst));
    insts.emplace_back(std::make_unique<CmpInst>(upperBound, src, AsmType::Double));
    insts.emplace_back(std::make_unique<JmpCCInst>(Inst::CondCode::AE, labelOne));
    insts.emplace_back(std::make_unique<Cvttsd2siInst>(src, dst, AsmType::QuadWord));
    insts.emplace_back(std::make_unique<JmpInst>(labelTwo));
    insts.emplace_back(std::make_unique<LabelInst>(labelOne));
    insts.emplace_back(std::make_unique<MoveInst>(src, ax, AsmType::Double));
    insts.emplace_back(std::make_unique<BinaryInst>(
        upperBound, ax, BinaryInst::Operator::Sub, AsmType::Double));
    insts.emplace_back(std::make_unique<Cvtsi2sdInst>(ax, dst, AsmType::QuadWord));
    insts.emplace_back(std::make_unique<MoveInst>(upperBound, xmm0, AsmType::QuadWord));
    insts.emplace_back(std::make_unique<LabelInst>(labelTwo));
}

void GenerateAsmTree::genIntToDouble(const Ir::IntToDoubleInst& intToDouble)
{
    std::shared_ptr<Operand> src = genOperand(intToDouble.src);
    std::shared_ptr<Operand> dst = genOperand(intToDouble.dst);
    insts.emplace_back(std::make_unique<Cvttsd2siInst>(src, dst, src->type));
}

void GenerateAsmTree::genUIntToDouble(const Ir::UIntToDoubleInst& uintToDouble)
{
    if (uintToDouble.type == Type::U32)
        genUIntToDoubleLong(uintToDouble);
    if (uintToDouble.type == Type::U64)
        genUIntToDoubleQuad(uintToDouble);
}

void GenerateAsmTree::genUIntToDoubleLong(const Ir::UIntToDoubleInst& uintToDouble)
{
    std::shared_ptr<Operand> src = genOperand(uintToDouble.src);
    std::shared_ptr<Operand> ax = std::make_unique<RegisterOperand>(RegType::AX, AsmType::QuadWord);
    insts.emplace_back(std::make_unique<MoveZeroExtendInst>(src, ax, getAsmType(uintToDouble.type)));
    std::shared_ptr<Operand> dst = genOperand(uintToDouble.dst);
    insts.emplace_back(std::make_unique<Cvttsd2siInst>(ax, dst, AsmType::QuadWord));
}

void GenerateAsmTree::genUIntToDoubleQuad(const Ir::UIntToDoubleInst& uintToDouble)
{
    std::shared_ptr<ImmOperand> zero = getZeroImmOfType(AsmType::QuadWord);
    std::shared_ptr<Operand> src = genOperand(uintToDouble.src);
    Identifier labelOneIden(makeTemporaryPseudoName());
    insts.emplace_back(std::make_unique<JmpCCInst>(Inst::CondCode::L, labelOneIden));
    std::shared_ptr<Operand> dst = genOperand(uintToDouble.dst);
    insts.emplace_back(std::make_unique<Cvtsi2sdInst>(src, dst, AsmType::QuadWord));
    Identifier labelTwoIden(makeTemporaryPseudoName());
    insts.emplace_back(std::make_unique<JmpInst>(labelTwoIden));
    insts.emplace_back(std::make_unique<LabelInst>(labelOneIden));
    auto xmm0 = std::make_shared<RegisterOperand>(RegType::XMM0, AsmType::Double);
    auto xmm1 = std::make_shared<RegisterOperand>(RegType::XMM1, AsmType::Double);
    insts.emplace_back(std::make_unique<MoveInst>(src, xmm0, AsmType::QuadWord));
    insts.emplace_back(std::make_unique<MoveInst>(xmm0, xmm1, AsmType::QuadWord));
    insts.emplace_back(std::make_unique<UnaryInst>(
        xmm0, UnaryInst::Operator::Shr, AsmType::QuadWord));
    auto one = std::make_shared<ImmOperand>(1l);
    insts.emplace_back(std::make_unique<BinaryInst>(
        one, xmm0, BinaryInst::Operator::AndBitwise, AsmType::QuadWord));
    insts.emplace_back(std::make_unique<BinaryInst>(
        xmm0, xmm1, BinaryInst::Operator::OrBitwise, AsmType::QuadWord));
    insts.emplace_back(std::make_unique<Cvtsi2sdInst>(xmm1, dst, AsmType::QuadWord));
    insts.emplace_back(std::make_unique<MoveInst>(dst, dst, AsmType::Double));
    insts.emplace_back(std::make_unique<JmpInst>(labelTwoIden));
}

void GenerateAsmTree::genSignExtend(const Ir::SignExtendInst& signExtend)
{
    std::shared_ptr<Operand> src1 = genOperand(signExtend.src);
    std::shared_ptr<Operand> src2 = genOperand(signExtend.dst);
    insts.emplace_back(std::make_unique<MoveSXInst>(src1, src2));
}

void GenerateAsmTree::genTruncate(const Ir::TruncateInst& truncate)
{
    std::shared_ptr<Operand> src1 = genOperand(truncate.src);
    std::shared_ptr<Operand> src2 = genOperand(truncate.dst);
    insts.emplace_back(std::make_unique<MoveInst>(src1, src2, AsmType::LongWord));
}

void GenerateAsmTree::genBinaryCond(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    insts.emplace_back(std::make_unique<CmpInst>(src2, src1, src1->type));

    std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    std::shared_ptr<Operand> imm = getZeroImmOfType(dst->type);
    insts.emplace_back(std::make_unique<MoveInst>(imm, dst, dst->type));
    const bool isSigned = irBinary.type == Type::I32 || irBinary.type == Type::I64;
    BinaryInst::CondCode cc = condCode(irBinary.operation, isSigned);
    insts.emplace_back(std::make_unique<SetCCInst>(cc, dst));
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
    std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    insts.emplace_back(std::make_unique<MoveInst>(lhs, dst, AsmType::Double));
    std::shared_ptr<Operand> rhs = genOperand(irBinary.lhs);
    insts.emplace_back(std::make_unique<BinaryInst>(
        lhs, rhs, BinaryInst::Operator::DivDouble, AsmType::Double));
}

void GenerateAsmTree::genBinaryDivideSigned(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegType::AX, getAsmType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(src1, regAX, src1->type));

    insts.emplace_back(std::make_unique<CdqInst>(src1->type));

    std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    insts.emplace_back(std::make_unique<IdivInst>(src2, src1->type));

    std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    insts.emplace_back(std::make_unique<MoveInst>(regAX, dst, src1->type));
}

void GenerateAsmTree::genUnsignedBinaryDivide(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegType::AX, getAsmType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(src1, regAX, src1->type));

    const auto zero = getZeroImmOfType(src1->type);
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(zero, regDX, src1->type));

    std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    insts.emplace_back(std::make_unique<DivInst>(src2, src1->type));

    std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    insts.emplace_back(std::make_unique<MoveInst>(regAX, dst, src1->type));
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
    std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegType::AX, getAsmType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(src1, regAX, src1->type));

    insts.emplace_back(std::make_unique<CdqInst>(src1->type));

    std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    insts.emplace_back(std::make_unique<IdivInst>(src2, src1->type));

    std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    const auto regDX = std::make_shared<RegisterOperand>(RegType::DX, getAsmType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(regDX, dst, src1->type));
}

void GenerateAsmTree::genUnsignedBinaryRemainder(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegType::AX, getAsmType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(src1, regAX, src1->type));

    const auto zero = getZeroImmOfType(src1->type);
    const auto regDX = std::make_shared<RegisterOperand>(
        RegType::DX, getAsmType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(zero, regDX, src1->type));

    std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    insts.emplace_back(std::make_unique<DivInst>(src2, src1->type));

    std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    insts.emplace_back(std::make_unique<MoveInst>(regDX, dst, src1->type));
}

void GenerateAsmTree::genBinaryBasic(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = genOperand(irBinary.lhs);
    std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    insts.emplace_back(std::make_unique<MoveInst>(src1, dst, src1->type));

    BinaryInst::Operator oper = binaryOperator(irBinary.operation);
    std::shared_ptr<Operand> src2 = genOperand(irBinary.rhs);
    insts.emplace_back(std::make_unique<BinaryInst>(src2, dst, oper, src1->type));
}

void GenerateAsmTree::genBinaryShift(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> lhs = genOperand(irBinary.lhs);
    std::shared_ptr<Operand> dst = genOperand(irBinary.dst);
    insts.emplace_back(std::make_unique<MoveInst>(lhs, dst, lhs->type));

    const bool isSigned = irBinary.type == Type::I32 || irBinary.type == Type::I64;
    BinaryInst::Operator oper = getShiftOperator(irBinary.operation, isSigned);
    std::shared_ptr<Operand> rhs = genOperand(irBinary.rhs);
    insts.emplace_back(std::make_unique<BinaryInst>(rhs, dst, oper, lhs->type));
}

void GenerateAsmTree::genReturn(const Ir::ReturnInst& returnInst)
{
    std::shared_ptr<Operand> val = genOperand(returnInst.returnValue);
    std::shared_ptr<Operand> regReturn;
    if (val->type == AsmType::Double) {
        regReturn = std::make_shared<RegisterOperand>(RegType::XMM0, AsmType::Double);
    }
    else {
        regReturn = std::make_shared<RegisterOperand>(RegType::AX, getAsmType(returnInst.type));
    }
    insts.emplace_back(std::make_unique<MoveInst>(
        val, regReturn, getAsmType(returnInst.type)));
    insts.emplace_back(std::make_unique<ReturnInst>());
}

void GenerateAsmTree::genFunCall(const Ir::FunCallInst& funcCall)
{
    const i32 stackPadding = getStackPadding(funcCall.args.size());
    if (0 < stackPadding)
        insts.emplace_back(std::make_unique<BinaryInst>(
            std::make_shared<ImmOperand>(8),
            std::make_shared<RegisterOperand>(RegType::SP, AsmType::QuadWord),
            BinaryInst::Operator::Sub, AsmType::QuadWord));
    genFunCallPushArgs(funcCall);
    insts.emplace_back(std::make_unique<CallInst>(Identifier(funcCall.funName.value)));
    const i32 bytesToRemove = 8 * (funcCall.args.size() - 6) + stackPadding;
    if (0 < bytesToRemove)
        insts.emplace_back(std::make_unique<BinaryInst>(
        std::make_shared<ImmOperand>(bytesToRemove),
        std::make_shared<RegisterOperand>(RegType::SP, AsmType::QuadWord),
        BinaryInst::Operator::Add,
        AsmType::QuadWord));
    std::shared_ptr<Operand> destination = genOperand(funcCall.destination);
    insts.emplace_back(std::make_unique<MoveInst>(
        std::make_shared<RegisterOperand>(RegType::AX, getAsmType(funcCall.type)),
        destination,
        getAsmType(funcCall.type)));
}

void GenerateAsmTree::genFunCallPushArgs(const Ir::FunCallInst& funcCall)
{
    i32 regIndex = 0;
    for (; regIndex < funcCall.args.size() && regIndex < intRegs.size(); ++regIndex) {
        std::shared_ptr<Operand> src = genOperand(funcCall.args[regIndex]);
        const AsmType type = getAsmType(funcCall.args[regIndex]->type);
        insts.emplace_back(std::make_unique<MoveInst>(
            src,
            std::make_shared<RegisterOperand>(intRegs[regIndex], type),
            type));
    }
    for (i32 i = funcCall.args.size() - 1; regIndex <= i; --i) {
        std::shared_ptr<Operand> src = genOperand(funcCall.args[i]);
        if (src->kind == Operand::Kind::Imm ||
            src->kind == Operand::Kind::Register ||
            funcCall.args[i]->type == Type::I64) {
            insts.emplace_back(std::make_unique<PushInst>(src));
        }
        else {
            const AsmType type = getAsmType(funcCall.args[i]->type);
            insts.emplace_back(std::make_unique<MoveInst>(
                src, std::make_shared<RegisterOperand>(RegType::AX, type),
                type));
            insts.emplace_back(std::make_unique<PushInst>(
                std::make_shared<RegisterOperand>(RegType::AX, AsmType::QuadWord)));
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

UnaryInst::Operator unaryOperator(const Ir::UnaryInst::Operation type)
{
    using IrOper = Ir::UnaryInst::Operation;
    using AsmOper = UnaryInst::Operator;
    switch (type)
    {
        case IrOper::Complement:        return AsmOper::Not;
        case IrOper::Negate:            return AsmOper::Neg;
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

BinaryInst::Operator binaryOperator(const Ir::BinaryInst::Operation type)
{
    using IrOper = Ir::BinaryInst::Operation;
    using AsmOper = BinaryInst::Operator;
    switch (type) {
        case IrOper::Add:          return AsmOper::Add;
        case IrOper::Subtract:     return AsmOper::Sub;
        case IrOper::Multiply:     return AsmOper::Mul;

        case IrOper::BitwiseAnd:   return AsmOper::AndBitwise;
        case IrOper::BitwiseOr:    return AsmOper::OrBitwise;
        case IrOper::BitwiseXor:   return AsmOper::BitwiseXor;
        default:
            throw std::invalid_argument("Invalid BinaryOperation type: " +
                std::to_string(static_cast<int>(type)));
    }
}

BinaryInst::Operator getShiftOperator(const Ir::BinaryInst::Operation type, const bool isSigned)
{
    using IrOper = Ir::BinaryInst::Operation;
    using AsmOper = BinaryInst::Operator;
    if (isSigned) {
        switch (type) {
            case IrOper::LeftShift:    return AsmOper::LeftShiftSigned;
            case IrOper::RightShift:   return AsmOper::RightShiftSigned;
            default:
                throw std::invalid_argument("Invalid BinaryOperation type: " +
                    std::to_string(static_cast<int>(type)));
        }
    }
    switch (type) {
        case IrOper::LeftShift:    return AsmOper::LeftShiftUnsigned;
        case IrOper::RightShift:   return AsmOper::RightShiftUnsigned;
        default:
            throw std::invalid_argument("Invalid BinaryOperation type: " +
                std::to_string(static_cast<int>(type)));
    }
}

BinaryInst::CondCode condCode(const Ir::BinaryInst::Operation oper, const bool isSigned)
{
    using IrOper = Ir::BinaryInst::Operation;
    using BinCond = BinaryInst::CondCode;
    if (isSigned)
        switch (oper) {
            case IrOper::Equal:             return BinCond::E;
            case IrOper::NotEqual:          return BinCond::NE;
            case IrOper::LessThan:          return BinCond::L;
            case IrOper::LessOrEqual:       return BinCond::LE;
            case IrOper::GreaterThan:       return BinCond::G;
            case IrOper::GreaterOrEqual:    return BinCond::GE;
            default:
                throw std::invalid_argument("Invalid BinaryOperation type");
        }
    switch (oper) {
        case IrOper::Equal:             return BinCond::E;
        case IrOper::NotEqual:          return BinCond::NE;
        case IrOper::LessThan:          return BinCond::B;
        case IrOper::LessOrEqual:       return BinCond::BE;
        case IrOper::GreaterThan:       return BinCond::A;
        case IrOper::GreaterOrEqual:    return BinCond::AE;
        default:
                throw std::invalid_argument("Invalid BinaryOperation type");
    }
}

std::shared_ptr<Operand> GenerateAsmTree::genOperand(const std::shared_ptr<Ir::Value>& value)
{
    switch (value->kind) {
        case Ir::Value::Kind::Constant: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueConst = static_cast<Ir::ValueConst*>(value.get());
            if (valueConst->type == Type::Double) {
                Identifier constLabel(makeTemporaryPseudoName());
                m_programCodegen.topLevels.emplace_back(std::make_unique<ConstVariable>(
                    Identifier(constLabel), 8, std::get<double>(valueConst->value)));
                auto data = std::make_shared<DataOperand>(constLabel, AsmType::Double);
                auto pseudo = std::make_shared<PseudoOperand>(constLabel, ReferingTo::Local, AsmType::Double);
                insts.emplace_back(std::make_unique<MoveInst>(data, pseudo, AsmType::Double));
                return pseudo;
            }
            if (valueConst->type == Type::U32) {
                if (INT_MAX < std::get<u32>(valueConst->value)) {
                    const auto imm = std::make_shared<ImmOperand>(std::get<u32>(valueConst->value));
                    const auto reg10 = std::make_shared<RegisterOperand>(
                        RegType::R10, AsmType::QuadWord);
                    insts.emplace_back(std::make_unique<MoveInst>(imm, reg10, AsmType::QuadWord));
                    const auto pseudo = std::make_shared<PseudoOperand>(
                        Identifier(makeTemporaryPseudoName()), ReferingTo::Local, AsmType::QuadWord);
                    insts.emplace_back(std::make_unique<MoveInst>(reg10, pseudo, AsmType::QuadWord));
                    return pseudo;
                }
                return std::make_shared<ImmOperand>(std::get<u32>(valueConst->value));
            }
            if (valueConst->type == Type::U64) {
                if (INT_MAX < std::get<u64>(valueConst->value)) {
                    const auto imm = std::make_shared<ImmOperand>(std::get<u64>(valueConst->value));
                    const auto reg10 = std::make_shared<RegisterOperand>(
                        RegType::R10, AsmType::QuadWord);
                    insts.emplace_back(std::make_unique<MoveInst>(imm, reg10, AsmType::QuadWord));
                    const auto pseudo = std::make_shared<PseudoOperand>(
                        Identifier(makeTemporaryPseudoName()), ReferingTo::Local, AsmType::QuadWord);
                    insts.emplace_back(std::make_unique<MoveInst>(reg10, pseudo, AsmType::QuadWord));
                    return pseudo;
                }
                return std::make_shared<ImmOperand>(std::get<u64>(valueConst->value));
            }
            if (valueConst->type == Type::I32)
                return std::make_shared<ImmOperand>(std::get<i32>(valueConst->value));
            const auto imm = std::make_shared<ImmOperand>(std::get<i64>(valueConst->value));
            if (INT_MAX < std::get<i64>(valueConst->value)) {
                const auto reg10 = std::make_shared<RegisterOperand>(
                    RegType::R10, AsmType::QuadWord);
                insts.emplace_back(std::make_unique<MoveInst>(imm, reg10, AsmType::QuadWord));
                const auto pseudo = std::make_shared<PseudoOperand>(
                    Identifier(makeTemporaryPseudoName()), ReferingTo::Local, AsmType::QuadWord);
                insts.emplace_back(std::make_unique<MoveInst>(reg10, pseudo, AsmType::QuadWord));
                return pseudo;
            }
            return imm;
        }
        case Ir::Value::Kind::Variable: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueVar = static_cast<Ir::ValueVar*>(value.get());
            return std::make_shared<PseudoOperand>(
                Identifier(valueVar->value.value), valueVar->referingTo, getAsmType(valueVar->type));
        }
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

i32 replacingPseudoRegisters(const Function& function)
{
    PseudoRegisterReplacer pseudoRegisterReplacer;
    for (const auto& inst : function.instructions)
        inst->accept(pseudoRegisterReplacer);
    return pseudoRegisterReplacer.stackPointer();
}

void fixUpInstructions(Function& function, const i32 stackAlloc)
{
    FixUpInstructions fixUpInstructions(function.instructions, stackAlloc);
    fixUpInstructions.fixUp();
}

std::shared_ptr<ImmOperand> getZeroImmOfType(const AsmType type)
{
    if (type == AsmType::LongWord)
        return std::make_shared<ImmOperand>(0);
    if (type == AsmType::QuadWord)
        return std::make_shared<ImmOperand>(0l);
    std::abort();
}

}// namespace CodeGen