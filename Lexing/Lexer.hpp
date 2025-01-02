#pragma once

#ifndef CC_LEXING_LEXER_H
#define CC_LEXING_LEXER_H

#include "Token.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace Lexing {

    static std::unordered_map<std::string, TokenType> keywords = {
        { "return", TokenType::Return },
        { "int", TokenType::IntKeyword },
        { "void", TokenType::Void },
        {"main", TokenType::Identifier}
    };

class Lexer {
    const std::string& c_source;
    i32 m_current = 0;
    i32 m_start = 0;
    i32 m_line = 1;
    u16 m_column = 1;
    std::vector<Token> m_tokens;
public:
    explicit Lexer(const std::string& input)
        : c_source(input) {}
    [[nodiscard]] std::vector<Token> tokenize();
private:
    [[nodiscard]] bool isAtEnd() const { return c_source.size() <= m_current; }
    [[nodiscard]] char peek() const;
    [[nodiscard]] char peekNext() const;

    bool match(char expected);
    char advance();
    void integer();
    void identifier();
    void scanToken();
    void addToken(TokenType type);
    void addToken(TokenType type, i32 value);
};


}

#endif // CC_LEXING_LEXER_H