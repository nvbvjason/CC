#pragma once

#ifndef CC_SEMANTICS_VARIABLE_SOLUTION_HPP
#define CC_SEMANTICS_VARIABLE_SOLUTION_HPP

#include "ASTParser.hpp"
#include "ASTTraverser.hpp"
#include "ScopeStack.hpp"

#include <string>

#include "ASTParser.hpp"

namespace Semantics {
struct FunctionClarification {
    std::vector<std::string> args;
    Parsing::Declaration::StorageClass storage;
    bool defined;
    FunctionClarification(const std::vector<std::string>& _args, const bool _defined,
                          const Parsing::Declaration::StorageClass _storage)
        : args(_args), storage(_storage), defined(_defined) {}
};

class VariableResolution : public Parsing::ASTTraverser {
    struct ScopeRAII {
        ScopeStack& stack;
        explicit ScopeRAII(ScopeStack& s) : stack(s) { stack.push(); }
        ~ScopeRAII() { stack.pop(); }
    };
    using StorageClass = Parsing::Declaration::StorageClass;
    ScopeStack m_scopeStack;
    std::unordered_map<std::string, FunctionClarification> m_functions;
    i32 m_counter = 0;
    bool m_valid = true;
public:
    VariableResolution() = default;
    [[nodiscard]] bool resolve(Parsing::Program& program);
    void visit(Parsing::Program& program) override;
    void visit(Parsing::FunDecl& funDecl) override;
    void visit(Parsing::Block& block) override;

    void visit(Parsing::ForStmt& forStmt) override;

    void visit(Parsing::VarDecl& varDecl) override;
    void visit(Parsing::VarExpr& varExpr) override;
    void visit(Parsing::AssignmentExpr& assignmentExpr) override;
    void visit(Parsing::FunCallExpr& funCallExpr) override;

private:
    bool inFunction() const { return 1 < m_scopeStack.size();}
    inline std::string makeStatic(const std::string& name);
    inline std::string makeTemporary(const std::string& name);
    inline std::string makeStaticTemporary(const std::string& name);
};

bool isInvalidInFunctionBody(const Parsing::FunDecl& function, bool inFunction);
bool hasDuplicateParameters(const std::vector<std::string>& names);
bool allowedVarDecl(const ScopeStack& variableStack, const Parsing::VarDecl& varDecl, bool inFunction);
bool allowedVarDeclGlobal(const ScopeStack& variableStack, const Parsing::VarDecl& varDecl);
bool notDeclared(const ScopeStack& variableStack, const Parsing::VarExpr& varExpr);
bool matchesExistingDeclaration(const Parsing::FunDecl& funDecl,
                                const std::unordered_map<std::string, FunctionClarification>& functions);
bool validateFunctionDeclaration(const Parsing::FunDecl& funDecl,
                                const std::unordered_map<std::string, FunctionClarification>& functions,
                                const ScopeStack& variableStack,
                                bool inFunction);
bool conflictingIdenInScope(const Parsing::FunDecl& funDecl, const ScopeStack& scopeStack);
bool conflictingIdenGlobal(const Parsing::FunDecl& funDecl, const ScopeStack& scopeStack);
bool allowedInnermost(const ScopeStack& scopeStack,
                      const Parsing::VarDecl& varDecl);

bool isFunctionCallValid(const std::unordered_map<std::string, FunctionClarification>& functions,
                        const ScopeStack& scopeStack,
                        const Parsing::FunCallExpr& funCallExpr);
bool functionNotFound(
    const std::unordered_map<std::string, FunctionClarification>& functions,
    const std::unordered_map<std::string, FunctionClarification>::const_iterator& functionsIt);

bool shouldSkipVarDecl(const Parsing::VarDecl& varDecl, const ScopeStack& variableStack, bool inFunction);
bool shouldSkipFunDecl(const Parsing::FunDecl& funDecl, const ScopeStack& variableStack, bool inFunction);

bool internalLinkage(Parsing::Declaration::StorageClass storageClass);

inline std::string VariableResolution::makeStatic(const std::string& name)
{
    return name + ".s.";
}

inline std::string VariableResolution::makeStaticTemporary(const std::string& name)
{
    return name + ".s." + std::to_string(m_counter++);
}

inline std::string VariableResolution::makeTemporary(const std::string& name)
{
    return name + '.' + std::to_string(m_counter++);
}

} // Semantics

#endif // CC_SEMANTICS_VARIABLE_SOLUTION_HPP