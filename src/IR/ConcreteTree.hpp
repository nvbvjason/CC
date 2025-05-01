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
std::shared_ptr<Function> function(const Parsing::Function* parsingFunction);
std::shared_ptr<Value> inst(const Parsing::Expr* parsingExpr,
                            std::vector<std::shared_ptr<Instruction>>& instructions);
std::shared_ptr<ValueVar> unaryInst(const Parsing::Expr *parsingExpr,
                                    std::vector<std::shared_ptr<Instruction>>& instructions);
std::shared_ptr<Value> binaryInst(const Parsing::Expr *parsingExpr,
                                  std::vector<std::shared_ptr<Instruction>>& instructions);
std::shared_ptr<Value> binaryAndInst(const Parsing::BinaryExpr *parsingExpr,
                                     std::vector<std::shared_ptr<Instruction>>& instructions);
std::shared_ptr<Value> binaryOrInst(const Parsing::BinaryExpr* binaryExpr,
                                    std::vector<std::shared_ptr<Instruction>>& instructions);
std::shared_ptr<ValueConst> returnInst(const Parsing::Expr *parsingExpr);

} // IR

#endif // CC_IR_CONCRETE_TREE_HPP