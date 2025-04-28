#include "gtest/gtest.h"
#include "CompilerDriver.hpp"
#include "Lexing/Lexer.hpp"
#include "Parsing/ConcreteTree.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

const fs::path testsFolderPath = fs::path(PROJECT_ROOT_DIR) / "test/external/writing-a-c-compiler-tests/tests";

std::string removeLinesStartingWithHash(const std::string& input)
{
    std::istringstream iss(input);
    std::string line;
    std::string result;
    while (std::getline(iss, line))
        if (line.empty() || line[0] != '#')
            result += line + '\n';
    if (!result.empty())
        result.pop_back();
    return result;
}

i32 getLexerErrors(const std::filesystem::directory_entry& filePath)
{
    const std::string sourceCode = removeLinesStartingWithHash(getSourceCode(filePath.path()));
    std::vector<Lexing::Token> lexemes;
    Lexing::Lexer lexer(sourceCode);
    return lexer.getLexemes(lexemes);
}

bool ParseFileAndGiveResult(const std::filesystem::directory_entry& filePath)
{
    const std::string sourceCode = removeLinesStartingWithHash(getSourceCode(filePath.path()));
    std::vector<Lexing::Token> lexemes;
    Lexing::Lexer lexer(sourceCode);
    lexer.getLexemes(lexemes);
    Parsing::Parse parser(lexemes);
    Parsing::Program program;
    return parser.programParse(program);
}

TEST(Chapter1, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter1, lexingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_1/invalid_lex";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_NE(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter1, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter1, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_1/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter2, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter2, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter2, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_2/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_3/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter3, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_3/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_3/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3, parsingValidExtra)
{
    const fs::path validPath = testsFolderPath / "chapter_3/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3, parsingInvalidExtra)
{
    const fs::path invalidPath = testsFolderPath / "chapter_3/invalid_parse/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter4, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_4/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter4, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_4/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter4, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_4/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file())
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
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