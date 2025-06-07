#include "GenerateAsmTree.hpp"
#include "PseudoRegisterReplacer.hpp"
#include "AsmAST.hpp"
#include "FixUpInstructions.hpp"

#include <cassert>
#include <random>
#include <stdexcept>

namespace {
using RegType = CodeGen::RegisterOperand::Kind;
const std::vector registerTypes = {RegType::DI, RegType::SI, RegType::DX,
                                   RegType::CX, RegType::R8, RegType::R9};
}

namespace CodeGen {

void GenerateAsmTree::generateProgram(const Ir::Program &program, Program &programCodegen)
{
    for (const auto& toplevelIr : program.topLevels)
        programCodegen.topLevels.push_back(std::move(generateTopLevel(*toplevelIr)));
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
        auto src = std::make_shared<RegisterOperand>(registerTypes[regIndex], AssemblyType::LongWord);
        auto arg = std::make_shared<Ir::ValueVar>(function.args[regIndex], function.argTypes[regIndex]);
        std::shared_ptr<Operand> dst = operand(arg);
        functionCodeGen->instructions.emplace_back(std::make_unique<MoveInst>(
            src, dst, getAssemblyType(function.argTypes[regIndex]))
            );
    }
    i32 stackPtr = 2;
    for (; regIndex < function.args.size(); ++regIndex, ++stackPtr) {
        auto stack = std::make_shared<StackOperand>(8 * stackPtr);
        auto arg = std::make_shared<Ir::ValueVar>(function.args[regIndex], function.argTypes[regIndex]);
        std::shared_ptr<Operand> dst = operand(arg);
        functionCodeGen->instructions.emplace_back(std::make_unique<MoveInst>(
            stack, dst, getAssemblyType(function.argTypes[regIndex])));
    }
    for (const std::unique_ptr<Ir::Instruction>& inst : function.insts)
        transformInst(inst);
    functionCodeGen->instructions = std::move(insts);
    return functionCodeGen;
}

std::unique_ptr<TopLevel> GenerateAsmTree::generateStaticVariable(const Ir::StaticVariable& staticVariable)
{
    const Type type = staticVariable.type;
    if (type == Type::I32)
        return std::make_unique<StaticVariable>(staticVariable.name,
                                                staticVariable.global,
                                                getAssemblyType(staticVariable.type),
                                                staticVariable.global);
    return std::make_unique<StaticVariable>(staticVariable.name,
                                        staticVariable.global,
                                        getAssemblyType(staticVariable.type),
                                        staticVariable.global);
}

void GenerateAsmTree::transformInst(const std::unique_ptr<Ir::Instruction>& inst)
{
    switch (inst->kind) {
        case Ir::Instruction::Kind::Unary: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irUnary = static_cast<Ir::UnaryInst*>(inst.get());
            unaryInst(*irUnary);
            break;
        }
        case Ir::Instruction::Kind::Return: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irReturn = static_cast<Ir::ReturnInst*>(inst.get());
            returnInst(*irReturn);
            break;
        }
        case Ir::Instruction::Kind::Binary: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irBinary = static_cast<Ir::BinaryInst*>(inst.get());
            binaryInst(*irBinary);
            break;
        }
        case Ir::Instruction::Kind::Label: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irLabel = static_cast<Ir::LabelInst*>(inst.get());
            generateLabelInst(*irLabel);
            break;
        }
        case Ir::Instruction::Kind::Jump: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irJump = static_cast<Ir::JumpInst*>(inst.get());
            generateJumpInst(*irJump);
            break;
        }
        case Ir::Instruction::Kind::JumpIfZero: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irJumpIfZero = static_cast<Ir::JumpIfZeroInst*>(inst.get());
            generateJumpIfZeroInst(*irJumpIfZero);
            break;
        }
        case Ir::Instruction::Kind::JumpIfNotZero: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irJumpIfNotZero = static_cast<Ir::JumpIfNotZeroInst*>(inst.get());
            generateJumpIfNotZeroInst(*irJumpIfNotZero);
            break;
        }
        case Ir::Instruction::Kind::Copy: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto irCopy = static_cast<Ir::CopyInst*>(inst.get());
            generateCopyInst(*irCopy);
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

void GenerateAsmTree::generateJumpInst(const Ir::JumpInst& irJump)
{
    Identifier iden(irJump.target.value);
    insts.push_back(std::make_unique<JmpInst>(iden));
}

void GenerateAsmTree::generateJumpIfZeroInst(const Ir::JumpIfZeroInst& jumpIfZero)
{
    std::shared_ptr<Operand> condition = operand(jumpIfZero.condition);
    insts.push_back(std::make_unique<CmpInst>(std::make_shared<ImmOperand>(0), condition, AssemblyType::LongWord));

    Identifier target(jumpIfZero.target.value);
    insts.push_back(std::make_unique<JmpCCInst>(BinaryInst::CondCode::E, target));
}

