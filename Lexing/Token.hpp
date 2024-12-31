#pragma once

#ifndef CC_LEXING_TOKEN_H
#define CC_LEXING_TOKEN_H

#include <cstdint>
#include <string>
#include <utility>
#include <ostream>

namespace Lexing {

using i32 = int32_t;
using u16 = uint16_t;

enum class TokenType : u16 {
    OPEN_PAREN, CLOSE_PAREN,
    OPEN_BRACE, CLOSE_BRACE,

    SEMICOLON,

    RETURN, VOID, MAIN,

    INT_KEYWORD,

    INTEGER,

    END_OF_FILE,

    INVALID
};

class Token {
    union Data {
        i32 int32;
    };
    i32 m_line;
    u16 m_column;
public:
    TokenType type;
    std::string lexeme;
    Data data;
    Token(const i32 line, const u16 column, std::string lexeme)
        : m_line(line), m_column(column), lexeme(std::move(lexeme)) {}
    Token(const i32 line, const u16 column, TokenType type, std::string lexeme)
        : m_line(line), m_column(column), type(type), lexeme(std::move(lexeme)) {}
    Token (i32 line, u16 column, TokenType type, std::string lexeme, i32 value);
    Token() = delete;

    [[nodiscard]] i32 line() const { return m_line; }
    [[nodiscard]] u16 column() const { return m_column; }

};

[[nodiscard]] static std::string getTypeName(TokenType type);
std::ostream& operator<<(std::ostream& os, const Token& token);
bool operator==(const Token& lhs, const Token& rhs);

}

#endif // CC_LEXING_TOKEN_H