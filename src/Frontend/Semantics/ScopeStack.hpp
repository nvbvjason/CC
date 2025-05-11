#pragma once

#ifndef CC_SEMANTICS_VARIABLE_STACK_HPP
#define CC_SEMANTICS_VARIABLE_STACK_HPP

#include "ASTParser.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

struct Variable {
    using StorageClass = Parsing::Declaration::StorageClass;
    enum class Type {
        Function, Int, Invalid
    };
    std::string name;
    StorageClass storage;
    Type type = Type::Invalid;
    Variable() = default;
    Variable(std::string n, const Type t, const StorageClass storageClassType)
        : name(std::move(n)), storage(storageClassType), type(t) {}
};

namespace Semantics {
class ScopeStack {
    using StorageClass = Parsing::Declaration::StorageClass;
    std::vector<std::unordered_map<std::string, Variable>> m_stack;
    std::unordered_set<std::string> m_args;
public:
    ScopeStack() = default;
    void push();
    void pop();
    void clearArgs();
    void addArgs(const std::vector<std::string>& args);
    void addDecl(const std::string& name,
                 const std::string& value,
                 Variable::Type type,
                 Parsing::Declaration::StorageClass storageClass);
    void addExternGlobal(const std::string& name);
    [[nodiscard]] std::tuple<Variable, bool> showIden(const std::string& name) const;
    [[nodiscard]] std::tuple<Variable, bool> showIdenInnermost(const std::string& name) const;
    [[nodiscard]] std::tuple<Variable, bool> showIdenGlobal(const std::string& name) const;
    [[nodiscard]] bool tryDeclareGlobal(const std::string& name,
                                        Variable::Type type,
                                        Parsing::Declaration::StorageClass storageClass) const;
    [[nodiscard]] bool tryDeclare(const std::string& value,
                                  Variable::Type type,
                                  Parsing::Declaration::StorageClass storageClass) const;
    [[nodiscard]] std::tuple<std::string, bool> tryCall(const std::string& callName,
                                                        Variable::Type type) const;
    [[nodiscard]] bool inArgs(const std::string& name) const noexcept;
    [[nodiscard]] bool existInInnerMost(const std::string& name,
                                        StorageClass storageClass) const;

    [[nodiscard]] size_t size() const noexcept { return m_stack.size(); }
};

} // Semantics

#endif // CC_SEMANTICS_VARIABLE_STACK_HPP