void GenerateAsmTree::generateJumpIfNotZeroInst(const Ir::JumpIfNotZeroInst& jumpIfNotZero)
{
    std::shared_ptr<Operand> condition = operand(jumpIfNotZero.condition);
    insts.push_back(std::make_unique<CmpInst>(std::make_shared<ImmOperand>(0), condition, AssemblyType::LongWord));

    Identifier target(jumpIfNotZero.target.value);
    insts.push_back(std::make_unique<JmpCCInst>(BinaryInst::CondCode::NE, target));
}

void GenerateAsmTree::generateCopyInst(const Ir::CopyInst& type)
{
    std::shared_ptr<Operand> src = operand(type.source);
    std::shared_ptr<Operand> dst = operand(type.destination);
    insts.push_back(std::make_unique<MoveInst>(src, dst, getAssemblyType(type.type)));
}

void GenerateAsmTree::generateLabelInst(const Ir::LabelInst& irLabel)
{
    Identifier label(irLabel.target.value);
    insts.push_back(std::make_unique<LabelInst>(label));
}

void GenerateAsmTree::unaryInst(const Ir::UnaryInst& irUnary)
{
    if (irUnary.operation == Ir::UnaryInst::Operation::Not) {
        generateUnaryNotInst(irUnary);
        return;
    }
    UnaryInst::Operator oper = unaryOperator(irUnary.operation);
    std::shared_ptr<Operand> src = operand(irUnary.source);
    std::shared_ptr<Operand> dst = operand(irUnary.destination);
    insts.emplace_back(std::make_unique<MoveInst>(src, dst, getAssemblyType(irUnary.type)));
    insts.emplace_back(std::make_unique<UnaryInst>(dst, oper, getAssemblyType(irUnary.type)));
}

void GenerateAsmTree::generateUnaryNotInst(const Ir::UnaryInst& irUnary)
{
    std::shared_ptr<Operand> src = operand(irUnary.source);
    auto immOperand = std::make_shared<ImmOperand>(0);
    insts.push_back(std::make_unique<CmpInst>(immOperand, src, getAssemblyType(irUnary.type)));

    std::shared_ptr<Operand> dst = operand(irUnary.destination);
    insts.push_back(std::make_unique<MoveInst>(immOperand, dst, getAssemblyType(irUnary.type)));

    insts.push_back(std::make_unique<SetCCInst>(BinaryInst::CondCode::E, dst));
}

void GenerateAsmTree::binaryInst(const Ir::BinaryInst& irBinary)
{
    using IrOper = Ir::BinaryInst::Operation;
    switch (irBinary.operation) {
        case IrOper::Add:
        case IrOper::Subtract:
        case IrOper::Multiply:
        case IrOper::BitwiseAnd:
        case IrOper::BitwiseOr:
        case IrOper::BitwiseXor:
        case IrOper::LeftShift:
        case IrOper::RightShift:
            generateBinaryBasicInst(irBinary);
            break;
        case IrOper::Divide:
            generateBinaryDivideInst(irBinary);
            break;
        case IrOper::Remainder:
            generateBinaryRemainderInst(irBinary);
            break;
        case IrOper::Equal:
        case IrOper::NotEqual:
        case IrOper::LessThan:
        case IrOper::LessOrEqual:
        case IrOper::GreaterThan:
        case IrOper::GreaterOrEqual:
            generateBinaryCondInst(irBinary);
            break;
        default:
            throw std::runtime_error("Unsupported binary operation");
    }
}

void GenerateAsmTree::generateBinaryCondInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.source1);
    std::shared_ptr<Operand> src2 = operand(irBinary.source2);
    insts.push_back(std::make_unique<CmpInst>(src2, src1, getAssemblyType(irBinary.type)));

    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    std::shared_ptr<Operand> imm = std::make_shared<ImmOperand>(0);
    insts.push_back(std::make_unique<MoveInst>(imm, dst, getAssemblyType(irBinary.type)));

    BinaryInst::CondCode cc = condCode(irBinary.operation);
    insts.push_back(std::make_unique<SetCCInst>(cc, dst));
}

void GenerateAsmTree::generateBinaryDivideInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.source1);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegisterOperand::Kind::AX, getAssemblyType(irBinary.type));
    insts.push_back(std::make_unique<MoveInst>(src1, regAX, getAssemblyType(irBinary.type)));

    insts.push_back(std::make_unique<CdqInst>(getAssemblyType(irBinary.type)));

    std::shared_ptr<Operand> src2 = operand(irBinary.source2);
    insts.push_back(std::make_unique<IdivInst>(src2, getAssemblyType(irBinary.type)));

    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    insts.push_back(std::make_unique<MoveInst>(regAX, dst, getAssemblyType(irBinary.type)));
}

void GenerateAsmTree::generateBinaryRemainderInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.source1);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(
        RegisterOperand::Kind::AX, getAssemblyType(irBinary.type));
    insts.push_back(std::make_unique<MoveInst>(src1, regAX, getAssemblyType(irBinary.type)));

    insts.push_back(std::make_unique<CdqInst>(getAssemblyType(irBinary.type)));

    std::shared_ptr<Operand> src2 = operand(irBinary.source2);
    insts.push_back(std::make_unique<IdivInst>(src2, getAssemblyType(irBinary.type)));

    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    const auto regDX = std::make_shared<RegisterOperand>(
        RegisterOperand::Kind::DX, getAssemblyType(irBinary.type));
    insts.push_back(std::make_unique<MoveInst>(regDX, dst, getAssemblyType(irBinary.type)));
}

