#include "ASTIr.hpp"

namespace Ir {
Program::~Program() = default;

Function::~Function() = default;
StaticVariable::~StaticVariable() = default;

ReturnInst::~ReturnInst() = default;
UnaryInst::~UnaryInst() = default;
BinaryInst::~BinaryInst() = default;
CopyInst::~CopyInst() = default;
GetAddressInst::~GetAddressInst() = default;
LoadInst::~LoadInst() = default;
StoreInst::~StoreInst() = default;
JumpInst::~JumpInst() = default;
JumpIfZeroInst::~JumpIfZeroInst() = default;
JumpIfNotZeroInst::~JumpIfNotZeroInst() = default;
LabelInst::~LabelInst() = default;
FunCallInst::~FunCallInst() = default;

ValueVar::~ValueVar() = default;
ValueConst::~ValueConst() = default;
} // IR