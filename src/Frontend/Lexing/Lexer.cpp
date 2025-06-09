#include "Lexer.hpp"

#include <string>
#include <cctype>
#include <climits>

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
        case '?':
            addToken(Token::Type::QuestionMark);
            break;
        case ':':
            addToken(Token::Type::Colon);
            break;
        case ',':
            addToken(Token::Type::Comma);
            break;
        case '+': {
            if (match('+')) {
                addToken(Token::Type::Increment);
                break;
            }
            if (match('=')) {
                addToken(Token::Type::PlusAssign);
                break;
            }
            addToken(Token::Type::Plus);
            break;
        }
        case '%': {
            if (match('=')) {
                addToken(Token::Type::ModuloAssign);
                break;
            }
            addToken(Token::Type::Percent);
            break;
        }
        case '*': {
            if (match('=')) {
                addToken(Token::Type::MultiplyAssign);
                break;
            }
            addToken(Token::Type::Asterisk);
            break;
        }
        case '^': {
            if (match('=')) {
                addToken(Token::Type::BitwiseXorAssign);
                break;
            }
            addToken(Token::Type::Circumflex);
            break;
        }
        case '!':
            if (match('=')) {
                addToken(Token::Type::LogicalNotEqual);
                break;
            }
            addToken(Token::Type::ExclamationMark);
            break;
        case '=':
            if (match('=')) {
                addToken(Token::Type::LogicalEqual);
                break;
            }
            addToken(Token::Type::Equal);
            break;
        case '&':
            if (match('&')) {
                addToken(Token::Type::LogicalAnd);
                break;
            }
            if (match('=')) {
                addToken(Token::Type::BitwiseAndAssign);
                break;
            }
            addToken(Token::Type::Ampersand);
            break;
        case '|':
            if (match('|')) {
                addToken(Token::Type::LogicalOr);
                break;
            }
            if (match('=')) {
                addToken(Token::Type::BitwiseOrAssign);
                break;
            }
            addToken(Token::Type::Pipe);
            break;
        case '>':
            if (match(">=")) {
                addToken(Token::Type::RightShiftAssign);
                break;
            }
            if (match('>')) {
                addToken(Token::Type::RightShift);
                break;
            }
            if (match('=')) {
                addToken(Token::Type::GreaterOrEqual);
                break;
            }
            addToken(Token::Type::Greater);
            break;
        case '<':
            if (match("<=")) {
                addToken(Token::Type::LeftShiftAssign);
                break;
            }
            if (match('<')) {
                addToken(Token::Type::LeftShift);
                break;
            }
            if (match('=')) {
                addToken(Token::Type::LessOrEqual);
                break;
            }
            addToken(Token::Type::Less);
            break;
        case '-':
            if (match('=')) {
                addToken(Token::Type::MinusAssign);
                break;
            }
            if (match('-')) {
                addToken(Token::Type::Decrement);
                break;
            }
            addToken(Token::Type::Minus);
            break;
        case '/':
            forwardSlash();
            break;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            break;
        default:
            if (isdigit(ch))
                number();
            else if (ch == '.')
                floating();
            else if (isalpha(ch) || ch == '_')
                identifier();
            else
                addToken(Token::Type::Invalid);
            break;
    }
}

void Lexer::forwardSlash()
{
    if (match('=')) {
        addToken(Token::Type::DivideAssign);
        return;
    }
    if (match('/'))
        while (peek() != '\n' && !isAtEnd())
            advance();
    else if (match('*')) {
        while (c_source.substr(m_current, 2) != "*/" && !isAtEnd())
            advance();
        advance();
        advance();
    }
    else
        addToken(Token::Type::ForwardSlash);
}

bool Lexer::match(const char expected)
{
    if (isAtEnd())
        return false;
    if (c_source[m_current] != expected)
        return false;
    advance();
    return true;
}