void GenerateAsmTree::generateBinaryBasicInst(const Ir::BinaryInst& irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary.source1);
    std::shared_ptr<Operand> dst = operand(irBinary.destination);
    insts.push_back(std::make_unique<MoveInst>(src1, dst, getAssemblyType(irBinary.type)));

    BinaryInst::Operator oper = binaryOperator(irBinary.operation);
    std::shared_ptr<Operand> src2 = operand(irBinary.source2);
    insts.push_back(std::make_unique<BinaryInst>(src2, dst, oper, getAssemblyType(irBinary.type)));
}

void GenerateAsmTree::returnInst(const Ir::ReturnInst& returnInst)
{
    std::shared_ptr<Operand> val = operand(returnInst.returnValue);
    std::shared_ptr<Operand> operandRegister = std::make_shared<RegisterOperand>(
        RegisterOperand::Kind::AX, getAssemblyType(returnInst.type));
    insts.push_back(std::make_unique<MoveInst>(val, operandRegister, getAssemblyType(returnInst.type)));

    insts.push_back(std::make_unique<ReturnInst>());
}

void GenerateAsmTree::generateFunCallInst(const Ir::FunCallInst& type)
{
    const i32 stackPadding = getStackPadding(type.args.size());
    if (0 < stackPadding)
        insts.emplace_back(std::make_unique<BinaryInst>(
            std::make_shared<ImmOperand>(8),
            std::make_shared<RegisterOperand>(RegisterOperand::Kind::SP, AssemblyType::QuadWord),
            BinaryInst::Operator::Sub, AssemblyType::QuadWord));
    pushFunCallArgs(type);
    insts.push_back(std::make_unique<CallInst>(Identifier(type.funName.value)));
    const i32 bytesToRemove = 8 * (type.args.size() - 6) + stackPadding;
    if (0 < bytesToRemove)
        insts.emplace_back(std::make_unique<BinaryInst>(
        std::make_shared<ImmOperand>(bytesToRemove),
        std::make_shared<RegisterOperand>(RegisterOperand::Kind::SP, AssemblyType::QuadWord),
        BinaryInst::Operator::Sub,
        AssemblyType::QuadWord));
    std::shared_ptr<Operand> destination = operand(type.destination);
    insts.emplace_back(std::make_unique<MoveInst>(
        std::make_shared<RegisterOperand>(RegisterOperand::Kind::AX, AssemblyType::LongWord),
        destination,
        getAssemblyType(type.type)));
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
                src, std::make_shared<RegisterOperand>(RegisterOperand::Kind::AX, type),
                type));
            insts.emplace_back(std::make_unique<PushInst>(
                std::make_shared<RegisterOperand>(RegisterOperand::Kind::AX, type))
            );
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

        case IrOper::LeftShift:    return AsmOper::LeftShift;
        case IrOper::RightShift:   return AsmOper::RightShift;
        default:
            throw std::invalid_argument("Invalid BinaryOperation type: " + std::to_string(static_cast<int>(type)));
    }
}

BinaryInst::CondCode condCode(const Ir::BinaryInst::Operation type)
{
    using IrOper = Ir::BinaryInst::Operation;
    using BinCond = BinaryInst::CondCode;
    switch (type) {
        case IrOper::Equal:             return BinCond::E;
        case IrOper::NotEqual:          return BinCond::NE;
        case IrOper::LessThan:          return BinCond::L;
        case IrOper::LessOrEqual:       return BinCond::LE;
        case IrOper::GreaterThan:       return BinCond::G;
        case IrOper::GreaterOrEqual:    return BinCond::GE;
        default:
            throw std::invalid_argument("Invalid BinaryOperation type");
    }
}

std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value)
{
    switch (value->kind) {
        case Ir::Value::Kind::Constant: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueConst = static_cast<Ir::ValueConst*>(value.get());
            if (valueConst->type == Type::I32)
                return std::make_shared<ImmOperand>(std::get<i32>(valueConst->value));
            return std::make_shared<ImmOperand>(std::get<i64>(valueConst->value));
        }
        case Ir::Value::Kind::Variable: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueConst = static_cast<Ir::ValueVar*>(value.get());
            return std::make_shared<PseudoOperand>(valueConst->value.value);
        }
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

i32 replacingPseudoRegisters(Function& function, const SymbolTable& symbolTable)
{
    PseudoRegisterReplacer pseudoRegisterReplacer(symbolTable);
    for (auto& inst : function.instructions)
        inst->accept(pseudoRegisterReplacer);
    return pseudoRegisterReplacer.stackPointer();
}

void fixUpInstructions(Function& function, const i32 stackAlloc)
{
    FixUpInstructions fixUpInstructions(function.instructions, stackAlloc);
    fixUpInstructions.fixUp();
}

}// namespace CodeGen