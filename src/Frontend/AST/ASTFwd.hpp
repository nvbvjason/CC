#pragma once

#ifndef CC_PARSING_AST_FWD_HPP
#define CC_PARSING_AST_FWD_HPP

namespace Parsing {

// for Levelisation

// Program structure
struct Program;
struct Function;

// Block items
struct BlockItem;
struct StmtBlockItem;
struct DeclBlockItem;

// Declaration
struct Declaration;

// Statements
struct Stmt;
struct IfStmt;
struct ReturnStmt;
struct ExprStmt;
struct NullStmt;

// Expressions
struct Expr;
struct ConstExpr;
struct VarExpr;
struct UnaryExpr;
struct BinaryExpr;
struct AssignmentExpr;

} // Parsing

#endif // CC_PARSING_AST_FWD_HPP
