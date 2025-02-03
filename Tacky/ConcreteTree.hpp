#pragma once

#ifndef CC_TACKY_CONCRETE_TREE_HPP
#define CC_TACKY_CONCRETE_TREE_HPP

#include "AbstractTree.hpp"
#include "../Parsing/AbstractTree.hpp"

/*

emit_tacky(e, instructions)
    match e with
    | Constant(c) ->
        return Constant(c)
    | Unary(op, inner)
        src = emit_tacky(inner, instructions)
        dst_name = make_temporary()
        dst = Var(dst_name)
        tacky_op = convert_unop(op)
        instructions.append(Unary(tacky_op, src, dst))
        return dst

*/

namespace Tacky {

void programTacky(const Parsing::ProgramNode* parsingProgram, ProgramNode& tackyProgram);
FunctionNode* functionTacky(const Parsing::FunctionNode* parsingFunction);
ValueNode* instructionTacky(const Parsing::ExpressionNode* parsingExpression, std::vector<InstructionNode>& instructions);
} // Tacky


#endif // CC_TACKY_CONCRETE_TREE_HPP
