#pragma once

#ifndef CC_PARSING_AST_FWD_HPP
#define CC_PARSING_AST_FWD_HPP

namespace Parsing {

// for Levelisation

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
struct WhileStmt;
struct DoWhileStmt;
struct ForStmt;
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
