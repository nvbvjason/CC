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

i32 Assembly::getOutput(std::string &output) const
{
    output = getFunction(c_program->function);
    return 0;
}

std::string Assembly::getFunction(const std::unique_ptr<Parsing::Function> &functionNode)
{
    std::string result = "    .globl " + functionNode->name + '\n';
    result += functionNode->name + ":\n";
    result += getInstruction(functionNode->body);
    result += "    ret    \n";
    result += "    .section .note.GNU-stack,\"\",@progbits\n";
    return result;
}

std::string Assembly::getInstruction(const std::unique_ptr<Parsing::Statement>& returnNode)
{
    return std::format("    mov    ${}, %eax\n", getOperand(returnNode->expression));
}

std::string Assembly::getOperand(const std::unique_ptr<Parsing::Expression>& constantNode)
{
    return std::to_string(std::get<i32>(constantNode->value));
}
}
