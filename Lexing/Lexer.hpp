#pragma once

#ifndef CC_LEXING_LEXER_H
#define CC_LEXING_LEXER_H

#include "Lexeme.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace Lexing {

    static std::unordered_map<std::string, LexemeType> keywords = {
        { "return", LexemeType::Return },
        { "int", LexemeType::IntKeyword },
        { "void", LexemeType::Void },
    };

class Lexer {
    const std::string& c_source;
    i32 m_current = 0;
    i32 m_start = 0;
    i32 m_line = 1;
    u16 m_column = 1;
    std::vector<Lexeme> m_tokens;
public:
    explicit Lexer(const std::string& input)
        : c_source(input) {}
    [[nodiscard]] std::vector<Lexeme> tokenize();
private:
    [[nodiscard]] bool isAtEnd() const { return c_source.size() <= m_current; }
    [[nodiscard]] char peek() const;
    [[nodiscard]] char peekNext() const;

    bool match(char expected);
    char advance();
    void integer();
    void identifier();
    void scanToken();
    void addToken(LexemeType type);
    void addToken(LexemeType type, i32 value);
};


}

#endif // CC_LEXING_LEXER_H