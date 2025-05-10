#include "Token.hpp"

namespace Lexing {

std::string Token::getTypeName() const
{
    switch (m_type) {
        // Bracketing Symbols
        case Type::OpenParen:           return "Open Paren";
        case Type::CloseParen:          return "Close Paren";
        case Type::OpenBrace:           return "Open Brace";
        case Type::CloseBrace:          return "Close Brace";

        // Punctuation & Symbols
        case Type::Semicolon:           return "Semicolon";
        case Type::Comma:               return "Comma";
        case Type::Tilde:               return "Tilde";
        case Type::ExclamationMark:     return "Exclamation Mark";

        // Arithmetic Operators
        case Type::Plus:                return "Plus";
        case Type::Minus:               return "Minus";
        case Type::Asterisk:            return "Asterisk";
        case Type::ForwardSlash:        return "Forward Slash";
        case Type::Percent:             return "Percent";

        // Bitwise Operators
        case Type::Ampersand:           return "Ampersand";
        case Type::Pipe:                return "Pipe";
        case Type::Circumflex:          return "Circumflex";
        case Type::LeftShift:           return "Left Shift";
        case Type::RightShift:          return "Right Shift";

        // Special Operators
        case Type::Decrement:           return "Decrement";
        case Type::Increment:           return "Increment";

        // Logical Operators
        case Type::LogicalAnd:          return "Logical And";
        case Type::LogicalOr:           return "Logical Or";
        case Type::LogicalNotEqual:     return "Not Equal";
        case Type::LogicalEqual:        return "Equal";
        case Type::Less:                return "Less";
        case Type::LessOrEqual:         return "Less Or Equal";
        case Type::Greater:             return "Greater";
        case Type::GreaterOrEqual:      return "Greater Or Equal";

        // Identifiers & Literals
        case Type::Identifier:          return "Identifier";
        case Type::Integer:             return "Integer";

        // Assignment
        case Type::Equal:               return "Assign";
        case Type::PlusAssign:          return "Plus Assign";
        case Type::MinusAssign:         return "Minus Assign";
        case Type::MultiplyAssign:      return "Multiply Assign";
        case Type::DivideAssign:        return "Divide Assign";
        case Type::ModuloAssign:        return "Modulo Assign";
        case Type::BitwiseAndAssign:    return "Bitwise And Assign";
        case Type::BitwiseOrAssign:     return "Bitwise Or Assign";
        case Type::BitwiseXorAssign:    return "Bitwise Xor Assign";
        case Type::LeftShiftAssign:     return "Left Shift Assign";
        case Type::RightShiftAssign:    return "Right Shift Assign";

        // Ternary
        case Type::QuestionMark:        return "Question Mark";
        case Type::Colon:               return "Colon";

        // Keywords
        case Type::Return:              return "Return";
        case Type::Void:                return "Void";
        case Type::IntKeyword:          return "Int";
        case Type::If:                  return "If";
        case Type::Else:                return "Else";
        case Type::Do:                  return "Do";
        case Type::While:               return "While";
        case Type::For:                 return "For";
        case Type::Break:               return "Break";
        case Type::Continue:            return "Continue";
        case Type::Goto:                return "Goto";
        case Type::Switch:              return "Switch";
        case Type::Case:                return "Case";
        case Type::Default:             return "Default";
        case Type::Static:              return "Static";
        case Type::Extern:              return "extern";

        // Special Tokens
        case Type::EndOfFile:           return "End Of File";
        case Type::Invalid:             return "Invalid";

        default:                        return "Unknown Token";
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
