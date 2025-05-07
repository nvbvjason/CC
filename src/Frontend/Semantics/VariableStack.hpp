#pragma once

#ifndef CC_SEMANTICS_VARIABLE_STACK_HPP
#define CC_SEMANTICS_VARIABLE_STACK_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Semantics {

class VariableStack {
    std::vector<std::unordered_map<std::string, std::string>> m_stack;
    std::unordered_set<std::string> m_args;
public:
    VariableStack() = default;
    void push();
    void pop();
    void addDecl(const std::string& name, const std::string& value);
    bool tryRename(const std::string& oldName, const std::string& newName);
    void addArgs(const std::vector<std::string>& args);
    void clearArgs();
    [[nodiscard]] bool isDeclared(const std::string& name) const;
    [[nodiscard]] std::string* find(const std::string& name) noexcept;
    [[nodiscard]] const std::string* find(const std::string& name) const noexcept;
    [[nodiscard]] bool contains(const std::string& name) const noexcept;
};

} // Semantics

#endif // CC_SEMANTICS_VARIABLE_STACK_HPP
