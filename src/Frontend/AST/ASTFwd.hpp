#pragma once

namespace Parsing {

// for Levelisation

/*
    program = Program(declaration*)
    declaration = FunDecl(function_declaration)
                | VarDecl(variable_declaration)
                | StructDecl(struct_declaration)
                | UnionDecl(union_declaration)

    variable_declaration = (identifier name, exp? init, type var_type, storage_class?)
    function_declaration = (identifier name, identifier* params, block? body, type var_type, storage_class?)
    struct_declaration = (identifier tag, member_declaration* members)
    union_declaration = (identifier tag, member_declaration* members)
    member_declaration = (identifier member_name, type member_type)

    type = Int | Long | UInt | ULong | Double | Char | SChar | UChar | Void
         | FunType(type* params, type ret)
         | Pointer(type referenced)
         | Array(type element, int size)
         | StructDecl(identifier tag)
    storage_class = Static | Extern
    block = Block(block_item)
    block_item = S(statement) | D(declaration)
    for_init = InitDecl(variable_declaration) | InitExp(exp?)
    statement = Return(exp?)
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
              | FunctionCall(identifier, exp* args)
              | Null
    exp = Constant(const)
        | String(string)
        | Var(identifier)
        | Cast(type target, exp)
        | Unary(unary_operator, exp)
        | Binary(binary_operator, exp, exp)
        | Assignment(assign_operator, exp, exp)
        | Conditional(exp condition, exp, exp)
        | FunctionCall(identifier, exp* args)
        | Dereference(exp)
        | AddrOf(exp)
        | Subscript(exp, exp)
        | SizeOf(exp)
        | SizeOf(type)
        | Dot(exp structure, identifier member)
        | Arrow(exp pointer, identifier member)
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
struct FuncDecl;
struct VarDecl;
struct StructDecl;
struct UnionDecl;
struct MemberDecl;

struct Block;

// Block items
struct BlockItem;
struct StmtBlockItem;
struct DeclBlockItem;

// Types
struct TypeBase;
struct VarType;
struct FuncType;
struct PointerType;
struct ArrayType;
struct StructType;
struct UnionType;

// Initializer
struct Initializer;
struct SingleInitializer;
struct CompoundInitializer;
struct ZeroInitializer;
struct StringInitializer;

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
struct StringExpr;
struct VarExpr;
struct CastExpr;
struct UnaryExpr;
struct BinaryExpr;
struct AssignmentExpr;
struct TernaryExpr;
struct FuncCallExpr;
struct DereferenceExpr;
struct AddrOffExpr;
struct SubscriptExpr;
struct SizeOfExprExpr;
struct SizeOfTypeExpr;
struct DotExpr;
struct ArrowExpr;
} // Parsing