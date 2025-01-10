#pragma once

#ifndef ABSTRACTTREE_HPP
#define ABSTRACTTREE_HPP

#include "../AbbreviationsOfTypes.hpp"

#include <string>
#include <variant>
#include <vector>

/*
    program = Program(function_definition)
    function_definition = Function(identifier, instruction* body)
    instruction = Return(val) | Unary(unary_operator, val src, val dst)
    val = Constant(int) | Var(identifier)
    unary_operator = Complement | Negate
*/

namespace Tacky {
struct ProgramNode;
struct FunctionNode;
struct InstructionNode;
struct ValueNode;
struct UnaryNode;

struct ProgramNode {
    FunctionNode* function;
};

struct FunctionNode {
    std::string identifier;
    std::vector<ValueNode*> instructions;
};

enum class InstructionType {
    Return, Unary,
    Invalid
};

struct InstructionNode {
    InstructionType type;
    std::variant<ValueNode*, UnaryNode*> value;
};

enum class UnaryOperationType {
    Complement, Negate,
    Invalid
};

struct UnaryNode {
    UnaryOperationType type;
    ValueNode* source;
    ValueNode* destination;
};

struct ValueNode {
    std::variant<i32, std::string> value;
};

/*
    emit_tacky(e, instructions)
        match e with
        | Constant(c) ->
            return Constant(c)
        | Unary(op, inner)
            src =   emit_tacky(inner, instructions)
            dst_name = make_temporary()
            dst = Var(dst_name)
            tacky_op = convert_unop(op)
            instructions.append(Unary(tacky_op, src, dst))
            return dst
*/


} // Tacky

#endif //ABSTRACTTREE_HPP
