#pragma once

#include "Type.hpp"

Type getCommonType(Type t1, Type t2);
bool isSigned(Type t);
i32 getSize(Type t);
bool isInteger(Type t);
bool isArithmetic(Type t);