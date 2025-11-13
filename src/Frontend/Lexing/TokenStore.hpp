#pragma once

#include "ShortTypes.hpp"
#include "Token.hpp"

#include <cassert>
#include <vector>

class TokenStore {
    std::vector<std::variant<i8, u8, i32, i64, u32, u64, double>> m_data;
    std::vector<i32> m_line;
    std::vector<u16> m_column;
    std::vector<Lexing::Token::Type> m_type;
    std::vector<std::string> m_lexeme;
public:
    void clear()
    {
        m_data.clear();
        m_line.clear();
        m_column.clear();
        m_type.clear();
        m_lexeme.clear();
    }
    void reserve(const size_t size)
    {
        m_data.reserve(size);
        m_line.reserve(size);
        m_column.reserve(size);
        m_type.reserve(size);
        m_lexeme.reserve(size);
    }
    void emplaceBack(
        std::variant<i8, u8, i32, i64, u32, u64, double> value,
        const i32 line,
        const u16 column,
        Lexing::Token::Type type,
        std::string lexeme)
    {
        m_data.emplace_back(value);
        m_line.emplace_back(line);
        m_column.emplace_back(column);
        m_type.emplace_back(type);
        m_lexeme.emplace_back(std::move(lexeme));
    }
    [[nodiscard]] Lexing::Token getToken(const size_t i) const
    {
        assert(i < m_line.size());
        return {getValue(i), getLineNumber(i), getColumnNumber(i), getType(i), getLexeme(i)};
    }
    [[nodiscard]] std::variant<i8, u8, i32, i64, u32, u64, double> getValue(const size_t i) const
    {
        assert(i < m_line.size());
        return m_data[i];
    }
    [[nodiscard]] i32 getLineNumber(const size_t i) const
    {
        assert(i < m_line.size());
        return m_line[i];
    }
    [[nodiscard]] u16 getColumnNumber(const size_t i) const
    {
        assert(i < m_line.size());
        return m_column[i];
    }
    [[nodiscard]] Lexing::Token::Type getType(const size_t i) const
    {
        assert(i < m_line.size());
        return m_type[i];
    }
    [[nodiscard]] std::string getLexeme(const size_t i) const
    {
        assert(i < m_line.size());
        return m_lexeme[i];
    }
    [[nodiscard]] size_t size() const { return m_line.size(); }
};