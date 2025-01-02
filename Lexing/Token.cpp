#include "Token.hpp"

#include <utility>

namespace Lexing {

std::string getTypeName(const TokenType type)
{
    switch (type) {
        case TokenType::OPEN_PAREN:
            return "Open Paren";
        case TokenType::CLOSE_PAREN:
            return "Close Paren";
        case TokenType::OPEN_BRACE:
            return "Open Brace";
        case TokenType::CLOSE_BRACE:
            return "Close Brace";
        case TokenType::SEMICOLON:
            return "Semicolon";
        case TokenType::RETURN:
            return "Return";
        case TokenType::VOID:
            return "Void";
        case TokenType::INT_KEYWORD:
            return "Int Keyword";
        case TokenType::INTEGER:
            return "Integer";
        case TokenType::END_OF_FILE:
            return "End of File";
        case TokenType::INVALID:
            return "Invalid Token";
        case TokenType::IDENTIFIER:
            return "Identifier";
        default:
            return "Unknown Token";
    }

}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    os << "line: " << token.line() << " column: " << token.column() << " type: " << getTypeName(token.type) << " lexeme: " << token.lexeme;
    return os;
}

bool operator==(const Token &lhs, const Token &rhs)
{
    return lhs.line() == rhs.line()
        && lhs.column() == rhs.column()
        && lhs.type == rhs.type
        && lhs.lexeme == rhs.lexeme;
}

bool operator!=(const Token &lhs, const Token &rhs)
{
    return !(lhs == rhs);
}
}
