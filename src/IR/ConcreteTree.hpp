#pragma once

#ifndef CC_IR_CONCRETE_TREE_HPP
#define CC_IR_CONCRETE_TREE_HPP

#include "AbstractTree.hpp"
#include "Parsing/AbstractTree.hpp"

/*

emit_tacky(e, instructions)c
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

namespace Ir {

void program(const Parsing::Program* parsingProgram, Program& tackyProgram);
std::unique_ptr<Function> function(const Parsing::Function* parsingFunction);

Value instruction(const Parsing::Expr* parsingExpr,
                  std::vector<Instruction>& instructions);
Value unaryInstruction(const Parsing::Expr *parsingExpr, std::vector<Instruction> &instructions);
Value returnInstruction(const Parsing::Expr *parsingExpr);


} // IR

#endif // CC_IR_CONCRETE_TREE_HPP