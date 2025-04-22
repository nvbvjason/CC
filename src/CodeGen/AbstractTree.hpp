#pragma once

#ifndef CC_ABSTRACTTREE_HPP
#define CC_ABSTRACTTREE_HPP

#include "ShortTypes.hpp"

#include <memory>
#include <vector>

/*

program = Program(function_definition)
function_definition = Function(identifier name, instruciton* instructions)
instruction = Mov(operand src, operand dst)
            | Unary(unary_operator, operand)
            | Allocate_stack(int)
            | Ret
unary_operator = Neg | Not
operand = Imm(int)
        | Reg(reg)
        | Pseudo(identifier)
        | Stack(int)
reg = AX | R10

*/

namespace CodeGen {

struct Program;
struct Function;
struct Instruction;
enum class UnaryOperator;
struct Operand;
enum class Register;

struct Program {
    std::unique_ptr<Function> function;
};

struct Function {
    std::string name;
    std::vector<Instruction> instructions;
};

enum class OperandType {
    Imm, Register, Pseudo, Stack,

    Invalid
};

struct Operand {
    OperandType type;
    std::variant<i32, Register, std::string> value;
};

struct Mov {
    Operand src;
    Operand dst;
};

struct Unary {
    UnaryOperator oper;
    Operand operand;
};

struct Return {

};

enum class InstructionType {
    Mov, Unary, AllocateStack, Ret,

    Invalid
};

struct Instruction {
    InstructionType type = InstructionType::Invalid;
    std::variant<Mov, Unary, i32, Return> value;
};

enum class UnaryOperator {
    Neg, Not,

    Invalid
};

enum class Register {
    AX, R10,

    Invalid
};

}

#endif // CC_ABSTRACTTREE_HPP
