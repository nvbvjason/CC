#include "Lexer.hpp"

namespace Lexing {

std::vector<Token> Lexer::tokenize()
{
    while (!isAtEnd()) {
        m_start = m_current;
        scanToken();
    }
    return m_tokens;
}

void Lexer::scanToken()
{
    switch (const char ch = advance()) {
        case '(':
            addToken(TokenType::OPEN_PAREN);
            break;
        case ')':
            addToken(TokenType::CLOSE_PAREN);
            break;
        case '{':
            addToken(TokenType::OPEN_BRACE);
            break;
        case '}':
            addToken(TokenType::CLOSE_BRACE);
            break;
        case ';':
            addToken(TokenType::SEMICOLON);
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
                addToken(TokenType::INVALID);
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
    while (isdigit(peek()))
        advance();
    i32 value = std::stoi(c_source.substr(m_start, m_current - m_start));
    addToken(TokenType::INTEGER, value);
}

void Lexer::identifier()
{
    while (isalpha(peek()))
        advance();
    std::string text = c_source.substr(m_start, m_current - m_start);
    const auto iden = keywords.find(text);
    if (iden == keywords.end()) {
        addToken(TokenType::INVALID);
        return;
    }
    addToken(iden->second);
}

void Lexer::addToken(TokenType type, i32 value)
{
    std::string text = c_source.substr(m_start, m_current - m_start);
    m_tokens.emplace_back(m_line, m_column, type, text, value);
    m_column += m_current - m_start;
}

void Lexer::addToken(const TokenType type)
{
    std::string text = c_source.substr(m_start, m_current - m_start);
    m_tokens.emplace_back(m_line, m_column, type, text);
    m_column += m_current - m_start;
}

}