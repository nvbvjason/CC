#pragma once

#ifndef CC_SEMANTICS_VARIABLE_STACK_HPP
#define CC_SEMANTICS_VARIABLE_STACK_HPP

#include "ShortTypes.hpp"

#include "ASTParser.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Semantics {

struct Variable {
    enum class Type : u8 {
        Function,
        Int
    };
    std::string name;
    Type type;
    Parsing::Declaration::StorageClass storage;
    Variable(std::string  name, Type type, Parsing::Declaration::StorageClass storage)
        : name(std::move(name)), type(type), storage(storage) {}

    Variable() = delete;
};

class VariableStack {
    using StorageClass = Parsing::Declaration::StorageClass;
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
    [[nodiscard]] bool cannotDeclareInInnerMost(const std::string& name,
                                   StorageClass storageClass) const;
};

} // Semantics

#endif // CC_SEMANTICS_VARIABLE_STACK_HPP
