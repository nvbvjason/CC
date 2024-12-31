#pragma once

#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>

#include "Program.hpp"
#include "FunctionDefinition.hpp"
#include "Statement.hpp"
#include "Expression.hpp"
#include "../Lexing/Token.hpp"

class Parser {
    std::vector<Lexing::Token> c_tokens;
public:
    Parser(const Parser& other) = delete;
    Parser(Parser&& other) = delete;
    Parser& operator=(const Parser& other) = delete;
    Parser& operator=(Parser&& other) = delete;
    explicit Parser(std::vector<Lexing::Token> tokens)
        : c_tokens(std::move(tokens)) {}

    Program parse();
private:
    FunctionDefinition parseProgram();
    Statement parseFunction();
    Expression parseStatement();

    bool expect(Lexing::TokenType expected, const Lexing::Token& token);
};



#endif //PARSER_HPP
