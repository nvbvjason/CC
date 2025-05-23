#include "ASTParser.hpp"

namespace Parsing {
Program::~Program() = default;

VarDecl::~VarDecl() = default;
FunDecl::~FunDecl() = default;

Block::~Block() = default;

StmtBlockItem::~StmtBlockItem() = default;
DeclBlockItem::~DeclBlockItem() = default;

DeclForInit::~DeclForInit() = default;
ExprForInit::~ExprForInit() = default;

ReturnStmt::~ReturnStmt() = default;
ExprStmt::~ExprStmt() = default;
IfStmt::~IfStmt() = default;
GotoStmt::~GotoStmt() = default;
CompoundStmt::~CompoundStmt() = default;
BreakStmt::~BreakStmt() = default;
ContinueStmt::~ContinueStmt() = default;
LabelStmt::~LabelStmt() = default;
CaseStmt::~CaseStmt() = default;
DefaultStmt::~DefaultStmt() = default;
WhileStmt::~WhileStmt() = default;
DoWhileStmt::~DoWhileStmt() = default;
ForStmt::~ForStmt() = default;
SwitchStmt::~SwitchStmt() = default;

ConstExpr::~ConstExpr() = default;
VarExpr::~VarExpr() = default;
UnaryExpr::~UnaryExpr() = default;
BinaryExpr::~BinaryExpr() = default;
AssignmentExpr::~AssignmentExpr() = default;
ConditionalExpr::~ConditionalExpr() = default;
FunCallExpr::~FunCallExpr() = default;

} // Parsing