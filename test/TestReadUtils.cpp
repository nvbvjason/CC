#include "TestReadUtils.hpp"
#include "FrontendDriver.hpp"
#include "StateCode.hpp"
#include "SymbolTable.hpp"
#include "TokenStore.hpp"
#include "Lexing/Lexer.hpp"
#include "Parsing/Parser.hpp"

#include <fstream>

std::string getSourceCode(const std::filesystem::path& inputFile)
{
    std::ifstream file(inputFile);
    std::string source((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    return source;
}

std::string removeLinesStartingWithHashOrComment(const std::string& input)
{
    std::istringstream iss(input);
    std::string line;
    std::string result;
    while (std::getline(iss, line)) {
        if (line.size() < 2)
            continue;
        if (line[0] == '#')
            continue;
        if (line[0] == '\\' || line[1] == '\\')
            continue;
        result += line + '\n';
    }
    if (!result.empty())
        result.pop_back();
    return result;
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

bool lexerValid(const std::filesystem::directory_entry& filePath)
{
    const std::string sourceCode = removeLinesStartingWithHash(getSourceCode(filePath.path()));
    TokenStore tokenStore;
    Lexing::Lexer lexer(sourceCode, tokenStore);
    const std::vector<Error> errors = lexer.getLexemes();
    return errors.empty();
}

bool ParseFileAndGiveResult(const std::filesystem::directory_entry& filePath)
{
    const std::string sourceCode = removeLinesStartingWithHash(getSourceCode(filePath.path()));
    TokenStore tokenStore;
    Lexing::Lexer lexer(sourceCode, tokenStore);
    lexer.getLexemes();
    Parsing::Parser parser(tokenStore);
    Parsing::Program program;
    return parser.programParse(program).empty();
}

bool CheckSemantics(const std::filesystem::directory_entry& filePath)
{
    const std::string sourceCode = removeLinesStartingWithHash(getSourceCode(filePath.path()));
    TokenStore tokenStore;
    Lexing::Lexer lexer(sourceCode, tokenStore);
    lexer.getLexemes();
    Parsing::Parser parser(tokenStore);
    Parsing::Program program;
    if (!parser.programParse(program).empty())
        return false;
    SymbolTable symbolTable;
    VarTable varTable;
    const auto [err, errors] = validateSemantics(program, symbolTable, varTable);
    return err == StateCode::Done;
}

bool CheckSemanticsWithInclude(const std::string& sourceCode)
{
    TokenStore tokenStore;
    Lexing::Lexer lexer(sourceCode, tokenStore);
    lexer.getLexemes();
    Parsing::Parser parser(tokenStore);
    Parsing::Program program;
    if (!parser.programParse(program).empty())
        return false;
    SymbolTable symbolTable;
    VarTable varTable;
    const auto [err, errors] = validateSemantics(program, symbolTable, varTable);
    return err == StateCode::Done;
}

bool isCFile(const std::filesystem::directory_entry& entry)
{
    return entry.is_regular_file() && entry.path().extension() == ".c";
}

bool isHFile(const std::filesystem::directory_entry& entry)
{
    return entry.is_regular_file() && entry.path().extension() == ".h";
}

bool isCorHFile(const std::filesystem::directory_entry& entry)
{
    return entry.is_regular_file() && entry.path().extension() == ".c" || entry.path().extension() == ".h";
}

void handlePreprocessDumb(const std::unordered_map<std::string, std::string>& hFiles,
                          const std::string& line, std::string& result)
{
    if (line.starts_with("#include")) {
        for (const auto[name, contents] : hFiles) {
            if (line.contains(name)) {
                result += contents;
                result += '\n';
                return;
            }
        }
    }
}

std::string buildFileWithIncludes(
    const std::filesystem::path& path,
    const std::unordered_map<std::string, std::string>& hFiles)
{
    std::string codes = getSourceCode(path);
    std::istringstream iss(codes);
    std::string line;
    std::string result;
    while (std::getline(iss, line)) {
        if (line.size() < 2)
            continue;
        if (line[0] == '#') {
            handlePreprocessDumb(hFiles, line, result);
            continue;
        }
        if (line[0] == '\\' || line[1] == '\\')
            continue;
        result += line + '\n';
    }
    if (!result.empty())
        result.pop_back();
    return result;
}