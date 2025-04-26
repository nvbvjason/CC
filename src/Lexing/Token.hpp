#pragma once

#ifndef CC_LEXING_TOKEN_H
#define CC_LEXING_TOKEN_H

#include "ShortTypes.hpp"

#include <string>
#include <utility>
#include <ostream>
#include <variant>

namespace Lexing {


struct Token {
    enum class Type : u16 {
        // Bracketing Symbols
        OpenParen, CloseParen,
        OpenBrace, CloseBrace,

        // Operators
        Semicolon,
        Tilde,
        Plus, Minus,
        Decrement,
        Asterisk,
        ForwardSlash,
        Percent,

        // Keywords
        Return,
        Void,
        IntKeyword,

        Identifier,

        Integer,

        EndOfFile,

        Invalid
    };
    std::variant<i32> m_data;
    i32 m_line;
    u16 m_column;
    Type m_type;
    std::string m_lexeme;
    Token(const i32 line, const u16 column, const Type type, std::string lexeme)
        : m_line(line), m_column(column), m_type(type), m_lexeme(std::move(lexeme)) {}
    Token(const i32 line, const u16 column, const Type type, std::string lexeme, const i32 value)
        : m_data(value), m_line(line), m_column(column), m_type(type), m_lexeme(std::move(lexeme)) {}
    [[nodiscard]] i32 line() const { return m_line; }
    [[nodiscard]] u16 column() const { return m_column; }
    [[nodiscard]] std::string getTypeName() const;
};

std::ostream& operator<<(std::ostream& os, const Token& token);
bool operator==(const Token& lhs, const Token& rhs);

}

#endif // CC_LEXING_TOKEN_H