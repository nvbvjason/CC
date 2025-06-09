#include "GenerateAsmTree.hpp"
#include "PseudoRegisterReplacer.hpp"
#include "AsmAST.hpp"
#include "FixUpInstructions.hpp"

#include <cassert>
#include <random>

namespace {
using RegType = CodeGen::RegisterOperand::Kind;
const std::vector registerTypes = {RegType::DI, RegType::SI, RegType::DX,
                                   RegType::CX, RegType::R8, RegType::R9};
}

namespace CodeGen {

void GenerateAsmTree::generateProgram(const Ir::Program &program, Program &programCodegen)
{
    for (const auto& toplevelIr : program.topLevels)
        programCodegen.topLevels.emplace_back(std::move(generateTopLevel(*toplevelIr)));
}

std::unique_ptr<TopLevel> GenerateAsmTree::generateTopLevel(const Ir::TopLevel& topLevel)
{
    using Type = Ir::TopLevel::Kind;
    switch (topLevel.type) {
        case Type::Function: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto function = static_cast<const Ir::Function*>(&topLevel);
            return generateFunction(*function);
        }
        case Type::StaticVariable: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto staticVariable = static_cast<const Ir::StaticVariable*>(&topLevel);
            return generateStaticVariable(*staticVariable);
        }
        assert("generateTopLevel idk type");
    }
    std::unreachable();
}

std::unique_ptr<TopLevel> GenerateAsmTree::generateFunction(const Ir::Function& function)
{
    auto functionCodeGen = std::make_unique<Function>(function.name, function.isGlobal);
    insts = std::move(functionCodeGen->instructions);
    i32 regIndex = 0;
    for (; regIndex < function.args.size() && regIndex < registerTypes.size(); ++regIndex) {
        auto src = std::make_shared<RegisterOperand>(registerTypes[regIndex], getAssemblyType(function.argTypes[regIndex]));
        auto arg = std::make_shared<Ir::ValueVar>(function.args[regIndex], function.argTypes[regIndex]);
        std::shared_ptr<Operand> dst = operand(arg);
        insts.emplace_back(std::make_unique<MoveInst>(
            src, dst, getAssemblyType(function.argTypes[regIndex]))
            );
    }
    i32 stackPtr = 2;
    for (; regIndex < function.args.size(); ++regIndex, ++stackPtr) {
        auto stack = std::make_shared<StackOperand>(8 * stackPtr, getAssemblyType(function.argTypes[regIndex]));
        auto arg = std::make_shared<Ir::ValueVar>(function.args[regIndex], function.argTypes[regIndex]);
        std::shared_ptr<Operand> dst = operand(arg);
        insts.emplace_back(std::make_unique<MoveInst>(
            stack, dst, getAssemblyType(function.argTypes[regIndex])));
    }
    for (const std::unique_ptr<Ir::Instruction>& inst : function.insts)
        transformInst(inst);
    functionCodeGen->instructions = std::move(insts);
    return functionCodeGen;
}

std::unique_ptr<TopLevel> generateStaticVariable(const Ir::StaticVariable& staticVariable)
{
    const Type type = staticVariable.type;
    const auto value = static_cast<const Ir::ValueConst*>(staticVariable.value.get());
    if (type == Type::I32)
        return std::make_unique<StaticVariable>(staticVariable.name,
                                                std::get<i32>(value->value),
                                                getAssemblyType(staticVariable.type),
                                                staticVariable.global);
    if (type == Type::I64)
        return std::make_unique<StaticVariable>(staticVariable.name,
                                                std::get<i64>(value->value),
                                                getAssemblyType(staticVariable.type),
                                                staticVariable.global);
    if (type == Type::U32)
        return std::make_unique<StaticVariable>(staticVariable.name,
                                                std::get<u32>(value->value),
                                                getAssemblyType(staticVariable.type),
                                                staticVariable.global);
    if (type == Type::U64)
        return std::make_unique<StaticVariable>(staticVariable.name,
                                                std::get<u64>(value->value),
                                                getAssemblyType(staticVariable.type),
                                                staticVariable.global);
    std::abort();
}

