#include "Assembly.hpp"

namespace Codegen {

// struct Assembly::OperandNode {
//     struct Imm {
//         i32 value;
//     };
//     struct Register {
//         std::string regis;
//     };
//     std::variant<Imm, Register> imm;
// };
//
// struct Assembly::InstructionNode {
//     struct Move {
//         std::string src;
//         std::string dst;
//     };
//     struct Return {
//     };
//     std::variant<Move, Return> operand;
// };
//
// struct Assembly::FunctionNode {
//     std::string name;
//     std::vector<InstructionNode> constants;
// };
//
// struct Assembly::ProgramNode {
//     FunctionNode function;
// };
//
// void Assembly::writeToFile(const std::string &fileName) const
// {
//     std::string filename = fileName + ".asm";
//     std::string contents = getFunction(c_program.function);
// }
//
// std::string Assembly::getFunction(const Parsing::FunctionNode& functionNode) const
// {
//     std::string result = functionNode.name;
//     // result +=
//     return result;
// }
//
// std::string Assembly::getInstruction() const
// {
//     return "";
// }
//
// std::string Assembly::getOperand() const
// {
//     return "";
// }
}
