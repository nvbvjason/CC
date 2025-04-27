#include "Lexer.hpp"

#include <string>
#include <cctype>
#include <algorithm>

namespace Lexing {

i32 Lexer::getLexemes(std::vector<Token>& lexemes)
{
    while (!isAtEnd()) {
        m_start = m_current;
        scanToken();
    }
    lexemes = m_tokens;
    for (const Token& lexeme : lexemes)
        if (lexeme.m_type == Token::Type::Invalid)
            return 1;
    lexemes.emplace_back(m_line, m_column, Token::Type::EndOfFile, "");
    return 0;
}

void Lexer::scanToken()
{
    switch (const char ch = advance()) {
        case '(':
            addToken(Token::Type::OpenParen);
            break;
        case ')':
            addToken(Token::Type::CloseParen);
            break;
        case '{':
            addToken(Token::Type::OpenBrace);
            break;
        case '}':
            addToken(Token::Type::CloseBrace);
            break;
        case ';':
            addToken(Token::Type::Semicolon);
            break;
        case '~':
            addToken(Token::Type::Tilde);
            break;
        case '%':
            addToken(Token::Type::Percent);
            break;
        case '*':
            addToken(Token::Type::Asterisk);
            break;
        case '&':
            addToken(Token::Type::Ampersand);
            break;
        case '|':
            addToken(Token::Type::Pipe);
            break;
        case '+':
            addToken(Token::Type::Plus);
            break;
        case '>':
            if (match('>')) {
                addToken(Token::Type::RightShift);
                break;
            }
            break;
        case '<':
            if (match('<')) {
                addToken(Token::Type::LeftShift);
                break;
            }
            break;
        case '-':
            if (match('-')) {
                addToken(Token::Type::Decrement);
                break;
            }
            addToken(Token::Type::Minus);
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
                advance();
            }
            else
                addToken(Token::Type::ForwardSlash);
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
                addToken(Token::Type::Invalid);
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
    if (const std::string str = c_source.substr(m_start, m_current - m_start); !std::ranges::all_of(str,isdigit))
        addToken(Token::Type::Invalid);
    const i32 value = std::stoi(c_source.substr(m_start, m_current - m_start));
    addToken(Token::Type::Integer, value);
}

void Lexer::identifier()
{
    while (isalpha(peek()))
        advance();
    const std::string text = c_source.substr(m_start, m_current - m_start);
    const auto iden = keywords.find(text);
    if (iden == keywords.end()) {
        addToken(Token::Type::Identifier);
        return;
    }
    addToken(iden->second);
}

void Lexer::addToken(Token::Type type, i32 value)
{
    std::string text = c_source.substr(m_start, m_current - m_start);
    m_tokens.emplace_back(m_line, m_column, type, text, value);
    m_column += m_current - m_start;
}

void Lexer::addToken(const Token::Type type)
{
    std::string text = c_source.substr(m_start, m_current - m_start);
    m_tokens.emplace_back(m_line, m_column, type, text);
    m_column += m_current - m_start;
}

}
