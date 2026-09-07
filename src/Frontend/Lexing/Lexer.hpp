#pragma once

#include "Token.hpp"
#include "TokenStore.hpp"
#include "Error.hpp"

#include <climits>
#include <string>
#include <unordered_map>

namespace Lexing {

class Lexer {
    using Type = Token::Type;
    static constexpr u64 MAX_I32 = INT_MAX;
    static constexpr u64 MAX_U32 = UINT_MAX;
    const std::string& source;
    i32 current = 0;
    i32 start = 0;
    i32 line = 1;
    u16 column = 1;
    u16 ahead = 0;
    TokenStore& tokenStore;
    std::vector<Error> errors;
    static inline std::unordered_map<std::string, Token::Type> keywords = {
        { "return", Type::Return },
        { "int", Type::IntKeyword },
        { "void", Type::VoidKeyword },
        { "if", Type::If },
        { "else", Type::Else },
        { "do", Type::Do },
        { "while", Type::While },
        { "for", Type::For },
        { "break", Type::Break },
        { "continue", Type::Continue },
        { "goto", Type::Goto },
        { "switch", Type::Switch },
        { "case", Type::Case },
        { "default", Type::Default },
        { "static", Type::Static },
        { "extern", Type::Extern },
        { "long", Type::LongKeyword},
        { "signed", Type::Signed},
        { "unsigned", Type::Unsigned},
        { "double", Type::DoubleKeyword},
        { "struct", Type::StructKeyword},
        { "union", Type::UnionKeyword},
        { "char", Type::CharKeyword},
        { "sizeof", Type::SizeOf},
    };
public:
    explicit Lexer(const std::string& input, TokenStore& tokenStore)
        : source(input), tokenStore(tokenStore)
    {
        const size_t estimatedTokens = source.length() / 4;
        tokenStore.reserve(estimatedTokens);
    }
    std::vector<Error> getLexemes();
    void string();
    i32 handleEscapedChars();
private:
    [[nodiscard]] bool isAtEnd() const { return source.size() <= current; }
    [[nodiscard]] char peek() const;
    [[nodiscard]] char peekNext() const;

    bool match(char expected);
    bool match(std::string_view expected);
    void scanToken();
    char advance();
    void addCharLiteral(char ch) const;
    void addStringLiteral(const std::string& str) const;
    void addToken(Token::Type type, u64 num, i32 ahead, std::string& text) const;
    void addToken(Token::Type type);
    void addTokenStoreString(Token::Type type) const;

    void forwardSlash();
    void identifier();
    void number();
    void floating();
    void character();
};

inline bool matchesUL(const std::string& text, const i32 endNumbers, const i32 current)
{
    if (endNumbers + 2 != current)
        return false;
    return (tolower(text[text.size() - 2]) == 'l' &&
            tolower(text.back()) == 'u') ||
           (tolower(text[text.size() - 2]) == 'u' &&
            tolower(text.back()) == 'l');
}

} // Lexing