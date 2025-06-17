#pragma once

#ifndef CC_LEXING_LEXER_HPP
#define CC_LEXING_LEXER_HPP

#include <regex>

#include "Token.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace Lexing {


class Lexer {
    using Type = Token::Type;

    const std::string& c_source;
    i32 m_current = 0;
    i32 m_start = 0;
    i32 m_line = 1;
    u16 m_column = 1;
    std::vector<Token> m_tokens;
    static inline std::unordered_map<std::string, Token::Type> keywords = {
        { "return", Type::Return },
        { "int", Type::IntKeyword },
        { "void", Type::Void },
        { "if", Type::If },
        { "else", Type::Else },
        { "do", Type::Do },
        { "while", Type::While },
        { "for", Type::For },
        { "break", Type::Break },
        { "continue", Type::Continue },
        { "goto", Type::Goto },
        { "switch", Type::Switch },
        { "case", Type::Case },
        { "default", Type::Default },
        { "static", Type::Static },
        { "extern", Type::Extern },
        {"long", Type::LongKeyword},
        {"signed", Type::Signed},
        {"unsigned", Type::Unsigned},
        {"double", Type::DoubleKeyword},
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
    void scanToken();
    char advance();
    void addToken(Token::Type type);

    void forwardSlash();
    void identifier();
    void number();
    void floating();
};

inline bool matchesUL(const std::string& text, const i32 endNumbers, const i32 current)
{
    if (endNumbers + 2 != current)
        return false;
    return (tolower(text[text.size() - 2]) == 'l' &&
            tolower(text.back()) == 'u') ||
           (tolower(text[text.size() - 2]) == 'u' &&
            tolower(text.back()) == 'l');
}

}
#endif // CC_LEXING_LEXER_HPP