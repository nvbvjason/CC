#include "gtest/gtest.h"
#include "CompilerDriver.hpp"
#include "Lexing/Lexer.hpp"
#include "Parsing/ConcreteTree.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

const fs::path testsFolderPath = fs::path(PROJECT_ROOT_DIR) / "test/external/writing-a-c-compiler-tests/tests";

TEST(Chapter1, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexemes(lexemes)) << path.path().string();
    }
}

TEST(Chapter1, lexingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_1/invalid_lex";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_NE(0, lexer.getLexemes(lexemes)) << path.path().string();
    }
}

TEST(Chapter1, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        lexer.getLexemes(lexemes);
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_EQ(true, parser.programParse(program)) << path.path().string();
    }
}

TEST(Chapter1, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_1/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        lexer.getLexemes(lexemes);
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_FALSE(parser.programParse(program)) << path.path().string();
    }
}

TEST(Chapter2, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexemes(lexemes)) << path.path().string();
    }
}

TEST(Chapter2, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        lexer.getLexemes(lexemes);
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_TRUE(parser.programParse(program)) << path.path().string();
    }
}

TEST(Chapter2, parsingInvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_2/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        lexer.getLexemes(lexemes);
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_FALSE(parser.programParse(program)) << path.path().string();
    }
}

TEST(Chapter3, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_3/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        lexer.getLexemes(lexemes);
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_TRUE(parser.programParse(program)) << path.path().string();
    }
}

TEST(Chapter3, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_3/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file())
            continue;
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        lexer.getLexemes(lexemes);
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_FALSE(parser.programParse(program)) << path.path().string();
    }
}

TEST(Chapter3, parsingValidExtra)
{
    const fs::path invalidPath = testsFolderPath / "chapter_3/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file())
            continue;
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        lexer.getLexemes(lexemes);
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_TRUE(parser.programParse(program)) << path.path().string();
    }
}

TEST(Chapter3, parsingInvalidExtra)
{
    const fs::path invalidPath = testsFolderPath / "chapter_3/invalid_parse/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file())
            continue;
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        lexer.getLexemes(lexemes);
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_FALSE(parser.programParse(program)) << path.path().string();
    }
}

void cleanUp()
{
    for (const auto& entry : std::filesystem::directory_iterator("/home/jason/src/CC/generated_files/"))
        remove_all(entry.path());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
    cleanUp();
    return result;
}