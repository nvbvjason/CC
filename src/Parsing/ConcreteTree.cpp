#include "ConcreteTree.hpp"

#include "IR/AbstractTree.hpp"
#include "Lexing/Token.hpp"

namespace Parsing {

bool Parse::programParse(Program& program)
{
    Function function;
    if (!functionParse(function))
        return false;
    program.function = std::make_unique<Function>(std::move(function));
    if (program.function->name != "main")
        return false;
    if (c_tokens[m_current].m_type != Lexing::TokenType::EndOfFile)
        return false;
    return true;
}

bool Parse::functionParse(Function& function)
{
    if (!expect(Lexing::TokenType::IntKeyword))
        return false;
    const Lexing::Token lexeme = advance();
    if (lexeme.m_type != Lexing::TokenType::Identifier)
        return false;
    function.name = lexeme.m_lexeme;
    if (!expect(Lexing::TokenType::OpenParen))
        return false;
    if (!expect(Lexing::TokenType::Void))
        return false;
    if (!expect(Lexing::TokenType::CloseParen))
        return false;
    if (!expect(Lexing::TokenType::OpenBrace))
        return false;
    Statement statement;
    if (!statementParse(statement))
        return false;
    function.body = std::make_unique<Statement>(std::move(statement));
    if (!expect(Lexing::TokenType::CloseBrace))
        return false;
    return true;
}

bool Parse::statementParse(Statement& statement)
{
    if (!expect(Lexing::TokenType::Return))
        return false;
    std::unique_ptr<Expr> expr = expressionParse();
    if (expr == nullptr)
        return false;
    statement.expression = std::move(expr);
    if (!expect(Lexing::TokenType::Semicolon))
        return false;
    return true;
}

std::unique_ptr<Expr> Parse::expressionParse()
{
    switch (const Lexing::Token lexeme = peek(); lexeme.m_type) {
        case Lexing::TokenType::Integer: {
            auto constantExpr = std::make_unique<ConstantExpr>(std::stoi(lexeme.m_lexeme));
            if (advance().m_type == Lexing::TokenType::EndOfFile)
                return nullptr;
            return constantExpr;
        }
        case Lexing::TokenType::Minus:
        case Lexing::TokenType::Tilde:
            return unaryParse();
        case Lexing::TokenType::OpenParen: {
            if (advance().m_type == Lexing::TokenType::EndOfFile)
                return nullptr;
            auto expr = expressionParse();
            if (!expect(Lexing::TokenType::CloseParen))
                return nullptr;
            return expr;
        }
        default:
            return nullptr;
    }
}

std::unique_ptr<UnaryExpr> Parse::unaryParse()
{
    UnaryExpr::Operator unaryOperator;
    if (!unaryOperatorParse(unaryOperator))
        return nullptr;
    std::unique_ptr<Expr> expr = expressionParse();
    if (expr == nullptr)
        return nullptr;
    return std::make_unique<UnaryExpr>(unaryOperator, std::move(expr));
}

bool Parse::unaryOperatorParse(UnaryExpr::Operator& unaryOperator)
{
    switch (const auto type = advance(); type.m_type) {
        case Lexing::TokenType::Minus:
            unaryOperator = UnaryExpr::Operator::Negate;
            break;
        case Lexing::TokenType::Tilde:
            unaryOperator = UnaryExpr::Operator::Complement;
            break;
        default:
            return false;
    }
    return true;
}

bool Parse::match(const Lexing::TokenType &type)
{
    if (type == c_tokens[m_current].m_type) {
        if (advance().m_type == Lexing::TokenType::EndOfFile)
            return false;
        return true;
    }
    return false;
}
} // Parsing