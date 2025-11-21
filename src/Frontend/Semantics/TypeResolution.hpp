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
    using Storage = Parsing::Declaration::StorageClass;
    const VarTable& varTable;
    TypeResolutionExpr m_resolveExpr;

    i64 location;

    std::vector<Error> m_errors;
    bool m_isConst = true;
    bool m_global = true;
    bool m_inArrayInit = false;
public:
    explicit TypeResolution(const VarTable& varTable)
        : varTable(varTable), m_resolveExpr(m_errors, varTable, m_functions) {}

    std::vector<Error> validate(Parsing::Program& program);
    void validateCompleteTypesFunc(const Parsing::FuncDecl& funDecl, const Parsing::FuncType& funcType);


    void visit(Parsing::FuncDecl& funDecl) override;
    void visit(Parsing::VarDecl& varDecl) override;
    bool isIllegalVarDecl(const Parsing::VarDecl& varDecl);
    void visit(Parsing::DeclForInit& declForInit) override;
    void visit(Parsing::ExprForInit& exprForInit) override;

    void visit(Parsing::SingleInitializer& singleInitializer) override;
    void visit(Parsing::CompoundInitializer& compoundInitializer) override;

    void initDecl(Parsing::VarDecl& varDecl);
    void initArrayWithCompound(const Parsing::TypeBase* type,
                               Parsing::Initializer* init,
                               std::vector<std::unique_ptr<Parsing::Initializer>>& newInit);
    void walkInit(const Parsing::TypeBase* type,
                  Parsing::Initializer* init,
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

    static void assignTypeToArithmeticUnaryExpr(Parsing::VarDecl& varDecl);
    [[nodiscard]] bool incompatibleFunctionDeclarations(const FuncEntry& funcEntry, const Parsing::FuncDecl& funDecl);
    void handelCompoundInit(const Parsing::VarDecl& varDecl);
    void verifyArrayInSingleInit(const Parsing::VarDecl& varDecl, const Parsing::SingleInitializer& singleInit);
    void handleCompoundInitArray(const Parsing::VarDecl& varDecl, const Parsing::CompoundInitializer* compoundInit);
    static bool hasStorageClassSpecifier(const Parsing::DeclForInit& declForInit);
private:
    void addError(const std::string& error, const i64 location) { m_errors.emplace_back(error, location); }
    [[nodiscard]] bool hasError() const { return !m_errors.empty(); }
};

inline bool TypeResolution::hasStorageClassSpecifier(const Parsing::DeclForInit& declForInit)
{
    return declForInit.decl->storage != Storage::None;
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