#include "gtest/gtest.h"
#include "CompilerDriver.hpp"
#include "Frontend/Lexing/Lexer.hpp"
#include "Frontend/Parsing/Parser.hpp"
#include "../src/Frontend/Semantics/VariableResolution.hpp"

#include <filesystem>
#include <fstream>

#include "LabelsUnique.hpp"
#include "LvalueVerification.hpp"
#include "Switch.hpp"

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
    Parsing::Parser parser(lexemes);
    Parsing::Program program;
    return parser.programParse(program);
}

bool CheckSemantics(const std::filesystem::directory_entry& filePath)
{
    const std::string sourceCode = removeLinesStartingWithHash(getSourceCode(filePath.path()));
    std::vector<Lexing::Token> lexemes;
    Lexing::Lexer lexer(sourceCode);
    lexer.getLexemes(lexemes);
    Parsing::Parser parser(lexemes);
    Parsing::Program program;
    parser.programParse(program);
    Semantics::VariableResolution variableResolution(program);
    if (!variableResolution.resolve())
        return false;
    Semantics::LvalueVerification lvalueVerification(program);
    if (!lvalueVerification.resolve())
        return false;
    Semantics::LabelsUnique labelsUnique;
    if (!labelsUnique.programValidate(program))
        return false;
    Semantics::Switch switchVerification;
    if (!switchVerification.programValidate(program))
        return false;
    return true;
}

TEST(Chapter1, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter1, lexingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_1/invalid_lex";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_NE(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter1, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_1/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter1, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_1/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter2, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter2, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter2, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_2/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_3/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter3, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_3/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_3/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3, parsingValidExtra)
{
    const fs::path validPath = testsFolderPath / "chapter_3/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3, parsingInvalidExtra)
{
    const fs::path invalidPath = testsFolderPath / "chapter_3/invalid_parse/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter4, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_4/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter4, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_4/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter4, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_4/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_5/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path validPathExtra = testsFolderPath / "chapter_5/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPathExtra)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path invalidSemantics = testsFolderPath / "chapter_5/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(validPathExtra)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter5, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_5/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5, parsingValidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_5/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5, parsingValidForInvalidSemantics)
{
    const fs::path validPath = testsFolderPath / "chapter_5/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_5/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5, parsingInvalidExtraCredit)
{
    const fs::path invalidPath = testsFolderPath / "chapter_5/invalid_parse/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5, semanticsInvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_5/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter5, semanticsInvalidExtraCredit)
{
    const fs::path invalidPath = testsFolderPath / "chapter_5/invalid_semantics/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter5, semanticsvalidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_5/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter6, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path invalidSemantics = testsFolderPath / "chapter_6/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(invalidSemantics)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path invalidParse = testsFolderPath / "chapter_6/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidParse)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter6, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter6, parsingValidForInvalidSemantics)
{
    const fs::path validPath = testsFolderPath / "chapter_6/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter6, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_6/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter6, semanticsInvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_6/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter6, semanticsvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter6, lexingValidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter6, parsingValidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter6, parsingInValidExtraCredit)
{
    const fs::path invalidPath = testsFolderPath / "chapter_6/invalid_parse/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter6, semanticsInvalidExtraCredit)
{
    const fs::path invalidPath = testsFolderPath / "chapter_6/invalid_semantics/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter6, semanticsValidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter7, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_7/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path invalidSemantics = testsFolderPath / "chapter_7/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(invalidSemantics)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path invalidParse = testsFolderPath / "chapter_7/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidParse)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter7, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_7/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter7, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_7/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter7, semanticsInvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_7/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter7, semanticsvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_7/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter8, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_8/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path invalidSemantics = testsFolderPath / "chapter_8/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(invalidSemantics)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path invalidParse = testsFolderPath / "chapter_8/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidParse)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter8, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_8/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter8, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_8/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter8, semanticsInvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_8/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter8, lexingValidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_8/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path invalidSemantics = testsFolderPath / "chapter_8/invalid_semantics/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidSemantics)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
    const fs::path invalidParse = testsFolderPath / "chapter_8/invalid_parse/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidParse)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter8, parsingValidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_8/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter8, parsingInValidExtraCredit)
{
    const fs::path invalidPath = testsFolderPath / "chapter_8/invalid_parse/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter8, semanticsInvalidExtraCredit)
{
    const fs::path invalidPath = testsFolderPath / "chapter_8/invalid_semantics/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter8, semanticsValidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_8/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter9, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_9/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter9, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_9/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter9, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_9/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}


TEST(Chapter9, invalidSemantics)
{
    const fs::path invalidPathDeclarations = testsFolderPath / "chapter_9/invalid_declarations";
    for (const auto& path : std::filesystem::directory_iterator(invalidPathDeclarations)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
    const fs::path invalidPathTypes = testsFolderPath / "chapter_9/invalid_types";
    for (const auto& path : std::filesystem::directory_iterator(invalidPathTypes)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
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