void GenerateAsmTree::transformInst(const std::unique_ptr<Ir::Instruction>& inst)
{
    switch (inst->kind) {
        case Ir::Instruction::Kind::Return: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irReturn = static_cast<Ir::ReturnInst*>(inst.get());
            returnInst(*irReturn);
            break;
        }
        case Ir::Instruction::Kind::ZeroExtend: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto zeroExtend = static_cast<Ir::ZeroExtendInst*>(inst.get());
            genZeroExtendInst(*zeroExtend);
            break;
        }
        case Ir::Instruction::Kind::SignExtend: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto signExtend = static_cast<Ir::SignExtendInst*>(inst.get());
            genSignExtendInst(*signExtend);
            break;
        }
        case Ir::Instruction::Kind::Truncate: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto truncate = static_cast<Ir::TruncateInst*>(inst.get());
            genTruncateInst(*truncate);
            break;
        }
        case Ir::Instruction::Kind::Unary: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irUnary = static_cast<Ir::UnaryInst*>(inst.get());
            unaryInst(*irUnary);
            break;
        }
        case Ir::Instruction::Kind::Binary: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irBinary = static_cast<Ir::BinaryInst*>(inst.get());
            genBinaryInst(*irBinary);
            break;
        }
        case Ir::Instruction::Kind::Copy: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irCopy = static_cast<Ir::CopyInst*>(inst.get());
            genCopyInst(*irCopy);
            break;
        }
        case Ir::Instruction::Kind::Jump: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irJump = static_cast<Ir::JumpInst*>(inst.get());
            genJumpInst(*irJump);
            break;
        }
        case Ir::Instruction::Kind::JumpIfZero: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irJumpIfZero = static_cast<Ir::JumpIfZeroInst*>(inst.get());
            genJumpIfZeroInst(*irJumpIfZero);
            break;
        }
        case Ir::Instruction::Kind::JumpIfNotZero: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irJumpIfNotZero = static_cast<Ir::JumpIfNotZeroInst*>(inst.get());
            genJumpIfNotZeroInst(*irJumpIfNotZero);
            break;
        }
        case Ir::Instruction::Kind::Label: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irLabel = static_cast<Ir::LabelInst*>(inst.get());
            genLabelInst(*irLabel);
            break;
        }
        case Ir::Instruction::Kind::FunCall: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irFunCall = static_cast<Ir::FunCallInst*>(inst.get());
            generateFunCallInst(*irFunCall);
            break;
        }
        default:
            throw std::runtime_error("Unsupported instruction type");
    }
}

void GenerateAsmTree::genJumpInst(const Ir::JumpInst& irJump)
{
    Identifier iden(irJump.target.value);
    insts.emplace_back(std::make_unique<JmpInst>(iden));
}

void GenerateAsmTree::genJumpIfZeroInst(const Ir::JumpIfZeroInst& jumpIfZero)
{
    std::shared_ptr<Operand> condition = operand(jumpIfZero.condition);
    std::shared_ptr<ImmOperand> zero = getZeroImmOfType(condition->type);
    insts.emplace_back(std::make_unique<CmpInst>(
        zero, condition, condition->type));

    Identifier target(jumpIfZero.target.value);
    insts.emplace_back(std::make_unique<JmpCCInst>(BinaryInst::CondCode::E, target));
}

void GenerateAsmTree::genJumpIfNotZeroInst(const Ir::JumpIfNotZeroInst& jumpIfNotZero)
{
    std::shared_ptr<Operand> condition = operand(jumpIfNotZero.condition);
    std::shared_ptr<ImmOperand> zero = getZeroImmOfType(condition->type);
    insts.emplace_back(std::make_unique<CmpInst>(
        zero, condition, condition->type));
    Identifier target(jumpIfNotZero.target.value);
    insts.emplace_back(std::make_unique<JmpCCInst>(BinaryInst::CondCode::NE, target));
}

void GenerateAsmTree::genCopyInst(const Ir::CopyInst& type)
{
    std::shared_ptr<Operand> src = operand(type.source);
    std::shared_ptr<Operand> dst = operand(type.destination);
    insts.emplace_back(std::make_unique<MoveInst>(src, dst, src->type));
}

void GenerateAsmTree::genLabelInst(const Ir::LabelInst& irLabel)
{
    Identifier label(irLabel.target.value);
    insts.emplace_back(std::make_unique<LabelInst>(label));
}

void GenerateAsmTree::unaryInst(const Ir::UnaryInst& irUnary)
{
    if (irUnary.operation == Ir::UnaryInst::Operation::Not) {
        genUnaryNotInst(irUnary);
        return;
    }
    UnaryInst::Operator oper = unaryOperator(irUnary.operation);
    std::shared_ptr<Operand> src = operand(irUnary.source);
    std::shared_ptr<Operand> dst = operand(irUnary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(src, dst, src->type));
    insts.emplace_back(std::make_unique<UnaryInst>(dst, oper, src->type));
}

