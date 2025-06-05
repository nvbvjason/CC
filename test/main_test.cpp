#include "gtest/gtest.h"
#include "CompilerDriver.hpp"
#include "Frontend/Lexing/Lexer.hpp"
#include "Frontend/Parsing/Parser.hpp"
#include "../src/Frontend/Semantics/VariableResolution.hpp"
#include "GotoLabelsUnique.hpp"
#include "FrontendDriver.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

const fs::path testsFolderPath = fs::path(PROJECT_ROOT_DIR) / "test/external/writing-a-c-compiler-tests/tests";

std::string getSourceCode(const std::filesystem::path& inputFile)
{
    std::ifstream file(inputFile);
    std::string source((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return source;
}

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
    if (!parser.programParse(program))
        return false;
    SymbolTable symbolTable;
    const ErrorCode err = validateSemantics(program, symbolTable);
    return err == ErrorCode::OK;
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

TEST(Chapter2_Unary_Operators, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter2_Unary_Operators, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_2/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter2_Unary_Operators, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_2/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3_Binary_Operators, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_3/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter3_Binary_Operators, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_3/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter3_Binary_Operators, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_3/invalid_parse";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter4_Logical_and_Relatinal_Operators, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_4/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter4_Logical_and_Relatinal_Operators, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_4/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter4_Logical_and_Relatinal_Operators, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_4/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5_Local_Variables, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_5/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter5_Local_Variables, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_5/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5_Local_Variables, parsingValidForInvalidSemantics)
{
    const fs::path validPath = testsFolderPath / "chapter_5/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5_Local_Variables, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_5/invalid_parse";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter5_Local_Variables, semanticsInvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_5/invalid_semantics";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter5_Local_Variables, semanticsvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_5/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter6_If_and_Conditional_Statements, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter6_If_and_Conditional_Statements, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter6_If_and_Conditional_Statements, parsingValidForInvalidSemantics)
{
    const fs::path validPath = testsFolderPath / "chapter_6/invalid_semantics";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter6_If_and_Conditional_Statements, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_6/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter6_If_and_Conditional_Statements, semanticsInvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_6/invalid_semantics";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter6_If_and_Conditional_Statements, semanticsvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter6_If_and_Conditional_Statements, lexingValidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter6_If_and_Conditional_Statements, parsingValidExtraCredit)
{
    const fs::path validPath = testsFolderPath / "chapter_6/valid/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter6_If_and_Conditional_Statements, parsingInValidExtraCredit)
{
    const fs::path invalidPath = testsFolderPath / "chapter_6/invalid_parse/extra_credit";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter7_Compound_Statements, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_7/valid";
    for (const auto& path : std::filesystem::directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter7_Compound_Statements, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_7/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter7_Compound_Statements, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_7/invalid_parse";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter7_Compound_Statements, semanticsInvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_7/invalid_semantics";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter7, semanticsvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_7/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter8_Loops, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_8/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter8_Loops, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_8/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter8_Loops, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_8/invalid_parse";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter8_Loops, semanticsInvalid)
{
    const fs::path validPath = testsFolderPath / "chapter_8/invalid_semantics";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter8_Loops, semanticsValid)
{
    const fs::path validPath = testsFolderPath / "chapter_8/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter9_Functions, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_9/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter9_Functions, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_9/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter9_Functions, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_9/invalid_parse";
    for (const auto& path : std::filesystem::directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter9_Functions, invalid_declarations)
{
    const fs::path invalidPathDeclarations = testsFolderPath / "chapter_9/invalid_declarations";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPathDeclarations)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter9_Functions, invalid_labels)
{
    const fs::path invalidPathDeclarations = testsFolderPath / "chapter_9/invalid_labels";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPathDeclarations)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter9_Functions, invalid_types)
{
    const fs::path invalidPathDeclarations = testsFolderPath / "chapter_9/invalid_types";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPathDeclarations)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter9_Functions, validSemantics)
{
    const fs::path invalidPath = testsFolderPath / "chapter_9/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter10_File_Scope_Decls_and_Storageclass, lexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_10/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter10_File_Scope_Decls_and_Storageclass, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_10/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter10_File_Scope_Decls_and_Storageclass, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_10/invalid_parse";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter10_File_Scope_Decls_and_Storageclass, validSemantics)
{
    const fs::path invalidPath = testsFolderPath / "chapter_10/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter10_File_Scope_Decls_and_Storageclass, invalid_declarations)
{
    const fs::path invalidPathDeclarations = testsFolderPath / "chapter_10/invalid_declarations";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPathDeclarations)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter10_File_Scope_Decls_and_Storageclass, invalid_types)
{
    const fs::path invalidPathLabels = testsFolderPath / "chapter_10/invalid_types";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPathLabels)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter10_File_Scope_Decls_and_Storageclass, invalid_labels)
{
    const fs::path invalidPathLabels = testsFolderPath / "chapter_10/invalid_labels";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPathLabels)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter11_Long_Integers, validLexingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_11/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_EQ(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter11_Long_Integers, inValidLexingValid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_11/invalid_lex";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_NE(0, getLexerErrors(path)) << path.path().string();
    }
}

TEST(Chapter11_Long_Integers, parsingValid)
{
    const fs::path validPath = testsFolderPath / "chapter_11/valid";
    for (const auto& path : std::filesystem::recursive_directory_iterator(validPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_TRUE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter11_Long_Integers, parsingInvalid)
{
    const fs::path invalidPath = testsFolderPath / "chapter_11/invalid_parse";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(ParseFileAndGiveResult(path)) << path.path().string();
    }
}

TEST(Chapter11_Long_Integers, invalidLabels)
{
    const fs::path invalidPath = testsFolderPath / "chapter_11/invalid_labels";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
        if (!path.is_regular_file() || path.path().extension() != ".c")
            continue;
        EXPECT_FALSE(CheckSemantics(path)) << path.path().string();
    }
}

TEST(Chapter11_Long_Integers, invalidTypes)
{
    const fs::path invalidPath = testsFolderPath / "chapter_11/invalid_types";
    for (const auto& path : std::filesystem::recursive_directory_iterator(invalidPath)) {
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