#include "gtest/gtest.h"
#include "../CompilerDriver.hpp"

#include <filesystem>
#include <fstream>

TEST(InvalidArguments, notAFile)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "", "--codegen"};
    args[1] = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/ch";
    CompilerDriver program(args);
    ASSERT_NE(0, program.run());
}

TEST(InvalidArguments, noArgument)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/";
    std::vector<std::string> args;
    CompilerDriver program(args);
    ASSERT_NE(0, program.run());
}

TEST(InvalidArguments, lexing)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/valid/newlines.c";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "--lexing", ""};
    args[2] = validPath;
    CompilerDriver program(args);
    ASSERT_NE(0, program.run());
}

TEST(Chpater1, lexingValid)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/valid";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "--lex", ""};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[2] = path.path();
        CompilerDriver program(args);
        ASSERT_EQ(0, program.run()) << path.path();
    }
}

TEST(Chpater1, parsingValid)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/valid";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "--parse", ""};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[2] = path.path();
        CompilerDriver program(args);
        ASSERT_EQ(0, program.run()) << path.path();
    }
}

TEST(Chpater1, codegenValid)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/valid";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "--codegen", ""};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[2] = path.path();
        CompilerDriver program(args);
        ASSERT_EQ(0, program.run()) << path.path();
    }
}

TEST(Chpater1, parsingInvalid)
{
    const std::string invalidPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/invalid_parse";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "--parse", ""};
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        args[2] = path.path();
        CompilerDriver program(args);
        EXPECT_NE(0, program.run()) << path.path();
    }
}

TEST(Chpater1, lexingInvalid)
{
    const std::string invalidPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/invalid_lex";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "--lex", ""};
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        args[2] = path.path();
        CompilerDriver program(args);
        EXPECT_NE(0, program.run()) << path.path();
    }
}

TEST(Chpater1, allvalid)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/valid";
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
    std::string expected =
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