void GenerateAsmTree::genUnaryNotInst(const Ir::UnaryInst& irUnary)
{
    std::shared_ptr<Operand> src = operand(irUnary.source);
    std::shared_ptr<ImmOperand> immOperand = getZeroImmOfType(src->type);
    insts.emplace_back(std::make_unique<CmpInst>(immOperand, src, src->type));

    std::shared_ptr<Operand> dst = operand(irUnary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(immOperand, dst, dst->type));

    insts.emplace_back(std::make_unique<SetCCInst>(BinaryInst::CondCode::E, dst));
}

void GenerateAsmTree::genBinaryInst(const Ir::BinaryInst& irBinary)
{
    using IrOper = Ir::BinaryInst::Operation;
    switch (irBinary.operation) {
        case IrOper::Add:
        case IrOper::Subtract:
        case IrOper::Multiply:
        case IrOper::BitwiseAnd:
        case IrOper::BitwiseOr:
        case IrOper::BitwiseXor:
            genBinaryBasicInst(irBinary);
            break;
        case IrOper::LeftShift:
        case IrOper::RightShift:
            genBinaryShiftInst(irBinary);
            break;
        case IrOper::Divide:
            genBinaryDivideInst(irBinary);
            break;
        case IrOper::Remainder:
            genBinaryRemainderInst(irBinary);
            break;
        case IrOper::Equal:
        case IrOper::NotEqual:
        case IrOper::LessThan:
        case IrOper::LessOrEqual:
        case IrOper::GreaterThan:
        case IrOper::GreaterOrEqual:
            genBinaryCondInst(irBinary);
            break;
        default:
            assert("Unsupported binary operation");
    }
    std::unreachable();
}

void GenerateAsmTree::genZeroExtendInst(const Ir::ZeroExtendInst& zeroExtend)
{
    std::shared_ptr<Operand> src1 = operand(zeroExtend.src);
    std::shared_ptr<Operand> src2 = operand(zeroExtend.dst);
    insts.emplace_back(std::make_unique<MoveZeroExtendInst>(src1, src2, src1->type));
}

void GenerateAsmTree::genSignExtendInst(const Ir::SignExtendInst& signExtend)
{
    std::shared_ptr<Operand> src1 = operand(signExtend.src);
    std::shared_ptr<Operand> src2 = operand(signExtend.dst);
    insts.emplace_back(std::make_unique<MoveSXInst>(src1, src2));
}

void GenerateAsmTree::genTruncateInst(const Ir::TruncateInst& truncate)
{
    std::shared_ptr<Operand> src1 = operand(truncate.src);
    std::shared_ptr<Operand> src2 = operand(truncate.dst);
    insts.emplace_back(std::make_unique<MoveInst>(src1, src2, AssemblyType::LongWord));
}

void GenerateAsmTree::genBinaryCondInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.lhs);
    std::shared_ptr<Operand> src2 = operand(irBinary.rhs);
    insts.emplace_back(std::make_unique<CmpInst>(src2, src1, src1->type));

    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    std::shared_ptr<Operand> imm = getZeroImmOfType(dst->type);
    insts.emplace_back(std::make_unique<MoveInst>(imm, dst, dst->type));
    const bool isSigned = irBinary.type == Type::I32 || irBinary.type == Type::I64;
    BinaryInst::CondCode cc = condCode(irBinary.operation, isSigned);
    insts.emplace_back(std::make_unique<SetCCInst>(cc, dst));
}

void GenerateAsmTree::genBinaryDivideInst(const Ir::BinaryInst& irBinary)
{
    if (irBinary.type == Type::I64 || irBinary.type == Type::I32) {
        genSignedBinaryDivideInst(irBinary);
        return;
    }
    genUnsignedBinaryDivideInst(irBinary);
}

void GenerateAsmTree::genSignedBinaryDivideInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.lhs);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegType::AX, getAssemblyType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(src1, regAX, src1->type));

    insts.emplace_back(std::make_unique<CdqInst>(src1->type));

    std::shared_ptr<Operand> src2 = operand(irBinary.rhs);
    insts.emplace_back(std::make_unique<IdivInst>(src2, src1->type));

    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(regAX, dst, src1->type));
}

void GenerateAsmTree::genUnsignedBinaryDivideInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.lhs);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegType::AX, getAssemblyType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(src1, regAX, src1->type));

    const auto zero = getZeroImmOfType(src1->type);
    const auto regDX = std::make_shared<RegisterOperand>(
        RegType::DX, getAssemblyType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(zero, regDX, src1->type));

    std::shared_ptr<Operand> src2 = operand(irBinary.rhs);
    insts.emplace_back(std::make_unique<DivInst>(src2, src1->type));

    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(regAX, dst, src1->type));
}

