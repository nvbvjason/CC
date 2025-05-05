#pragma once

#ifndef CC_LEXING_LEXER_HPP
#define CC_LEXING_LEXER_HPP

#include "Token.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace Lexing {


class Lexer {
    const std::string& c_source;
    i32 m_current = 0;
    i32 m_start = 0;
    i32 m_line = 1;
    u16 m_column = 1;
    std::vector<Token> m_tokens;
    static inline std::unordered_map<std::string, Token::Type> keywords = {
        { "return", Token::Type::Return },
        { "int", Token::Type::IntKeyword },
        { "void", Token::Type::Void },
        { "if", Token::Type::If },
        { "else", Token::Type::Else },
        { "do", Token::Type::Do },
        { "while", Token::Type::While },
        { "for", Token::Type::For },
        { "break", Token::Type::Break },
        { "continue", Token::Type::Continue },
        { "goto", Token::Type::Goto },
        { "switch", Token::Type::Switch },
        { "case", Token::Type::Case },
        { "default", Token::Type::Default },
    };
public:
    explicit Lexer(const std::string& input)
        : c_source(input) { }
    i32 getLexemes(std::vector<Token>& lexemes);
private:
    [[nodiscard]] bool isAtEnd() const { return c_source.size() <= m_current; }
    [[nodiscard]] char peek() const;
    [[nodiscard]] char peekNext() const;

    bool match(char expected);
    bool match(const std::string& expected);
    char advance();
    void integer();
    void identifier();
    void scanToken();
    void addToken(Token::Type type);
};

}

#endif // CC_LEXING_LEXER_HPP