bool Lexer::match(const std::string& expected)
{
    if (c_source.size() <= m_current + expected.size() - 1)
        return false;
    for (i32 i = 0; i < expected.size(); ++i)
        if (c_source[m_current + i] != expected[i])
            return false;
    for (i32 i = 0; i < expected.size(); ++i)
        advance();
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
    ++m_column;
    if (c_source[m_current] == '\n') {
        ++m_line;
        m_column = 1;
    }
    return c_source[m_current++];
}

void Lexer::number()
{
    while (!isAtEnd() && isdigit(peek()))
        advance();
    if (peek() == '.' || tolower(peek()) == 'e') {
        floating();
        return;
    }
    const i32 endNumbers = m_current;
    while (!isAtEnd() && isalpha(peek()))
        advance();
    if (endNumbers + 2 < m_current)
        addToken(Token::Type::Invalid);
    const i32 ahead = m_current - m_start;
    const std::string text = c_source.substr(m_start, ahead);
    if (matchesUL(text, endNumbers, m_current)) {
        addToken(Token::Type::UnsignedLongLiteral);
        return;
    }
    if (tolower(text.back()) == 'l' && endNumbers + 1 == m_current) {
        addToken(Token::Type::LongLiteral);
        return;
    }
    if (tolower(text.back()) == 'u' && endNumbers + 1 == m_current) {
        if (const u64 num = std::stoul(text); UINT_MAX < num)
            addToken(Token::Type::UnsignedLongLiteral);
        else
            addToken(Token::Type::UnsignedIntegerLiteral);
        return;
    }
    if (endNumbers == m_current) {
        if (const i64 num = std::stol(text); INT_MAX < num)
            addToken(Token::Type::LongLiteral);
        else
            addToken(Token::Type::IntegerLiteral);
        return;
    }
    addToken(Token::Type::Invalid);
}

void Lexer::floating()
{
    while (!isAtEnd() && (peek() == '_' || peek() == '.' || peek() == '+' || peek() == '-' ||
        isalpha(peek()) || isdigit(peek())))
        advance();
    const i32 ahead = m_current - m_start;
    const std::string text = c_source.substr(m_start, ahead);
    static const std::regex patternDouble(R"(^([0-9]*\.[0-9]+|[0-9]+\.?)[Ee][+-]?[0-9]+|[0-9]*\.[0-9]+|[0-9]+\.$)");
    if (std::regex_match(text, patternDouble)) {
        addToken(Token::Type::DoubleLiteral);
        return;
    }
    addToken(Token::Type::Invalid);
}

void Lexer::identifier()
{
    while (isalnum(peek()) || peek() == '_')
        advance();
    const i32 ahead = m_current - m_start;
    const std::string text = c_source.substr(m_start, ahead);
    const auto iden = keywords.find(text);
    if (iden == keywords.end()) {
        addToken(Token::Type::Identifier);
        return;
    }
    addToken(iden->second);
}

void Lexer::addToken(const Token::Type type)
{
    const i32 ahead = m_current - m_start;
    std::string text = c_source.substr(m_start, ahead);
    m_tokens.emplace_back(m_line, m_column - ahead, type, text);
    if (type == Token::Type::IntegerLiteral)
        m_tokens.back().m_data = std::stoi(text);
    if (type == Token::Type::LongLiteral)
        m_tokens.back().m_data = std::stol(text);
    if (type == Token::Type::UnsignedLongLiteral)
        m_tokens.back().m_data = std::stoul(text);
    if (type == Token::Type::UnsignedIntegerLiteral)
        m_tokens.back().m_data = static_cast<u32>(std::stoull(text));
    if (type == Token::Type::DoubleLiteral) {
        try {
            double value = std::stod(text);
            m_tokens.back().m_data = value;
        } catch (const std::out_of_range&) {
            m_tokens.back().m_data = 0.0; // Or handle differently (e.g., mark token as invalid)
        }
    }
}

bool isValid(const std::string& input, const std::regex& regex)
{
    return std::regex_match(input, regex);
}

}