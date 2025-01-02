#include "Assembly.hpp"

#include <fstream>
#include <iostream>

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

void Assembly::writeToFile(const std::string &fileName) const
{
    const std::string contents = getFunction(c_program.function);
    std::ofstream file(fileName);
    // std::cout << contents;
    file << contents;
    file.close();
}

std::string Assembly::getFunction(const Parsing::FunctionNode& functionNode)
{
    std::string result = functionNode.name + ":\n";
    result += getInstruction(functionNode.body);
    result += "\tret\t\n";
    result += "\t.section .note.GNU-stack,\"\",@progbits\n";
    return result;
}

std::string Assembly::getInstruction(const Parsing::ReturnNode& returnNode)
{
    return std::format("\tmov\teax, {}\n", getOperand(returnNode.expression));
}

std::string Assembly::getOperand(const Parsing::ConstantNode& constantNode)
{
    return std::to_string(constantNode.constant);
}
}
