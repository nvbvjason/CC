#include "Token.hpp"

namespace Lexing {

std::string Token::getTypeName() const
{
    switch (m_type) {
        case TokenType::OpenParen:
            return "Open Paren";
        case TokenType::CloseParen:
            return "Close Paren";
        case TokenType::OpenBrace:
            return "Open Brace";
        case TokenType::CloseBrace:
            return "Close Brace";
        case TokenType::Semicolon:
            return "Semicolon";
        case TokenType::Return:
            return "Return";
        case TokenType::Void:
            return "Void";
        case TokenType::IntKeyword:
            return "Int Keyword";
        case TokenType::Integer:
            return "Integer";
        case TokenType::EndOfFile:
            return "End of File";
        case TokenType::Invalid:
            return "Invalid Token";
        case TokenType::Identifier:
            return "Identifier";
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
