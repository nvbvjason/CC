#pragma once

#ifndef CC_LEXING_TOKEN_H
#define CC_LEXING_TOKEN_H

#include "ShortTypes.hpp"

#include <string>
#include <utility>
#include <ostream>
#include <regex>
#include <variant>

namespace Lexing {


struct Token {
    enum class Type : u16 {
        // Bracketing Symbols
        OpenParen,       CloseParen,     // (  )
        OpenBrace,       CloseBrace,     // {  }

        // Punctuation & Symbols
        Semicolon,                       // ;
        Comma,                           // ,
        Tilde,                           // ~
        ExclamationMark,                 // !

        // Arithmetic Operators
        Plus,                            // +
        Minus,                           // -
        Asterisk,                        // *
        ForwardSlash,                    // /
        Percent,                         // %

        // Bitwise Operators
        Ampersand,                       // &
        Pipe,                            // |
        Circumflex,                      // ^
        LeftShift,                       // <<
        RightShift,                      // >>

        // Special Operators
        Decrement,                       // --
        Increment,                       // ++

        // Logical Operators
        LogicalAnd,                      // &&
        LogicalOr,                       // ||
        LogicalNotEqual,                 // !=
        LogicalEqual,                    // ==
        Less,                            // <
        LessOrEqual,                     // <=
        Greater,                         // >
        GreaterOrEqual,                  // >=

        // Identifiers & Literals
        Identifier,                      // User-defined names
        IntegerLiteral,                  // Integer literals
        UnsignedIntegerLiteral,          // Unsigned Integer literals
        LongLiteral,                     // Long literal
        UnsignedLongLiteral,             // Unsigned long literals

        // Assignment
        Equal,                           // =
        PlusAssign,                      // +=
        MinusAssign,                     // -=
        MultiplyAssign,                  // *=
        DivideAssign,                    // /=
        ModuloAssign,                    // %=
        BitwiseAndAssign,                // &=
        BitwiseOrAssign,                 // |=
        BitwiseXorAssign,                // ^=
        LeftShiftAssign,                 // <<=
        RightShiftAssign,                // >>=

        // Ternary
        QuestionMark,                    // ?
        Colon,                           // :

        // Keywords
        Return,                          // 'return'
        Void,                            // 'void'
        IntKeyword,                      // 'int'
        LongKeyword,                     // 'long'
        If,                              // if
        Else,                            // else
        Do,                              // do
        While,                           // while
        For,                             // for
        Break,                           // break
        Continue,                        // continue
        Goto,                            // goto
        Switch,                          // switch
        Case,                            // case
        Default,                         // default
        Static,                          // static
        Extern,                          // extern
        Signed,                          // Signed
        Unsigned,                        // Unsigned

        // Special Tokens
        EndOfFile,                       // EOF marker
        NotAToken,                       // maybe bad design
        Invalid                          // Invalid token
    };
    std::variant<i32, i64, u32, u64> m_data;
    i32 m_line;
    u16 m_column;
    Type m_type;
    std::string m_lexeme;
    Token(const i32 line, const u16 column, const Type type, std::string lexeme)
        : m_line(line), m_column(column), m_type(type), m_lexeme(std::move(lexeme)) {}
    [[nodiscard]] i32 line() const { return m_line; }
    [[nodiscard]] u16 column() const { return m_column; }
    [[nodiscard]] i32 getI32Value() const { return std::get<i32>(m_data); }
    [[nodiscard]] u32 getU32Value() const { return std::get<u32>(m_data); }
    [[nodiscard]] i64 getI64Value() const { return std::get<i64>(m_data); }
    [[nodiscard]] u64 getU64Value() const { return std::get<u64>(m_data); }
    [[nodiscard]] std::string getTypeName() const;
};

std::ostream& operator<<(std::ostream& os, const Token& token);
bool isValid(const std::string& input, const std::regex& regex);
bool operator==(const Token& lhs, const Token& rhs);

}

#endif // CC_LEXING_TOKEN_H