#include "gtest/gtest.h"
#include "../CompilerDriver.hpp"

#include <filesystem>
#include <fstream>

#include "../Lexing/Lexer.hpp"

const std::string testsFolderPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/";

TEST(InvalidArguments, notAFile)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "", "--codegen"};
    args[1] = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/ch";
    const CompilerDriver program(args);
    ASSERT_NE(0, program.run());
}

TEST(InvalidArguments, noArgument)
{
    const std::string validPath = testsFolderPath + "chapter_1/";
    constexpr std::vector<std::string> args;
    const CompilerDriver program(args);
    ASSERT_NE(0, program.run());
}

TEST(InvalidArguments, lexing)
{
    const std::string validPath = testsFolderPath + "chapter_1/valid/newlines.c";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "--lexing", ""};
    args[2] = validPath;
    const CompilerDriver program(args);
    ASSERT_NE(0, program.run());
}

TEST(Chpater1, lexingValid)
{
    const std::string validPath = testsFolderPath + "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Lexeme> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexems(lexemes)) << path.path();
    }
}

TEST(Chpater1, parsingValid)
{
    const std::string validPath = testsFolderPath + "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Lexeme> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexems(lexemes)) << path.path();
        Parsing::Parser parser(lexemes);
        Parsing::ProgramNode program;
        ASSERT_EQ(0, parser.parseProgram(program)) << path.path();
    }
}

TEST(Chpater1, codegenValid)
{
    const std::string validPath = testsFolderPath + "chapter_1/valid";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "--codegen", ""};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[2] = path.path();
        CompilerDriver program(args);
        ASSERT_EQ(0, program.run()) << path.path();
    }
}

TEST(Chpater1, parsingInvalid)
{
    const std::string invalidPath = testsFolderPath + "chapter_1/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Lexeme> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexems(lexemes)) << path.path();
        Parsing::Parser parser(lexemes);
        Parsing::ProgramNode program;
        ASSERT_NE(0, parser.parseProgram(program)) << path.path();
    }
}

TEST(Chpater1, lexingInvalid)
{
    const std::string invalidPath = testsFolderPath + "chapter_1/invalid_lex";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Lexeme> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_NE(0, lexer.getLexems(lexemes)) << path.path();
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

TEST(Chapter1, astPrinter)
{
    Parsing::ProgramNode program;
    program.function.name = "main";
    program.function.body.constant.value.constant = 100;
    const std::string expected =
        "Program(\n"
        "\tFunction(\n"
        "\t\tname=\"main\",\n"
        "\t\tbody=Return(\n"
        "\t\t\tConstant(100)\n"
        "\t\t)\n"
        "\t)\n"
        ")\n";
    ASSERT_EQ(expected, astPrinter(program)) << astPrinter(program);
}

TEST(Chpater2, lexingValid)
{
    const std::string validPath = testsFolderPath + "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        const std::string sourceCode = getSourceCode(path.path());
        std::vector<Lexing::Lexeme> lexemes;
        Lexing::Lexer lexer(sourceCode);
        ASSERT_EQ(0, lexer.getLexems(lexemes)) << path.path();
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