void GenerateAsmTree::genBinaryRemainderInst(const Ir::BinaryInst& irBinary)
{
    if (irBinary.type == Type::I64 || irBinary.type == Type::I32) {
        genSignedBinaryRemainderInst(irBinary);
        return;
    }
    genUnsignedBinaryRemainderInst(irBinary);
}

void GenerateAsmTree::genSignedBinaryRemainderInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.lhs);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegType::AX, getAssemblyType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(src1, regAX, src1->type));

    insts.emplace_back(std::make_unique<CdqInst>(src1->type));

    std::shared_ptr<Operand> src2 = operand(irBinary.rhs);
    insts.emplace_back(std::make_unique<IdivInst>(src2, src1->type));

    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    const auto regDX = std::make_shared<RegisterOperand>(
        RegType::DX, getAssemblyType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(regDX, dst, src1->type));
}

void GenerateAsmTree::genUnsignedBinaryRemainderInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.lhs);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegType::AX, getAssemblyType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(src1, regAX, src1->type));

    const auto zero = getZeroImmOfType(src1->type);
    const auto regDX = std::make_shared<RegisterOperand>(
        RegType::DX, getAssemblyType(irBinary.type));
    insts.emplace_back(std::make_unique<MoveInst>(zero, regDX, src1->type));

    std::shared_ptr<Operand> src2 = operand(irBinary.rhs);
    insts.emplace_back(std::make_unique<DivInst>(src2, src1->type));

    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(regDX, dst, src1->type));
}

void GenerateAsmTree::genBinaryBasicInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.lhs);
    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(src1, dst, src1->type));

    BinaryInst::Operator oper = binaryOperator(irBinary.operation);
    std::shared_ptr<Operand> src2 = operand(irBinary.rhs);
    insts.emplace_back(std::make_unique<BinaryInst>(src2, dst, oper, src1->type));
}

void GenerateAsmTree::genBinaryShiftInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> lhs = operand(irBinary.lhs);
    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(lhs, dst, lhs->type));

    const bool isSigned = irBinary.type == Type::I32 || irBinary.type == Type::I64;
    BinaryInst::Operator oper = getShiftOperator(irBinary.operation, isSigned);
    std::shared_ptr<Operand> rhs = operand(irBinary.rhs);
    insts.emplace_back(std::make_unique<BinaryInst>(rhs, dst, oper, lhs->type));
}

void GenerateAsmTree::returnInst(const Ir::ReturnInst& returnInst)
{
    std::shared_ptr<Operand> val = operand(returnInst.returnValue);
    std::shared_ptr<Operand> operandRegister = std::make_shared<RegisterOperand>(
        RegType::AX, getAssemblyType(returnInst.type));
    insts.emplace_back(std::make_unique<MoveInst>(
        val, operandRegister, getAssemblyType(returnInst.type)));

    insts.emplace_back(std::make_unique<ReturnInst>());
}

void GenerateAsmTree::generateFunCallInst(const Ir::FunCallInst& funcCall)
{
    const i32 stackPadding = getStackPadding(funcCall.args.size());
    if (0 < stackPadding)
        insts.emplace_back(std::make_unique<BinaryInst>(
            std::make_shared<ImmOperand>(8),
            std::make_shared<RegisterOperand>(RegType::SP, AssemblyType::QuadWord),
            BinaryInst::Operator::Sub, AssemblyType::QuadWord));
    pushFunCallArgs(funcCall);
    insts.emplace_back(std::make_unique<CallInst>(Identifier(funcCall.funName.value)));
    const i32 bytesToRemove = 8 * (funcCall.args.size() - 6) + stackPadding;
    if (0 < bytesToRemove)
        insts.emplace_back(std::make_unique<BinaryInst>(
        std::make_shared<ImmOperand>(bytesToRemove),
        std::make_shared<RegisterOperand>(RegType::SP, AssemblyType::QuadWord),
        BinaryInst::Operator::Add,
        AssemblyType::QuadWord));
    std::shared_ptr<Operand> destination = operand(funcCall.destination);
    insts.emplace_back(std::make_unique<MoveInst>(
        std::make_shared<RegisterOperand>(RegType::AX, getAssemblyType(funcCall.type)),
        destination,
        getAssemblyType(funcCall.type)));
}

