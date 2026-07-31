#pragma once

#include "ShortTypes.hpp"
#include "Token.hpp"

#include <cassert>
#include <vector>

class TokenStore {
    std::vector<std::variant<char, i8, u8, i32, i64, u32, u64, double>> data;
    std::vector<i32> line;
    std::vector<u16> column;
    std::vector<Lexing::Token::Type> type;
    std::vector<std::string> lexeme;
public:
    void clear()
    {
        data.clear();
        line.clear();
        column.clear();
        type.clear();
        lexeme.clear();
    }

    void reserve(const size_t size)
    {
        data.reserve(size);
        line.reserve(size);
        column.reserve(size);
        type.reserve(size);
        lexeme.reserve(size);
    }

    void emplaceBack(
        std::variant<char, i8, u8, i32, i64, u32, u64, double> v,
        const i32 l,
        const u16 c,
        Lexing::Token::Type t,
        std::string lex)
    {
        data.emplace_back(v);
        line.emplace_back(l);
        column.emplace_back(c);
        type.emplace_back(t);
        lexeme.emplace_back(std::move(lex));
    }

    [[nodiscard]] Lexing::Token getToken(const size_t i) const
    {
        assert(i < line.size());
        return {getValue(i), getLineNumber(i), getColumnNumber(i), getType(i), getLexeme(i)};
    }

    [[nodiscard]] std::variant<char, i8, u8, i32, i64, u32, u64, double> getValue(const size_t i) const
    {
        assert(i < line.size());
        return data[i];
    }

    [[nodiscard]] i32 getLineNumber(const size_t i) const
    {
        assert(i < line.size());
        return line[i];
    }

    [[nodiscard]] u16 getColumnNumber(const size_t i) const
    {
        assert(i < line.size());
        return column[i];
    }

    [[nodiscard]] Lexing::Token::Type getType(const size_t i) const
    {
        assert(i < line.size());
        return type[i];
    }

    [[nodiscard]] std::string getLexeme(const size_t i) const
    {
        assert(i < line.size());
        return lexeme[i];
    }

    [[nodiscard]] size_t size() const { return line.size(); }
};