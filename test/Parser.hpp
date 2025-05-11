#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include "Token.hpp"
#include "Parsing/Parser.hpp"

#include <vector>

std::vector<Lexing::Token> generateTokens(const std::vector<Lexing::Token::Type>& tokenTypes);
Parsing::Parser createParser(const std::vector<Lexing::Token::Type>& tokenTypes);

#endif //PARSER_HPP
