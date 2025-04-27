#include "Token.hpp"

namespace Lexing {

std::string Token::getTypeName() const
{
    switch (m_type) {
        case Type::OpenParen:
            return "Open Paren";
        case Type::CloseParen:
            return "Close Paren";
        case Type::OpenBrace:
            return "Open Brace";
        case Type::CloseBrace:
            return "Close Brace";
        case Type::Semicolon:
            return "Semicolon";
        case Type::Return:
            return "Return";
        case Type::Void:
            return "Void";
        case Type::IntKeyword:
            return "Int Keyword";
        case Type::Integer:
            return "Integer";
        case Type::EndOfFile:
            return "End of File";
        case Type::Invalid:
            return "Invalid Token";
        case Type::Identifier:
            return "Identifier";
        case Type::Plus:
            return "Plus";
        case Type::Minus:
            return "Minus";
        case Type::Asterisk:
            return "Asterisk";
        case Type::ForwardSlash:
            return "ForwardSlash";
        case Type::Percent:
            return "Percent";
        case Type::Ampersand:
            return "Ampersand";
        case Type::Pipe:
            return "Pipe";
        case Type::LeftShift:
            return "LeftShift";
        case Type::RightShift:
            return "RightShift";
        case Type::Circumflex:
            return "Circumflex";
        default:
            return "Unknown Token";
    }
}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    os << "line: " << token.line() << " column: " << token.column() << " type: " << token.getTypeName() << " lexeme: " << token.m_lexeme;
    return os;
}

bool operator==(const Token &lhs, const Token &rhs)
{
    return lhs.line() == rhs.line()
        && lhs.column() == rhs.column()
        && lhs.m_type == rhs.m_type
        && lhs.m_lexeme == rhs.m_lexeme;
}

bool operator!=(const Token &lhs, const Token &rhs)
{
    return !(lhs == rhs);
}
}
