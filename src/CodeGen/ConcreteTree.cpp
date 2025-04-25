#include "ConcreteTree.hpp"
#include "IR/ConcreteTree.hpp"

#include "AbstractTree.hpp"

#include <stdexcept>

namespace CodeGen {

void program(const Ir::Program &program, Program &programCodegen)
{
    programCodegen.function = function(program.function.get());
}

std::unique_ptr<Function> function(const Ir::Function *function)
{
    auto functionCodeGen = std::make_unique<Function>();
    functionCodeGen->name = function->identifier;
    for (const std::unique_ptr<Ir::Instruction>& inst : function->instructions) {
        switch (inst->type) {
            case Ir::Instruction::Type::Unary: {
                const auto irUnary = dynamic_cast<Ir::UnaryInst*>(inst.get());
                unaryInst(functionCodeGen->instructions, irUnary);
                break;
            }
            case Ir::Instruction::Type::Return: {
                const auto irReturn = dynamic_cast<Ir::ReturnInst*>(inst.get());
                returnInst(functionCodeGen->instructions, irReturn);
                break;
            }
            default:
                throw std::runtime_error("Unsupported instruction type");
        }
    }
    return functionCodeGen;
}

void unaryInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::UnaryInst* irUnary)
{
    std::unique_ptr<Operand> src = operand(irUnary->source);
    std::unique_ptr<Operand> dst = operand(irUnary->destination);
    auto moveInst = std::make_unique<MoveInst>(std::move(src), std::move(dst));
    insts.push_back(std::move(moveInst));

    UnaryInst::Operator oper = unaryOperator(irUnary->operation);
    auto unaryInst = std::make_unique<UnaryInst>(oper, std::move(dst));
    insts.push_back(std::move(unaryInst));
}

void returnInst(std::vector<std::unique_ptr<Inst>>& insts, const Ir::ReturnInst* inst)
{
    std::unique_ptr<Operand> val = operand(inst->returnValue);
    std::unique_ptr<Operand> operandRegister = std::make_unique<OperandRegister>(OperandRegister::Kind::R10);
    auto moveInst = std::make_unique<MoveInst>(std::move(val), std::move(operandRegister));
    insts.push_back(std::move(moveInst));
    auto instRet = std::make_unique<InstRet>();
    insts.push_back(std::move(instRet));
}

UnaryInst::Operator unaryOperator(const Ir::UnaryInst::Operation type)
{
    switch (type)
    {
        case Ir::UnaryInst::Operation::Complement:
            return UnaryInst::Operator::Not;
        case Ir::UnaryInst::Operation::Negate:
            return UnaryInst::Operator::Neg;
        case Ir::UnaryInst::Operation::Invalid:
            return UnaryInst::Operator::Invalid;
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}

std::unique_ptr<Operand> operand(const std::unique_ptr<Ir::Value>& value)
{
    switch (value->type) {
        case Ir::Value::Type::Constant: {
            const auto valueConst = dynamic_cast<Ir::ValueConst*>(value.get());
            return std::make_unique<OperandImm>(valueConst->value);
        }
        case Ir::Value::Type::Variable: {
            const auto valueConst = dynamic_cast<Ir::ValueVar*>(value.get());
            return std::make_unique<OperandPseudo>(valueConst->value);
        }
        default:
            throw std::invalid_argument("Invalid UnaryOperator type");
    }
}



}