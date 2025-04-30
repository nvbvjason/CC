#include "ConcreteTree.hpp"
#include "IR/ConcreteTree.hpp"
#include "PseudoRegisterReplacer.hpp"
#include "AbstractTree.hpp"

#include <stdexcept>

#include "FixUpInstructions.hpp"

namespace CodeGen {

void program(const Ir::Program &program, Program &programCodegen)
{
    programCodegen.function = function(program.function.get());
}

std::unique_ptr<Function> function(const Ir::Function *function)
{
    auto functionCodeGen = std::make_unique<Function>();
    functionCodeGen->name = function->identifier;
    for (const std::shared_ptr<Ir::Instruction>& inst : function->instructions) {
        switch (inst->type) {
            case Ir::Instruction::Type::Unary: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irUnary = static_cast<Ir::UnaryInst*>(inst.get());
                unaryInst(functionCodeGen->instructions, irUnary);
                break;
            }
            case Ir::Instruction::Type::Return: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irReturn = static_cast<Ir::ReturnInst*>(inst.get());
                returnInst(functionCodeGen->instructions, irReturn);
                break;
            }
            case Ir::Instruction::Type::Binary: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irBinary = static_cast<Ir::BinaryInst*>(inst.get());
                binaryInst(functionCodeGen->instructions, irBinary);
                break;
            }
            case Ir::Instruction::Type::Label: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irLabel = static_cast<Ir::LabelInst*>(inst.get());
                generateLabelInst(functionCodeGen->instructions, irLabel);
                break;
            }
            case Ir::Instruction::Type::Jump: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irJump = static_cast<Ir::JumpInst*>(inst.get());
                generateJumpInst(functionCodeGen->instructions, irJump);
            }
            case Ir::Instruction::Type::JumpIfZero: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irJumpIfZero = static_cast<Ir::JumpIfZeroInst*>(inst.get());
                generateJumpIfZeroInst(functionCodeGen->instructions, irJumpIfZero);
                break;
            }
            case Ir::Instruction::Type::JumpIfNotZero: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irJumpIfNotZero = static_cast<Ir::JumpIfNotZeroInst*>(inst.get());
                generateJumpIfNotZeroInst(functionCodeGen->instructions, irJumpIfNotZero);
                break;
            }
            case Ir::Instruction::Type::Copy: {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto irCopy = static_cast<Ir::CopyInst*>(inst.get());
                generateCopyInst(functionCodeGen->instructions, irCopy);
                break;
            }
            default:
                throw std::runtime_error("Unsupported instruction type");
        }
    }
    return functionCodeGen;
}

void generateJumpInst(std::vector<std::unique_ptr<Inst>>& insts,
                      const Ir::JumpInst* irJump)
{
    Identifier iden(irJump->target.value);
    insts.push_back(std::make_unique<JmpInst>(iden));
}

void generateJumpIfZeroInst(std::vector<std::unique_ptr<Inst>>& insts,
                            const Ir::JumpIfZeroInst* jumpIfZero)
{
    std::shared_ptr<Operand> condition = operand(jumpIfZero->condition);
    insts.push_back(std::make_unique<CmpInst>(std::make_shared<ImmOperand>(0), condition));

    Identifier target(jumpIfZero->target.value);
    insts.push_back(std::make_unique<JmpCCInst>(BinaryInst::CondCode::E, target));
}

void generateJumpIfNotZeroInst(std::vector<std::unique_ptr<Inst>>& insts,
                               const Ir::JumpIfNotZeroInst* jumpIfNotZero)
{
    std::shared_ptr<Operand> condition = operand(jumpIfNotZero->condition);
    insts.push_back(std::make_unique<CmpInst>(std::make_shared<ImmOperand>(0), condition));

    Identifier target(jumpIfNotZero->target.value);
    insts.push_back(std::make_unique<JmpCCInst>(BinaryInst::CondCode::NE, target));
}

void generateCopyInst(std::vector<std::unique_ptr<Inst>>& insts, Ir::CopyInst* type)
{
    std::shared_ptr<Operand> src = operand(type->source);
    std::shared_ptr<Operand> dst = operand(type->destination);
    insts.push_back(std::make_unique<MoveInst>(src, dst));
}

void generateLabelInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::LabelInst* irLabel)
{
    Identifier label(irLabel->target.value);
    insts.push_back(std::make_unique<LabelInst>(label));
}

void unaryInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary)
{
    if (irUnary->operation == Ir::UnaryInst::Operation::Not) {
        generateUnaryNotInst(insts, irUnary);
        return;
    }
    UnaryInst::Operator oper = unaryOperator(irUnary->operation);
    std::shared_ptr<Operand> src = operand(irUnary->source);
    std::shared_ptr<Operand> dst = operand(irUnary->destination);
    insts.push_back(std::make_unique<MoveInst>(src, dst));

    insts.push_back(std::make_unique<UnaryInst>(oper, dst));
}

void generateUnaryNotInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary)
{
    std::shared_ptr<Operand> src = operand(irUnary->source);
    auto immOperand = std::make_shared<ImmOperand>(0);
    insts.push_back(std::make_unique<CmpInst>(immOperand, src));

    std::shared_ptr<Operand> dst = operand(irUnary->destination);
    insts.push_back(std::make_unique<MoveInst>(immOperand, dst));

    insts.push_back(std::make_unique<SetCCInst>(BinaryInst::CondCode::E, dst));
}

void binaryInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    using IrOper = Ir::BinaryInst::Operation;
    switch (irBinary->operation) {
        case IrOper::Add:
        case IrOper::Subtract:
        case IrOper::Multiply:
        case IrOper::BitwiseAnd:
        case IrOper::BitwiseOr:
        case IrOper::BitwiseXor:
        case IrOper::LeftShift:
        case IrOper::RightShift:
            generateBinaryBasicInst(insts, irBinary);
            break;
        case IrOper::Divide:
            generateBinaryDivideInst(insts, irBinary);
            break;
        case IrOper::Remainder:
            generateBinaryRemainderInst(insts, irBinary);
            break;
        case IrOper::Equal:
        case IrOper::NotEqual:
        case IrOper::LessThan:
        case IrOper::LessOrEqual:
        case IrOper::GreaterThan:
        case IrOper::GreaterOrEqual:
            generateBinaryCondInst(insts, irBinary);
            break;
        default:
            throw std::runtime_error("Unsupported binary operation");
    }
}


void generateBinaryDivideInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(RegisterOperand::Type::AX);
    insts.push_back(std::make_unique<MoveInst>(src1, regAX));

    insts.push_back(std::make_unique<CdqInst>());

    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    insts.push_back(std::make_unique<IdivInst>(src2));

    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    insts.push_back(std::make_unique<MoveInst>(regAX, dst));
}

void generateBinaryCondInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    insts.push_back(std::make_unique<CmpInst>(src2, src1));

    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    std::shared_ptr<Operand> imm = std::make_shared<ImmOperand>(0);
    insts.push_back(std::make_unique<MoveInst>(imm, dst));

    BinaryInst::CondCode cc = condCode(irBinary->operation);
    insts.push_back(std::make_unique<SetCCInst>(cc, dst));
}

void generateBinaryRemainderInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(RegisterOperand::Type::AX);
    insts.push_back(std::make_unique<MoveInst>(src1, regAX));

    insts.push_back(std::make_unique<CdqInst>());

    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    insts.push_back(std::make_unique<IdivInst>(src2));

    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    const auto regDX = std::make_shared<RegisterOperand>(RegisterOperand::Type::DX);
    insts.push_back(std::make_unique<MoveInst>(regDX, dst));
}

void generateBinaryBasicInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
{
    std::shared_ptr<Operand> src1 = operand(irBinary->source1);
    std::shared_ptr<Operand> dst = operand(irBinary->destination);
    insts.push_back(std::make_unique<MoveInst>(src1, dst));

    BinaryInst::Operator oper = binaryOperator(irBinary->operation);
    std::shared_ptr<Operand> src2 = operand(irBinary->source2);
    insts.push_back(std::make_unique<BinaryInst>(oper, src2, dst));
}

void returnInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::ReturnInst* inst)
{
    std::shared_ptr<Operand> val = operand(inst->returnValue);
    std::shared_ptr<Operand> operandRegister = std::make_shared<RegisterOperand>(RegisterOperand::Type::AX);
    insts.push_back(std::make_unique<MoveInst>(val, operandRegister));

    insts.push_back(std::make_unique<ReturnInst>());
}

UnaryInst::Operator unaryOperator(const Ir::UnaryInst::Operation type)
{
    switch (type)
    {
        case Ir::UnaryInst::Operation::Complement:
            return UnaryInst::Operator::Not;
        case Ir::UnaryInst::Operation::Negate:
            return UnaryInst::Operator::Neg;
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
    switch (value->type) {
        case Ir::Value::Type::Constant: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueConst = static_cast<Ir::ValueConst*>(value.get());
            return std::make_shared<ImmOperand>(valueConst->value);
        }
        case Ir::Value::Type::Variable: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            const auto valueConst = static_cast<Ir::ValueVar*>(value.get());
            return std::make_shared<PseudoOperand>(valueConst->value.value);
        }
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

i32 replacingPseudoRegisters(Program& programCodegen)
{
    PseudoRegisterReplacer pseudoRegisterReplacer;
    for (std::unique_ptr<Inst>& inst : programCodegen.function->instructions)
        inst->accept(pseudoRegisterReplacer);
    return pseudoRegisterReplacer.stackPointer();
}

void fixUpInstructions(Program& programCodegen, const i32 stackAlloc)
{
    FixUpInstructions fixUpInstructions(programCodegen.function->instructions, stackAlloc);
    fixUpInstructions.fixUp();
}

}// namespace CodeGen
