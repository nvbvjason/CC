#include "Lexer.hpp"

#include <bits/ranges_algo.h>

namespace Lexing {

i32 Lexer::getLexems(std::vector<Lexeme>& lexemes)
{
    while (!isAtEnd()) {
        m_start = m_current;
        scanToken();
    }
    lexemes = m_tokens;
    for (const Lexeme& lexeme : lexemes)
        if (lexeme.m_type == LexemeType::Invalid)
            return 1;
    return 0;
}

void Lexer::scanToken()
{
    switch (const char ch = advance()) {
        case '(':
            addToken(LexemeType::OpenParen);
            break;
        case ')':
            addToken(LexemeType::CloseParen);
            break;
        case '{':
            addToken(LexemeType::OpenBrace);
            break;
        case '}':
            addToken(LexemeType::CloseBrace);
            break;
        case ';':
            addToken(LexemeType::Semicolon);
            break;
        case '~':
            addToken(LexemeType::Tilde);
            break;
        case '-':
            if (match('-')) {
                addToken(LexemeType::Decrement);
                break;
            }
            addToken(LexemeType::Minus);
            break;
        case '/':
            if (match('/'))
                while (peek() != '\n' && !isAtEnd())
                    advance();
            else if (match('*')) {
                while (c_source.substr(m_current, 2) != "*/" && !isAtEnd()) {
                    if (peek() == '\n')
                        ++m_line;
                    advance();
                }
                advance();
            }
            break;
        case ' ':
        case '\t':
        case '\r':
            ++m_column;
            break;
        case '\n':
            m_column = 1;
            ++m_line;
            break;
        default:
            if (isdigit(ch))
                integer();
            else if (isalpha(ch))
                identifier();
            else
                addToken(LexemeType::Invalid);
            break;
    }
}

bool Lexer::match(const char expected)
{
    if (isAtEnd())
        return false;
    if (c_source[m_current] != expected)
        return false;
    ++m_current;
    return true;
}

char Lexer::peek() const
{
    if (isAtEnd())
        return '\0';
    return c_source[m_current];
}

char Lexer::peekNext() const
{
    if (c_source.size() <= m_current + 1)
        return '\0';
    return c_source[m_current + 1];
}

char Lexer::advance()
{
    return c_source[m_current++];
}

void Lexer::integer()
{
    while (isalnum(peek()))
        advance();
    const std::string str = c_source.substr(m_start, m_current - m_start);
    if (!std::ranges::all_of(str,::isdigit))
        addToken(LexemeType::Invalid);
    i32 value = std::stoi(c_source.substr(m_start, m_current - m_start));
    addToken(LexemeType::Integer, value);
}

void Lexer::identifier()
{
    while (isalpha(peek()))
        advance();
    const std::string text = c_source.substr(m_start, m_current - m_start);
    const auto iden = keywords.find(text);
    if (iden == keywords.end()) {
        addToken(LexemeType::Identifier);
        return;
    }
    addToken(iden->second);
}

void Lexer::addToken(LexemeType type, i32 value)
{
    std::string text = c_source.substr(m_start, m_current - m_start);
    m_tokens.emplace_back(m_line, m_column, type, text, value);
    m_column += m_current - m_start;
}

void Lexer::addToken(const LexemeType type)
{
    std::string text = c_source.substr(m_start, m_current - m_start);
    m_tokens.emplace_back(m_line, m_column, type, text);
    m_column += m_current - m_start;
}

}
