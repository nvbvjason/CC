#include "GenerateAsmTree.hpp"
#include "PseudoRegisterReplacer.hpp"
#include "AsmAST.hpp"
#include "FixUpInstructions.hpp"

#include <cassert>
#include <stdexcept>

namespace CodeGen {

void generateProgram(const Ir::Program &program, Program &programCodegen)
{
    // for (const auto& toplevelIr : program.topLevels)
    //     programCodegen.topLevels.push_back(std::move(generateTopLevel(*toplevelIr)));
}
//
// std::unique_ptr<TopLevel> generateTopLevel(const Ir::TopLevel& topLevel)
// {
//     using Type = Ir::TopLevel::Type;
//     switch (topLevel.type) {
//         case Type::Function: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto function = static_cast<const Ir::Function*>(&topLevel);
//             return generateFunction(*function);
//         }
//         case Type::StaticVariable: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto staticVariable = static_cast<const Ir::StaticVariable*>(&topLevel);
//             return generateStaticVariable(*staticVariable);
//         }
//         assert("generateTopLevel idk type");
//     }
//     std::unreachable();
// }
//
// std::unique_ptr<TopLevel> generateFunction(const Ir::Function& function)
// {
//     using RegType = RegisterOperand::Type;
//     static const std::vector<RegType> registerTypes = {RegType::DI, RegType::SI, RegType::DX,
//                                                        RegType::CX, RegType::R8, RegType::R9};
//     auto functionCodeGen = std::make_unique<Function>(function.name, function.isGlobal);
//     i32 regIndex = 0;
//     for (; regIndex < function.args.size() && regIndex < registerTypes.size(); ++regIndex) {
//         auto src = std::make_shared<RegisterOperand>(registerTypes[regIndex], 4);
//         auto arg = std::make_shared<Ir::ValueVar>(function.args[regIndex]);
//         std::shared_ptr<Operand> dst = operand(arg);
//         functionCodeGen->instructions.push_back(
//             std::make_unique<MoveInst>(src, dst)
//             );
//     }
//     i32 stackPtr = 2;
//     for (; regIndex < function.args.size(); ++regIndex, ++stackPtr) {
//         auto stack = std::make_shared<StackOperand>(8 * stackPtr);
//         auto arg = std::make_shared<Ir::ValueVar>(function.args[regIndex]);
//         std::shared_ptr<Operand> dst = operand(arg);
//         functionCodeGen->instructions.push_back(std::make_unique<MoveInst>(stack, dst));
//     }
//     for (const std::unique_ptr<Ir::Instruction>& inst : function.insts)
//         transformInst(functionCodeGen, inst);
//     return functionCodeGen;
// }
//
// std::unique_ptr<TopLevel> generateStaticVariable(const Ir::StaticVariable& staticVariable)
// {
//     return std::make_unique<StaticVariable>(staticVariable.name,
//                                             staticVariable.global,
//                                             getStaticVariableInitial(staticVariable));
// }
//
// void transformInst(const std::unique_ptr<Function>& functionCodeGen, const std::unique_ptr<Ir::Instruction>& inst)
// {
//     switch (inst->kind) {
//         case Ir::Instruction::Kind::Unary: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto irUnary = static_cast<Ir::UnaryInst*>(inst.get());
//             unaryInst(functionCodeGen->instructions, irUnary);
//             break;
//         }
//         case Ir::Instruction::Kind::Return: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto irReturn = static_cast<Ir::ReturnInst*>(inst.get());
//             returnInst(functionCodeGen->instructions, irReturn);
//             break;
//         }
//         case Ir::Instruction::Kind::Binary: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto irBinary = static_cast<Ir::BinaryInst*>(inst.get());
//             binaryInst(functionCodeGen->instructions, irBinary);
//             break;
//         }
//         case Ir::Instruction::Kind::Label: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto irLabel = static_cast<Ir::LabelInst*>(inst.get());
//             generateLabelInst(functionCodeGen->instructions, irLabel);
//             break;
//         }
//         case Ir::Instruction::Kind::Jump: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto irJump = static_cast<Ir::JumpInst*>(inst.get());
//             generateJumpInst(functionCodeGen->instructions, irJump);
//             break;
//         }
//         case Ir::Instruction::Kind::JumpIfZero: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto irJumpIfZero = static_cast<Ir::JumpIfZeroInst*>(inst.get());
//             generateJumpIfZeroInst(functionCodeGen->instructions, irJumpIfZero);
//             break;
//         }
//         case Ir::Instruction::Kind::JumpIfNotZero: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto irJumpIfNotZero = static_cast<Ir::JumpIfNotZeroInst*>(inst.get());
//             generateJumpIfNotZeroInst(functionCodeGen->instructions, irJumpIfNotZero);
//             break;
//         }
//         case Ir::Instruction::Kind::Copy: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto irCopy = static_cast<Ir::CopyInst*>(inst.get());
//             generateCopyInst(functionCodeGen->instructions, irCopy);
//             break;
//         }
//         case Ir::Instruction::Kind::FunCall: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto irFunCall = static_cast<Ir::FunCallInst*>(inst.get());
//             generateFunCallInst(functionCodeGen->instructions, irFunCall);
//             break;
//         }
//         default:
//             throw std::runtime_error("Unsupported instruction type");
//     }
// }
//
// void generateJumpInst(std::vector<std::unique_ptr<Inst>>& insts,
//                       const Ir::JumpInst* irJump)
// {
//     Identifier iden(irJump->target.value);
//     insts.push_back(std::make_unique<JmpInst>(iden));
// }
//
// void generateJumpIfZeroInst(std::vector<std::unique_ptr<Inst>>& insts,
//                             const Ir::JumpIfZeroInst* jumpIfZero)
// {
//     std::shared_ptr<Operand> condition = operand(jumpIfZero->condition);
//     insts.push_back(std::make_unique<CmpInst>(std::make_shared<ImmOperand>(0), condition));
//
//     Identifier target(jumpIfZero->target.value);
//     insts.push_back(std::make_unique<JmpCCInst>(BinaryInst::CondCode::E, target));
// }
//
// void generateJumpIfNotZeroInst(std::vector<std::unique_ptr<Inst>>& insts,
//                                const Ir::JumpIfNotZeroInst* jumpIfNotZero)
// {
//     std::shared_ptr<Operand> condition = operand(jumpIfNotZero->condition);
//     insts.push_back(std::make_unique<CmpInst>(std::make_shared<ImmOperand>(0), condition));
//
//     Identifier target(jumpIfNotZero->target.value);
//     insts.push_back(std::make_unique<JmpCCInst>(BinaryInst::CondCode::NE, target));
// }
//
// void generateCopyInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::CopyInst* type)
// {
//     std::shared_ptr<Operand> src = operand(type->source);
//     std::shared_ptr<Operand> dst = operand(type->destination);
//     insts.push_back(std::make_unique<MoveInst>(src, dst));
// }
//
// void generateLabelInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::LabelInst* irLabel)
// {
//     Identifier label(irLabel->target.value);
//     insts.push_back(std::make_unique<LabelInst>(label));
// }
//
// void unaryInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary)
// {
//     if (irUnary->operation == Ir::UnaryInst::Operation::Not) {
//         generateUnaryNotInst(insts, irUnary);
//         return;
//     }
//     UnaryInst::Operator oper = unaryOperator(irUnary->operation);
//     std::shared_ptr<Operand> src = operand(irUnary->source);
//     std::shared_ptr<Operand> dst = operand(irUnary->destination);
//     insts.push_back(std::make_unique<MoveInst>(src, dst));
//
//     insts.push_back(std::make_unique<UnaryInst>(oper, dst));
// }
//
// void generateUnaryNotInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary)
// {
//     std::shared_ptr<Operand> src = operand(irUnary->source);
//     auto immOperand = std::make_shared<ImmOperand>(0);
//     insts.push_back(std::make_unique<CmpInst>(immOperand, src));
//
//     std::shared_ptr<Operand> dst = operand(irUnary->destination);
//     insts.push_back(std::make_unique<MoveInst>(immOperand, dst));
//
//     insts.push_back(std::make_unique<SetCCInst>(BinaryInst::CondCode::E, dst));
// }
//
// void binaryInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
// {
//     using IrOper = Ir::BinaryInst::Operation;
//     switch (irBinary->operation) {
//         case IrOper::Add:
//         case IrOper::Subtract:
//         case IrOper::Multiply:
//         case IrOper::BitwiseAnd:
//         case IrOper::BitwiseOr:
//         case IrOper::BitwiseXor:
//         case IrOper::LeftShift:
//         case IrOper::RightShift:
//             generateBinaryBasicInst(insts, irBinary);
//             break;
//         case IrOper::Divide:
//             generateBinaryDivideInst(insts, irBinary);
//             break;
//         case IrOper::Remainder:
//             generateBinaryRemainderInst(insts, irBinary);
//             break;
//         case IrOper::Equal:
//         case IrOper::NotEqual:
//         case IrOper::LessThan:
//         case IrOper::LessOrEqual:
//         case IrOper::GreaterThan:
//         case IrOper::GreaterOrEqual:
//             generateBinaryCondInst(insts, irBinary);
//             break;
//         default:
//             throw std::runtime_error("Unsupported binary operation");
//     }
// }
//
// void generateBinaryCondInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
// {
//     std::shared_ptr<Operand> src1 = operand(irBinary->source1);
//     std::shared_ptr<Operand> src2 = operand(irBinary->source2);
//     insts.push_back(std::make_unique<CmpInst>(src2, src1));
//
//     std::shared_ptr<Operand> dst = operand(irBinary->destination);
//     std::shared_ptr<Operand> imm = std::make_shared<ImmOperand>(0);
//     insts.push_back(std::make_unique<MoveInst>(imm, dst));
//
//     BinaryInst::CondCode cc = condCode(irBinary->operation);
//     insts.push_back(std::make_unique<SetCCInst>(cc, dst));
// }
//
// void generateBinaryDivideInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
// {
//     std::shared_ptr<Operand> src1 = operand(irBinary->source1);
//     std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(RegisterOperand::Type::AX, 4);
//     insts.push_back(std::make_unique<MoveInst>(src1, regAX));
//
//     insts.push_back(std::make_unique<CdqInst>());
//
//     std::shared_ptr<Operand> src2 = operand(irBinary->source2);
//     insts.push_back(std::make_unique<IdivInst>(src2));
//
//     std::shared_ptr<Operand> dst = operand(irBinary->destination);
//     insts.push_back(std::make_unique<MoveInst>(regAX, dst));
// }
//
// void generateBinaryRemainderInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
// {
//     std::shared_ptr<Operand> src1 = operand(irBinary->source1);
//     std::shared_ptr<Operand> regAX = std::make_shared<RegisterOperand>(RegisterOperand::Type::AX, 4);
//     insts.push_back(std::make_unique<MoveInst>(src1, regAX));
//
//     insts.push_back(std::make_unique<CdqInst>());
//
//     std::shared_ptr<Operand> src2 = operand(irBinary->source2);
//     insts.push_back(std::make_unique<IdivInst>(src2));
//
//     std::shared_ptr<Operand> dst = operand(irBinary->destination);
//     const auto regDX = std::make_shared<RegisterOperand>(RegisterOperand::Type::DX, 4);
//     insts.push_back(std::make_unique<MoveInst>(regDX, dst));
// }
//
// void generateBinaryBasicInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::BinaryInst* irBinary)
// {
//     std::shared_ptr<Operand> src1 = operand(irBinary->source1);
//     std::shared_ptr<Operand> dst = operand(irBinary->destination);
//     insts.push_back(std::make_unique<MoveInst>(src1, dst));
//
//     BinaryInst::Operator oper = binaryOperator(irBinary->operation);
//     std::shared_ptr<Operand> src2 = operand(irBinary->source2);
//     insts.push_back(std::make_unique<BinaryInst>(oper, src2, dst));
// }
//
// void returnInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::ReturnInst* inst)
// {
//     std::shared_ptr<Operand> val = operand(inst->returnValue);
//     std::shared_ptr<Operand> operandRegister = std::make_shared<RegisterOperand>(RegisterOperand::Type::AX, 4);
//     insts.push_back(std::make_unique<MoveInst>(val, operandRegister));
//
//     insts.push_back(std::make_unique<ReturnInst>());
// }
//
// void generateFunCallInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::FunCallInst* type)
// {
//     const i32 stackPadding = getStackPadding(type->args.size());
//     if (0 < stackPadding)
//         insts.push_back(std::make_unique<AllocStackInst>(8));
//     pushFunCallArgs(insts, type);
//     insts.push_back(std::make_unique<CallInst>(Identifier(type->funName.value)));
//     const i32 bytesToRemove = 8 * (type->args.size() - 6) + stackPadding;
//     if (0 < bytesToRemove)
//         insts.push_back(std::make_unique<DeallocStackInst>(bytesToRemove));
//     std::shared_ptr<Operand> destination = operand(type->destination);
//     insts.push_back(
//         std::make_unique<MoveInst>(std::make_shared<RegisterOperand>(RegisterOperand::Type::AX, 4), destination)
//         );
// }
//
// i32 getStackPadding(const size_t numArgs)
// {
//     if (numArgs <= 6)
//         return 0;
//     i32 stackPadding = 0;
//     if (numArgs % 2 == 1)
//         stackPadding += 8;
//     return stackPadding;
// }
//
// void pushFunCallArgs(std::vector<std::unique_ptr<Inst>>& insts, const Ir::FunCallInst* type)
// {
//     using RegType = RegisterOperand::Type;
//     static const std::vector<RegType> registerTypes = {RegType::DI, RegType::SI, RegType::DX,
//                                                        RegType::CX, RegType::R8, RegType::R9};
//     i32 regIndex = 0;
//     for (; regIndex < type->args.size() && regIndex < registerTypes.size(); ++regIndex) {
//         std::shared_ptr<Operand> src = operand(type->args[regIndex]);
//         insts.push_back(
//             std::make_unique<MoveInst>(src, std::make_unique<RegisterOperand>(registerTypes[regIndex], 4))
//         );
//     }
//     for (i32 i = type->args.size() - 1; regIndex <= i; --i) {
//         std::shared_ptr<Operand> src = operand(type->args[i]);
//         if (src->kind == Operand::Kind::Imm ||
//             src->kind == Operand::Kind::Register) {
//             insts.push_back(std::make_unique<PushInst>(src));
//         }
//         else {
//             insts.push_back(
//                 std::make_unique<MoveInst>(src, std::make_shared<RegisterOperand>(RegisterOperand::Type::AX, 4))
//             );
//             insts.push_back(
//                 std::make_unique<PushInst>(std::make_shared<RegisterOperand>(RegisterOperand::Type::AX, 8))
//             );
//         }
//     }
// }
//
// UnaryInst::Operator unaryOperator(const Ir::UnaryInst::Operation type)
// {
//     using IrOper = Ir::UnaryInst::Operation;
//     switch (type)
//     {
//         case IrOper::Complement:        return UnaryInst::Operator::Not;
//         case IrOper::Negate:            return UnaryInst::Operator::Neg;
//         default:
//             throw std::invalid_argument("Invalid UnaryOperator type");
//     }
// }
//
// BinaryInst::Operator binaryOperator(const Ir::BinaryInst::Operation type)
// {
//     using IrOper = Ir::BinaryInst::Operation;
//     using AsmOper = BinaryInst::Operator;
//     switch (type) {
//         case IrOper::Add:          return AsmOper::Add;
//         case IrOper::Subtract:     return AsmOper::Sub;
//         case IrOper::Multiply:     return AsmOper::Mul;
//
//         case IrOper::BitwiseAnd:   return AsmOper::BitwiseAnd;
//         case IrOper::BitwiseOr:    return AsmOper::BitwiseOr;
//         case IrOper::BitwiseXor:   return AsmOper::BitwiseXor;
//
//         case IrOper::LeftShift:    return AsmOper::LeftShift;
//         case IrOper::RightShift:   return AsmOper::RightShift;
//         default:
//             throw std::invalid_argument("Invalid BinaryOperation type: " + std::to_string(static_cast<int>(type)));
//     }
// }
//
// BinaryInst::CondCode condCode(const Ir::BinaryInst::Operation type)
// {
//     using IrOper = Ir::BinaryInst::Operation;
//     using BinCond = BinaryInst::CondCode;
//     switch (type) {
//         case IrOper::Equal:             return BinCond::E;
//         case IrOper::NotEqual:          return BinCond::NE;
//         case IrOper::LessThan:          return BinCond::L;
//         case IrOper::LessOrEqual:       return BinCond::LE;
//         case IrOper::GreaterThan:       return BinCond::G;
//         case IrOper::GreaterOrEqual:    return BinCond::GE;
//         default:
//             throw std::invalid_argument("Invalid BinaryOperation type");
//     }
// }
//
// std::shared_ptr<Operand> operand(const std::shared_ptr<Ir::Value>& value)
// {
//     switch (value->kind) {
//         case Ir::Value::Kind::Constant: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto valueConst = static_cast<Ir::ValueConst*>(value.get());
//             return std::make_shared<ImmOperand>(valueConst->value);
//         }
//         case Ir::Value::Kind::Variable: {
//             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
//             const auto valueConst = static_cast<Ir::ValueVar*>(value.get());
//             return std::make_shared<PseudoOperand>(valueConst->value.value);
//         }
//         default:
//             throw std::invalid_argument("Invalid UnaryOperator type");
//     }
// }
//
// i32 getStaticVariableInitial(const Ir::StaticVariable& staticVariable)
// {
//     assert(staticVariable.value->kind == Ir::Value::Kind::Constant &&
//             "StaticVariable must be const initialized");
//     const auto valueConst = static_cast<Ir::ValueConst*>(staticVariable.value.get());
//     return valueConst->value;
// }
//
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