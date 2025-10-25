#include "Lexer.hpp"

#include <string>
#include <cctype>
#include <cmath>

namespace Lexing {

std::vector<Error> Lexer::getLexemes()
{
    while (!isAtEnd()) {
        m_start = m_current;
        scanToken();
    }
    tokenStore.emplaceBack(0, m_line, m_column, Type::EndOfFile, "");
    return errors;
}

void Lexer::scanToken()
{
    switch (const char ch = advance()) {
        case '(':
            addToken(Type::OpenParen);
            break;
        case ')':
            addToken(Type::CloseParen);
            break;
        case '{':
            addToken(Type::OpenBrace);
            break;
        case '}':
            addToken(Type::CloseBrace);
            break;
        case '[':
            addToken(Type::OpenSqBracket);
            break;
        case ']':
            addToken(Type::CloseSqBracket);
            break;
        case ';':
            addToken(Type::Semicolon);
            break;
        case '~':
            addToken(Type::Tilde);
            break;
        case '?':
            addToken(Type::QuestionMark);
            break;
        case ':':
            addToken(Type::Colon);
            break;
        case ',':
            addToken(Type::Comma);
            break;
        case '+': {
            if (match('+')) {
                addToken(Type::Increment);
                break;
            }
            if (match('=')) {
                addToken(Type::PlusAssign);
                break;
            }
            addToken(Type::Plus);
            break;
        }
        case '%': {
            if (match('=')) {
                addToken(Type::ModuloAssign);
                break;
            }
            addToken(Type::Percent);
            break;
        }
        case '*': {
            if (match('=')) {
                addToken(Type::MultiplyAssign);
                break;
            }
            addToken(Type::Asterisk);
            break;
        }
        case '^': {
            if (match('=')) {
                addToken(Type::BitwiseXorAssign);
                break;
            }
            addToken(Type::Circumflex);
            break;
        }
        case '!':
            if (match('=')) {
                addToken(Type::LogicalNotEqual);
                break;
            }
            addToken(Type::ExclamationMark);
            break;
        case '=':
            if (match('=')) {
                addToken(Type::LogicalEqual);
                break;
            }
            addToken(Type::Equal);
            break;
        case '&':
            if (match('&')) {
                addToken(Type::LogicalAnd);
                break;
            }
            if (match('=')) {
                addToken(Type::BitwiseAndAssign);
                break;
            }
            addToken(Type::Ampersand);
            break;
        case '|':
            if (match('|')) {
                addToken(Type::LogicalOr);
                break;
            }
            if (match('=')) {
                addToken(Type::BitwiseOrAssign);
                break;
            }
            addToken(Type::Pipe);
            break;
        case '>':
            if (match(">=")) {
                addToken(Type::RightShiftAssign);
                break;
            }
            if (match('>')) {
                addToken(Type::RightShift);
                break;
            }
            if (match('=')) {
                addToken(Type::GreaterOrEqual);
                break;
            }
            addToken(Type::Greater);
            break;
        case '<':
            if (match("<=")) {
                addToken(Type::LeftShiftAssign);
                break;
            }
            if (match('<')) {
                addToken(Type::LeftShift);
                break;
            }
            if (match('=')) {
                addToken(Type::LessOrEqual);
                break;
            }
            addToken(Type::Less);
            break;
        case '-':
            if (match('=')) {
                addToken(Type::MinusAssign);
                break;
            }
            if (match('-')) {
                addToken(Type::Decrement);
                break;
            }
            addToken(Type::Minus);
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
                addToken(Type::Invalid);
            break;
    }
}

void Lexer::forwardSlash()
{
    if (match('=')) {
        addToken(Type::DivideAssign);
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
        addToken(Type::ForwardSlash);
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
    u64 num = c_source[m_start] - '0';
    while (!isAtEnd() && isdigit(peek())) {
        num *= 10;
        num += peek() - '0';
        advance();
    }
    if (peek() == '.' || tolower(peek()) == 'e') {
        floating();
        return;
    }
    const i32 endNumbers = m_current;
    while (!isAtEnd() && isalpha(peek()))
        advance();
    if (endNumbers + 2 < m_current)
        addToken(Type::Invalid);
    const i32 ahead = m_current - m_start;
    std::string text = c_source.substr(m_start, ahead);
    if (matchesUL(text, endNumbers, m_current)) {
        addToken(Type::UnsignedLongLiteral, num, ahead, text);
        return;
    }
    if (tolower(text.back()) == 'l' && endNumbers + 1 == m_current) {
        addToken(Type::LongLiteral, num, ahead, text);
        return;
    }
    if (tolower(text.back()) == 'u' && endNumbers + 1 == m_current) {
        if (MAX_U32 < num)
            addToken(Type::UnsignedLongLiteral, num, ahead, text);
        else
            addToken(Type::UnsignedIntegerLiteral, num, ahead, text);
        return;
    }
    if (endNumbers == m_current) {
        if (MAX_I32 < num)
            addToken(Type::LongLiteral, num, ahead, text);
        else
            addToken(Type::IntegerLiteral, num, ahead, text);
        return;
    }
    addToken(Type::Invalid);
}

void Lexer::floating()
{
    if (peek() == '.')
        advance();
    while (!isAtEnd() && isdigit(peek()))
        advance();
    if (tolower(peek()) == 'e') {
        advance();
        if (peek() == '+' || peek() == '-')
            advance();
        if (!isdigit(peek())) {
            addToken(Type::Invalid);
            return;
        }
        while (!isAtEnd() && isdigit(peek()))
            advance();
    }
    if (peek() == '.' || isalnum(peek()) || peek() == '_') {
        addToken(Type::Invalid);
        return;
    }
    addToken(Type::DoubleLiteral);
}

void Lexer::identifier()
{
    while (isalnum(peek()) || peek() == '_')
        advance();
    const i32 ahead = m_current - m_start;
    const std::string text = c_source.substr(m_start, ahead);
    const auto iden = keywords.find(text);
    if (iden == keywords.end()) {
        addToken(Type::Identifier);
        return;
    }
    addToken(iden->second);
}

void Lexer::addToken(const Token::Type type, const u64 num, const i32 ahead, std::string& text) const
{
    std::variant<i32, i64, u32, u64, double> value;
    if (type == Type::IntegerLiteral)
        value = static_cast<i32>(num);
    else if (type == Type::LongLiteral)
        value = static_cast<i64>(num);
    else if (type == Type::UnsignedIntegerLiteral)
        value = static_cast<u32>(num);
    else if (type == Type::UnsignedLongLiteral)
        value = num;
    tokenStore.emplaceBack(
        value,
        m_line,
        m_column - ahead,
        type,
        std::move(text));
}

void Lexer::addToken(const Token::Type type)
{
    const i32 ahead = m_current - m_start;
    std::string text = c_source.substr(m_start, ahead);
    std::variant<i32, i64, u32, u64, double> valueToStore = 0;
    if (type == Type::DoubleLiteral) {
        const double value = std::strtod(text.c_str(), nullptr);
        if (errno == ERANGE && value == HUGE_VAL)
            valueToStore = std::numeric_limits<double>::infinity();
        else
            valueToStore = value;
    }
    tokenStore.emplaceBack(
        valueToStore,
        m_line,
        m_column - ahead,
        type,
        std::move(text));
    if (type == Type::Invalid)
        errors.emplace_back("Unknown token type", tokenStore.size() - 1);
}
}