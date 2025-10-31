#include "ASTBase.hpp"

namespace Parsing {

[[nodiscard]] std::unique_ptr<Parsing::TypeBase> deepCopy(const TypeBase& typeBase);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const VarType& typeBase);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const FuncType& funcType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const PointerType& pointerType);
[[nodiscard]] std::unique_ptr<TypeBase> deepCopy(const ArrayType& arrayType);

} // Parsing