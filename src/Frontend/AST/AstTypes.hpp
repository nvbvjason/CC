// #pragma once
//
// #ifndef CC_PARSING_TYPES_TREE_HPP
// #define CC_PARSING_TYPES_TREE_HPP
//
// #include "ASTVisitor.hpp"
// #include "ASTBase.hpp"
//
// #include <memory>
// #include <vector>
//
// namespace Parsing {
//
// struct VarType : Type {
//     explicit VarType(const Kind kind)
//         : Type(kind) {}
//
//     void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
//     void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
// };
//
// struct FunctionType : Type {
//     std::vector<std::unique_ptr<Type>> params;
//     std::unique_ptr<Type> returnType;
//     explicit FunctionType(const Kind kind, std::unique_ptr<Type>& rT)
//         : Type(kind), returnType(std::move(rT)) {}
//
//     void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
//     void accept(ConstASTVisitor& visitor) const override { visitor.visit(*this); }
// };
//
// } // Parsing
// #endif // CC_PARSING_TYPES_TREE_HPP