#include "Parser.hpp"

FunctionDefinition Parser::parseProgram()
{

}

bool Parser::expect(Lexing::TokenType expected, const Lexing::Token& token)
{
    return expected == token.type;
}