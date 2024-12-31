#pragma once

#ifndef MANUELTESTING_H
#define MANUELTESTING_H

#include "../Lexing/Token.h"

#include <vector>

void chapter_1();

bool allTokensValid(const std::vector<Lexing::Token>& tokens);

#endif //MANUELTESTING_H
