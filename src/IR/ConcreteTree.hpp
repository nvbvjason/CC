#pragma once

#ifndef CC_IR_CONCRETE_TREE_HPP
#define CC_IR_CONCRETE_TREE_HPP

#include "AbstractTree.hpp"
#include "Parsing/AbstractTree.hpp"

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

namespace IR {

void programTacky(const Parsing::Program* parsingProgram, Program& tackyProgram);
std::unique_ptr<Function> functionTacky(const Parsing::Function* parsingFunction);
std::unique_ptr<Value> instructionTacky(const Parsing::Expression* parsingExpression,
                                        std::vector<Instruction>& instructions);
} // IR

#endif // CC_IR_CONCRETE_TREE_HPP