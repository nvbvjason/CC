#pragma once

#ifndef CC_PARSING_AST_FWD_HPP
#define CC_PARSING_AST_FWD_HPP

namespace Parsing {

// for Levelisation

/*
    program = Program(declaration*)
    declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
    variable_declaration = (identifier name, exp? init,
                            type var_type, storage_class?)
    function_definition = (identifier name, identifier* params, block? body,
                           type var_type, storage_class?)
    type = Int | Long | FunType(type* params, type ret)
    storage_class = Static | Extern
    block = Block(block_item)
    block_item = S(statement) | D(declaration)
    for_init = InitDecl(variable_declaration) | InitExp(exp?)
    statement = Return(exp)
              | Expression(exp)
              | If(exp condition, statement then, statement? else)
              | Goto(identifier label)
              | Compound(block)
              | Break(identifier label)
              | Continue(identifier label)
              | Label(identifier label)
              | Case(identifier label, exp condition, statement body)
              | Default(identifier label, statement body)
              | While(exp condition, statement body, identifier label)
              | DoWhile(statement body, exp condition, identifier label)
              | For(for_init init, exp? condition, exp? post, statement body, identifier label)
              | Switch(identifier, exp condition, statement body, expr cases*, bool hasDefault)
              | Null
    exp = Constant(const)
        | Var(identifier)
        | Cast(type target, exp)
        | Unary(unary_operator, exp)
        | Binary(binary_operator, exp, exp)
        | Assignment(assign_operator, exp, exp)
        | Conditional(exp condition, exp, exp)
        | FunctionCall(identifier, exp* args)
    unary_operator = Complement | Negate | Not | Plus
                   | PrefixIncrement | PostFixIncrement
                   | PrefixDecrement | PostFixDecrement
    binary_operator = Add | Subtract | Multiply | Divide | Remainder
                    | BitwiseAnd | BitwiseOr | BitwiseXor
                    | LeftShift | RightShift
                    | And | Or
                    | Equal | NotEqual
                    | Less | LessThan | LessOrEqual
                    | Greater | GreaterThan | GreaterOrEqual
    assign_operator = Assign | PlusAssign | MinusAssign
                    | MultiplyAssign | DivideAssign | ModuloAssign
                    | BitwiseAndAssign | BitwiseOrAssign | BitwiseXorAssign
                    | LeftShiftAssign | RightShiftAssign
    const = ConstInt(int) | ConstLong(int)
*/

// Program structure
struct Program;

// Declaration
struct Declaration;
struct FunDecl;
struct VarDecl;

struct Block;

// Block items
struct BlockItem;
struct StmtBlockItem;
struct DeclBlockItem;

// Types
struct Type;
struct VarType;
struct FuncType;

// For loop initialization
struct ForInit;
struct DeclForInit;
struct ExprForInit;

// Statements
struct Stmt;
struct ReturnStmt;
struct ExprStmt;
struct IfStmt;
struct GotoStmt;
struct CompoundStmt;
struct BreakStmt;
struct ContinueStmt;
struct LabelStmt;
struct CaseStmt;
struct DefaultStmt;
struct WhileStmt;
struct DoWhileStmt;
struct ForStmt;
struct SwitchStmt;
struct NullStmt;

// Expressions
struct Expr;
struct ConstExpr;
struct VarExpr;
struct CastExpr;
struct UnaryExpr;
struct BinaryExpr;
struct AssignmentExpr;
struct TernaryExpr;
struct FunCallExpr;
} // Parsing

#endif // CC_PARSING_AST_FWD_HPP
