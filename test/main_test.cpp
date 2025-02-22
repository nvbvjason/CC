#include "gtest/gtest.h"
#include "../CompilerDriver.hpp"
#include "../Lexing/Lexer.hpp"
#include "../Parsing/ConcreteTree.hpp"

#include <filesystem>
#include <fstream>

const std::string testsFolderPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/";

TEST(Chpater1, lexingValid)
{
    const std::string validPath = testsFolderPath + "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexems(lexemes)) << path.path();
    }
}

TEST(Chpater1, lexingInvalid)
{
    const std::string invalidPath = testsFolderPath + "chapter_1/invalid_lex";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_NE(0, lexer.getLexems(lexemes)) << path.path();
    }
}

TEST(Chpater1, parsingValid)
{
    const std::string validPath = testsFolderPath + "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexems(lexemes)) << path.path();
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_EQ(0, parser.programParse(program)) << path.path();
    }
}

TEST(Chpater1, parsingInvalid)
{
    const std::string invalidPath = testsFolderPath + "chapter_1/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexems(lexemes)) << path.path();
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_NE(0, parser.programParse(program)) << path.path();
    }
}

TEST(Chpater1, allvalid)
{
    const std::string validPath = testsFolderPath + "chapter_1/valid";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", ""};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[1] = path.path();
        CompilerDriver program(args);
        ASSERT_EQ(0, program.run()) << path.path();
    }
}

TEST(Chpater2, lexingValid)
{
    const std::string validPath = testsFolderPath + "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexems(lexemes)) << path.path();
    }
}

TEST(Chpater2, parsingValid)
{
    const std::string validPath = testsFolderPath + "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexems(lexemes)) << path.path();
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_EQ(0, parser.programParse(program)) << path.path();
    }
}

TEST(Chpater2, parsingInvalid)
{
    const std::string validPath = testsFolderPath + "chapter_2/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Token> lexemes;
        Lexing::Lexer lexer(sourceCode);
        if (lexer.getLexems(lexemes) != 0)
            return;
        Parsing::Parse parser(lexemes);
        Parsing::Program program;
        ASSERT_NE(0, parser.programParse(program)) << path.path();
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