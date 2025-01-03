#include "Lexeme.hpp"

namespace Lexing {

std::string Lexeme::getTypeName() const
{
    switch (m_type) {
        case LexemeType::OpenParen:
            return "Open Paren";
        case LexemeType::CloseParen:
            return "Close Paren";
        case LexemeType::OpenBrace:
            return "Open Brace";
        case LexemeType::CloseBrace:
            return "Close Brace";
        case LexemeType::Semicolon:
            return "Semicolon";
        case LexemeType::Return:
            return "Return";
        case LexemeType::Void:
            return "Void";
        case LexemeType::IntKeyword:
            return "Int Keyword";
        case LexemeType::Integer:
            return "Integer";
        case LexemeType::EndOfFile:
            return "End of File";
        case LexemeType::Invalid:
            return "Invalid Token";
        case LexemeType::Identifier:
            return "Identifier";
        default:
            return "Unknown Token";
    }
}

std::ostream& operator<<(std::ostream& os, const Lexeme& token)
{
    os << "line: " << token.line() << " column: " << token.column() << " type: " << token.getTypeName() << " lexeme: " << token.lexeme;
    return os;
}

bool operator==(const Lexeme &lhs, const Lexeme &rhs)
{
    return lhs.line() == rhs.line()
        && lhs.column() == rhs.column()
        && lhs.m_type == rhs.m_type
        && lhs.lexeme == rhs.lexeme;
}

bool operator!=(const Lexeme &lhs, const Lexeme &rhs)
{
    return !(lhs == rhs);
}
}
