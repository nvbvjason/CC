#include <gtest/gtest.h>

#include "Frontend/Lexing/Token.hpp"

namespace {
using Type = Lexing::Token::Type;
using Token = Lexing::Token;
}

TEST(TokenTest, AllTypeNamesAreCorrect) {
    std::vector<std::pair<Type, std::string>> testCases = {
        // Bracketing Symbols
        {Type::OpenParen,           "Open Paren"},
        {Type::CloseParen,          "Close Paren"},
        {Type::OpenBrace,           "Open Brace"},
        {Type::CloseBrace,          "Close Brace"},
        {Type::OpenSqBracket,       "Open SqBracket"},
        {Type::CloseSqBracket,     "Close SqBracket"},

        // Punctuation & Symbols
        {Type::Semicolon,           "Semicolon"},
        {Type::Comma,               "Comma"},
        {Type::Tilde,               "Tilde"},
        {Type::ExclamationMark,     "Exclamation Mark"},

        // Arithmetic Operators
        {Type::Plus,                "Plus"},
        {Type::Minus,               "Minus"},
        {Type::Asterisk,            "Asterisk"},
        {Type::ForwardSlash,        "Forward Slash"},
        {Type::Percent,             "Percent"},

        // Bitwise Operators
        {Type::Ampersand,           "Ampersand"},
        {Type::Pipe,                "Pipe"},
        {Type::Circumflex,          "Circumflex"},
        {Type::LeftShift,           "Left Shift"},
        {Type::RightShift,          "Right Shift"},

        // Special Operators
        {Type::Decrement,           "Decrement"},
        {Type::Increment,           "Increment"},

        // Logical Operators
        {Type::LogicalAnd,          "Logical And"},
        {Type::LogicalOr,           "Logical Or"},
        {Type::LogicalNotEqual,     "Not Equal"},
        {Type::LogicalEqual,        "Equal"},
        {Type::Less,                "Less"},
        {Type::LessOrEqual,         "Less Or Equal"},
        {Type::Greater,             "Greater"},
        {Type::GreaterOrEqual,      "Greater Or Equal"},

        // Identifiers & Literals
        {Type::Identifier,          "Identifier"},
        {Type::IntegerLiteral,             "Integer"},
        {Type::UnsignedIntegerLiteral,  "Unsigned Integer"},
        {Type::LongLiteral,             "Long"},
        {Type::UnsignedLongLiteral,     "Unsigned Long"},
        {Type::DoubleLiteral,           "Double"},

        // Assignment
        {Type::Equal,               "Assign"},
        {Type::PlusAssign,          "Plus Assign"},
        {Type::MinusAssign,         "Minus Assign"},
        {Type::MultiplyAssign,      "Multiply Assign"},
        {Type::DivideAssign,        "Divide Assign"},
        {Type::ModuloAssign,        "Modulo Assign"},
        {Type::BitwiseAndAssign,    "Bitwise And Assign"},
        {Type::BitwiseOrAssign,     "Bitwise Or Assign"},
        {Type::BitwiseXorAssign,    "Bitwise Xor Assign"},
        {Type::LeftShiftAssign,     "Left Shift Assign"},
        {Type::RightShiftAssign,    "Right Shift Assign"},

        // Ternary
        {Type::QuestionMark,        "Question Mark"},
        {Type::Colon,               "Colon"},

        // Keywords
        {Type::Return,              "Return"},
        {Type::Void,                "Void"},
        {Type::IntKeyword,          "Int"},
        {Type::LongKeyword,         "Long"},
        {Type::DoubleKeyword,       "Double"},
        {Type::If,                  "If"},
        {Type::Else,                "Else"},
        {Type::Do,                  "Do"},
        {Type::While,               "While"},
        {Type::For,                  "For"},
        {Type::Break,               "Break"},
        {Type::Continue,            "Continue"},
        {Type::Goto,                "Goto"},
        {Type::Switch,              "Switch"},
        {Type::Case,                "Case"},
        {Type::Default,             "Default"},
        {Type::Static,              "Static"},
        {Type::Extern,              "extern"},

        // Special Tokens
        {Type::EndOfFile,           "End Of File"},
        {Type::NotAToken,           "Not a Token"},
        {Type::Invalid,             "Invalid"},

        // Default case (test with an invalid enum value)
        {static_cast<Type>(999),    "Unknown Token"}
    };

    for (const auto& [type, expectedName] : testCases) {
        EXPECT_EQ(Token(1, 1, type, "").getTypeName(), expectedName)
            << "Failed for TokenType: " << static_cast<int>(type);
    }
}
