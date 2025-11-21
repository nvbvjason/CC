#pragma once

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"
#include "FuncEntry.hpp"
#include "Error.hpp"
#include "VarTable.hpp"
#include "TypeResolutionExpr.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Semantics {



class TypeResolution final : public Parsing::ASTTraverser {
    std::unordered_map<std::string, FuncEntry> m_functions;
    std::unordered_set<std::string> m_definedFunctions;
    std::unordered_set<std::string> m_localExternVars;
    std::unordered_set<std::string> m_globalStaticVars;
    const VarTable& varTable;
    TypeResolutionExpr m_resolveExpr;

    i64 location = 0;

    std::vector<Error> m_errors;
    bool m_global = true;
public:
    explicit TypeResolution(const VarTable& varTable)
        : varTable(varTable), m_resolveExpr(m_errors, varTable, m_functions) {}

    std::vector<Error> validate(Parsing::Program& program);

    void visit(Parsing::FuncDecl& funDecl) override;
    void visit(Parsing::VarDecl& varDecl) override;
    bool isIllegalVarDecl(const Parsing::VarDecl& varDecl);
    void visit(Parsing::DeclForInit& declForInit) override;
    void visit(Parsing::ExprForInit& exprForInit) override;

    void initDecl(Parsing::VarDecl& varDecl);
    void walkInit(const Parsing::TypeBase* type,
                  Parsing::Initializer* init,
                  std::vector<std::unique_ptr<Parsing::Initializer>>& newInit);
    void initArrayWithCompound(const Parsing::ArrayType& type,
                               Parsing::CompoundInitializer& compoundInit,
                               std::vector<std::unique_ptr<Parsing::Initializer>>& newInit);
    void initArrayWithSingle(const Parsing::ArrayType& type,
                             Parsing::SingleInitializer& compoundInit,
                             std::vector<std::unique_ptr<Parsing::Initializer>>& newInit);
    void initVarWithSingle(const Parsing::TypeBase* type,
                           Parsing::Initializer* init,
                           std::vector<std::unique_ptr<Parsing::Initializer>>& newInit);

    void visit(Parsing::ReturnStmt& stmt) override;
    void visit(Parsing::ExprStmt& stmt) override;
    void visit(Parsing::IfStmt& ifStmt) override;
    void visit(Parsing::CaseStmt& caseStmt) override;
    void visit(Parsing::WhileStmt& whileStmt) override;
    void visit(Parsing::DoWhileStmt& doWhileStmt) override;
    void visit(Parsing::ForStmt& forStmt) override;
    void visit(Parsing::SwitchStmt& switchStmt) override;

    void validateCompleteTypesFunc(const Parsing::FuncDecl& funDecl, const Parsing::FuncType& funcType);
    [[nodiscard]] bool incompatibleFunctionDeclarations(
        const FuncEntry& funcEntry,
        const Parsing::FuncDecl& funDecl);
private:
    void addError(const std::string& error, const i64 location) { m_errors.emplace_back(error, location); }
    [[nodiscard]] bool hasError() const { return !m_errors.empty(); }
};

inline bool hasStorageClassSpecifier(const Parsing::DeclForInit& declForInit)
{
    return declForInit.decl->storage != Parsing::Declaration::StorageClass::None;
}

inline bool illegalNonConstInitialization(const Parsing::VarDecl& varDecl,
                                          const bool isConst,
                                          const bool global)
{
    return !isConst && (global || varDecl.storage ==  Parsing::Declaration::StorageClass::Static);
}

void emplaceZeroInit(std::vector<std::unique_ptr<Parsing::Initializer>>& newInit, i64 lengthZero);
bool isZeroSingleInit(const Parsing::Initializer& init);
} // Semantics