void GenerateAsmTree::pushFunCallArgs(const Ir::FunCallInst& funcCall)
{
    i32 regIndex = 0;
    for (; regIndex < funcCall.args.size() && regIndex < registerTypes.size(); ++regIndex) {
        std::shared_ptr<Operand> src = operand(funcCall.args[regIndex]);
        const AssemblyType type = getAssemblyType(funcCall.args[regIndex]->type);
        insts.emplace_back(std::make_unique<MoveInst>(
            src,
            std::make_shared<RegisterOperand>(registerTypes[regIndex], type),
            type));
    }
    for (i32 i = funcCall.args.size() - 1; regIndex <= i; --i) {
        std::shared_ptr<Operand> src = operand(funcCall.args[i]);
        if (src->kind == Operand::Kind::Imm ||
            src->kind == Operand::Kind::Register ||
            funcCall.args[i]->type == Type::I64) {
            insts.emplace_back(std::make_unique<PushInst>(src));
        }
        else {
            const AssemblyType type = getAssemblyType(funcCall.args[i]->type);
            insts.emplace_back(std::make_unique<MoveInst>(
                src, std::make_shared<RegisterOperand>(RegType::AX, type),
                type));
            insts.emplace_back(std::make_unique<PushInst>(
                std::make_shared<RegisterOperand>(RegType::AX, AssemblyType::QuadWord)));
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

        case IrOper::BitwiseAnd:   return AsmOper::BitwiseAnd;
        case IrOper::BitwiseOr:    return AsmOper::BitwiseOr;
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

std::shared_ptr<Operand> GenerateAsmTree::operand(const std::shared_ptr<Ir::Value>& value)
{
    switch (value->kind) {
        case Ir::Value::Kind::Constant: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueConst = static_cast<Ir::ValueConst*>(value.get());
            if (valueConst->type == Type::U32) {
                if (INT_MAX < std::get<u32>(valueConst->value)) {
                    const auto imm = std::make_shared<ImmOperand>(std::get<u32>(valueConst->value));
                    const auto reg10 = std::make_shared<RegisterOperand>(
                        RegType::R10, AssemblyType::QuadWord);
                    insts.emplace_back(std::make_unique<MoveInst>(imm, reg10, AssemblyType::QuadWord));
                    const auto pseudo = std::make_shared<PseudoOperand>(
                        makeTemporaryPseudoName(), ReferingTo::Local, AssemblyType::QuadWord);
                    insts.emplace_back(std::make_unique<MoveInst>(reg10, pseudo, AssemblyType::QuadWord));
                    return pseudo;
                }
                return std::make_shared<ImmOperand>(std::get<u32>(valueConst->value));
            }
            if (valueConst->type == Type::U64) {
                if (INT_MAX < std::get<u64>(valueConst->value)) {
                    const auto imm = std::make_shared<ImmOperand>(std::get<u64>(valueConst->value));
                    const auto reg10 = std::make_shared<RegisterOperand>(
                        RegType::R10, AssemblyType::QuadWord);
                    insts.emplace_back(std::make_unique<MoveInst>(imm, reg10, AssemblyType::QuadWord));
                    const auto pseudo = std::make_shared<PseudoOperand>(
                        makeTemporaryPseudoName(), ReferingTo::Local, AssemblyType::QuadWord);
                    insts.emplace_back(std::make_unique<MoveInst>(reg10, pseudo, AssemblyType::QuadWord));
                    return pseudo;
                }
                return std::make_shared<ImmOperand>(std::get<u64>(valueConst->value));
            }
            if (valueConst->type == Type::I32)
                return std::make_shared<ImmOperand>(std::get<i32>(valueConst->value));
            const auto imm = std::make_shared<ImmOperand>(std::get<i64>(valueConst->value));
            if (INT_MAX < std::get<i64>(valueConst->value)) {
                const auto reg10 = std::make_shared<RegisterOperand>(
                    RegType::R10, AssemblyType::QuadWord);
                insts.emplace_back(std::make_unique<MoveInst>(imm, reg10, AssemblyType::QuadWord));
                const auto pseudo = std::make_shared<PseudoOperand>(
                    makeTemporaryPseudoName(), ReferingTo::Local, AssemblyType::QuadWord);
                insts.emplace_back(std::make_unique<MoveInst>(reg10, pseudo, AssemblyType::QuadWord));
                return pseudo;
            }
            return imm;
        }
        case Ir::Value::Kind::Variable: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueVar = static_cast<Ir::ValueVar*>(value.get());
            return std::make_shared<PseudoOperand>(
                valueVar->value.value, valueVar->referingTo, getAssemblyType(valueVar->type));
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

std::shared_ptr<ImmOperand> getZeroImmOfType(const AssemblyType type)
{
    if (type == AssemblyType::LongWord)
        return std::make_shared<ImmOperand>(0);
    if (type == AssemblyType::QuadWord)
        return std::make_shared<ImmOperand>(0l);
    std::abort();
}

}// namespace CodeGen