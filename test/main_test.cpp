#include "gtest/gtest.h"
#include "../Program.hpp"

#include <filesystem>
#include <fstream>

TEST(Chpater1, lexingValid)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/valid";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "", "--lex"};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[1] = path.path();
        Program program(args);
        ASSERT_EQ(0, program.run()) << path.path();
    }
}

TEST(Chpater1, parsingValid)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/valid";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "", "--parse"};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[1] = path.path();
        Program program(args);
        ASSERT_EQ(0, program.run()) << path.path();
    }
}

TEST(Chpater1, parsingInvalid)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/invalid_parse";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "", "--parse"};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[1] = path.path();
        Program program(args);
        EXPECT_NE(0, program.run()) << path.path();
    }
}

TEST(Chpater1, lexingInvalid)
{
    const std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/invalid_lex";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "", "--lex"};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[1] = path.path();
        Program program(args);
        EXPECT_NE(0, program.run()) << path.path();
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
