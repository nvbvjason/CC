#include "gtest/gtest.h"
#include "../Program.h"

#include <filesystem>
#include <fstream>

TEST(Chpater1, lexingValid)
{
    std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/valid";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "", "--lex"};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[1] = path.path();
        Program program(args);
        ASSERT_EQ(0, program.run());
    }
}

TEST(Chpater1, lexingInvalid)
{
    std::string validPath = "/home/jason/src/CC/writing-a-c-compiler-tests/tests/chapter_1/invalid_lex";
    std::vector<std::string> args{"/home/jason/src/CC/cmake-build-debug/CC_run", "", "--lex"};
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        args[1] = path.path();
        Program program(args);
        ASSERT_NE(0, program.run());
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
