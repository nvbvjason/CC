#include "Assembly.hpp"

#include <fstream>

namespace Codegen {

struct Assembly::OperandNode {
    struct Imm {
        i32 value;
    };
    struct Register {
        std::string regis;
    };
    std::variant<Imm, Register> imm;
};

struct Assembly::InstructionNode {
    struct Move {
        std::string src;
        std::string dst;
    };
    struct Return {
    };
    std::variant<Move, Return> operand;
};

struct Assembly::FunctionNode {
    std::string name;
    std::vector<InstructionNode> constants;
};

struct Assembly::ProgramNode {
    FunctionNode function;
};

void Assembly::getOutput(std::string &output) const
{
    output = getFunction(c_program.function);
}

std::string Assembly::getFunction(const Parsing::FunctionNode& functionNode)
{
    std::string result = "    .globl " + functionNode.name + '\n';
    result += functionNode.name + ":\n";
    result += getInstruction(functionNode.body);
    result += "    ret    \n";
    // result += "    .section .note.GNU-stack,\"\",@progbits\n";
    return result;
}

std::string Assembly::getInstruction(const Parsing::ReturnNode& returnNode)
{
    return std::format("    mov    ${}, %eax\n", getOperand(returnNode.expression));
}

std::string Assembly::getOperand(const Parsing::ConstantNode& constantNode)
{
    return std::to_string(constantNode.constant);
}
}
