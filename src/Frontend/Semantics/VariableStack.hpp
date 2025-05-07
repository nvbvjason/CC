#pragma once

#ifndef CC_SEMANTICS_VARIABLE_STACK_HPP
#define CC_SEMANTICS_VARIABLE_STACK_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Semantics {

struct Variable {
    enum class Type {
        Function,
        Int
    };
    std::string name;
    Type type;
    Variable(const std::string& name, Type type)
        : name(name), type(type) {}
    Variable() = delete;
};

class VariableStack {
    std::vector<std::unordered_map<std::string, Variable>> m_stack;
    std::unordered_set<std::string> m_args;
public:
    VariableStack() = default;
    void push();
    void pop();
    void addDecl(const std::string& name, const std::string& value, Variable::Type type);
    void addArgs(const std::vector<std::string>& args);
    void clearArgs();
    [[nodiscard]] bool tryDeclare(const std::string& name, const Variable::Type type) const;
    [[nodiscard]] std::string tryCall(const std::string& name, Variable::Type type) const;
    [[nodiscard]] bool inArg(const std::string& name) const noexcept;
    [[nodiscard]] bool inInnerMost(const std::string& name) const;
};

} // Semantics

#endif // CC_SEMANTICS_VARIABLE_STACK_HPP
