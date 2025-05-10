#pragma once

#ifndef CC_SEMANTICS_VARIABLE_STACK_HPP
#define CC_SEMANTICS_VARIABLE_STACK_HPP

#include "ShortTypes.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ASTParser.hpp"

namespace Semantics {

struct Variable {
    enum class Type : u8 {
        Function,
        Int
    };
    std::string name;
    Type type;
    Parsing::Declaration::StorageClass storage;
    Variable(const std::string& name, Type type, Parsing::Declaration::StorageClass storage)
        : name(name), type(type), storage(storage) {}

    Variable() = delete;
};

class VariableStack {
    std::vector<std::unordered_map<std::string, Variable>> m_stack;
    std::unordered_set<std::string> m_args;
public:
    VariableStack() = default;
    void push();
    void pop();
    void clearArgs();
    void addArgs(const std::vector<std::string>& args);
    void addDecl(const std::string& name,
                 const std::string& value,
                 Variable::Type type,
                 Parsing::Declaration::StorageClass storageClass);
    [[nodiscard]] bool tryDeclare(const std::string& value,
                                  Variable::Type type,
                                  Parsing::Declaration::StorageClass storageClass) const;
    [[nodiscard]] std::string tryCall(const std::string& value,
                                      Variable::Type type) const;
    [[nodiscard]] bool inArg(const std::string& name) const noexcept;
    [[nodiscard]] bool inInnerMost(const std::string& name) const;
};

} // Semantics

#endif // CC_SEMANTICS_VARIABLE_STACK_HPP
