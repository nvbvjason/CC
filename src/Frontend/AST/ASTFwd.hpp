#pragma once

#ifndef CC_PARSING_AST_FWD_HPP
#define CC_PARSING_AST_FWD_HPP

namespace Parsing {

// for Levelisation

/*
    program = Program(function_definition)
    function_definition = Function(identifier names, block body)
    block = Block(block_item)
    block_item = S(statement) | D(declaration)
    declaration = Declaration(identifier name, exp? init)
    for_init = InitDecl(declaration) | Init(Expr)
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
    exp = Constant(int)
        | Var(identifier)
        | Unary(unary_operator, exp)
        | Binary(binary_operator, exp, exp)
        | Assignment(assign_operator, exp, exp)
        | Conditional(exp condition, exp, exp)
    unary_operator = Complement | Negate | Not
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
*/

// Program structure
struct Program;
struct Function;
struct Block;

// Block items
struct BlockItem;
struct StmtBlockItem;
struct DeclBlockItem;

// Declaration
struct Declaration;

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
struct UnaryExpr;
struct BinaryExpr;
struct AssignmentExpr;
struct ConditionalExpr;

} // Parsing

#endif // CC_PARSING_AST_FWD_HPP
